#include <Arduino.h>
#include "LumenProtocol.h"
#include "user_variables.h"
#include "hmi_bindings.h"
#include "hmi_renderer.h"
#include "smartcure_translations.h"

// UART configuration for HMI
#define HMI_RX     16
#define HMI_TX     17
#define HMI_BAUD   115200

// Use hardware serial port 2 to communicate with the UnicView display
HardwareSerial HMIserial(2);

// Forward declarations for LumenProtocol byte handlers.  The UnicView library
// calls these functions to send and receive raw bytes over the selected serial
// interface.  They MUST be defined with C linkage (extern "C") so that the
// C implementation of the protocol can call them.
extern "C" void lumen_write_bytes(uint8_t *data, uint32_t length) {
  HMIserial.write(data, length);
}

extern "C" uint16_t lumen_get_byte() {
  // When no data is available, DATA_NULL signals the parser that there is
  // nothing to read.  Otherwise return the next byte.
  return HMIserial.available() ? HMIserial.read() : DATA_NULL;
}

/*
 * SmartCure – control logic for cure timers on a Vitor Vision HMI.
 *
 * This sketch centralises all of the timing logic on the ESP32 instead of
 * relying on UnicView operators.  The display simply writes the duration
 * selected by the user (selected_pre_cureAddress) and a command to start,
 * pause or stop (timer_start_stopAddress).  The ESP32 keeps track of the
 * elapsed time using millis(), updates the running counter every second and
 * computes a progress bar value from 0–1000 (permille).  When the elapsed
 * time meets or exceeds the selected duration the counter resets and the
 * status returns to idle.  Additional presets may be configured on the HMI
 * for pre_cure_1–pre_cure_7.
 */

// Define an additional address for the progress bar permille value.  This
// address must be added to the UnicView project as a User Variable of type
// S32.  It should not conflict with existing addresses.  We chose 141 as
// it follows the existing range up to 140.
const uint16_t progress_permilleAddress = 141;

// Internal state for the timer
enum CureState { STATE_IDLE = 0, STATE_RUNNING = 1, STATE_PAUSED = 2 };
static CureState cureState = STATE_IDLE;

// Target duration in seconds.  This is set when the HMI writes to
// selected_pre_cureAddress.  If zero, the timer will not start.
static uint32_t target_time_s = 0;

// Timestamp (in milliseconds) when the current run started.  When the timer
// is paused we accumulate the time spent paused so that the elapsed time
// calculation excludes it.
static uint64_t start_ms = 0;
static uint64_t pause_start_ms = 0;
static uint64_t accumulated_pause_ms = 0;

// Last values reported to the HMI so that we avoid writing unchanged data
static int32_t last_time_reported = -1;
static int32_t last_progress_reported = -1;

// Local copies of preset durations.  These may be updated by the HMI at
// runtime if the user edits the preset values.  Defaults correspond to
// pre‑cure durations used previously: 6s, 15s, 30s, 60s, 90s, 120s, 180s.
static uint32_t pre_cure_values[7] = {6, 15, 30, 60, 90, 120, 180};

// Create lumen_packet_t instances for each variable we need to write back
// to the HMI.  The addresses for selected_pre_cure, time_curando and
// timer_start_stop come from user_variables.h.  We assign our custom
// progress_permilleAddress here.
static lumen_packet_t selected_pre_curePacket = { selected_pre_cureAddress, kS32 };
static lumen_packet_t time_curandoPacket     = { time_curandoAddress,     kS32 };
static lumen_packet_t timer_start_stopPacket = { timer_start_stopAddress, kS32 };
static lumen_packet_t progress_permillePacket= { progress_permilleAddress,kS32 };

// Helper to write an integer value to the HMI.  It updates the packet
// structure with the type and data then calls lumen_write_packet().
static inline void writeInt(lumen_packet_t* packet, int32_t value) {
  packet->type = kS32;
  packet->data._s32 = value;
  lumen_write_packet(packet);
}

// Helper to read packets from the LumenProtocol buffer.  It mimics the
// lumen_read_packet_compat() function found in the original code.  When
// lumen_available() indicates that at least one packet has been decoded
// this function retrieves the first packet into 'out' and returns true.
// Otherwise it returns false.
static bool lumen_read_packet_compat(lumen_packet_t &out) {
  if (lumen_available() > 0) {
    lumen_packet_t* p = lumen_get_first_packet();
    if (p) {
      out = *p;
      return true;
    }
  }
  return false;
}

// Start a new cure cycle.  This resets the internal timers and moves
// the state to RUNNING.  If no duration has been selected (target_time_s
// is zero) the function does nothing.  The HMI state variable
// timer_start_stop is set to 1 to indicate that counting is active.
static void startCure() {
  if (target_time_s == 0) {
    return;
  }
  cureState = STATE_RUNNING;
  start_ms = millis();
  accumulated_pause_ms = 0;
  last_time_reported = -1;
  last_progress_reported = -1;
  writeInt(&timer_start_stopPacket, 1);
}

// Pause a running cure.  The current time is stored so that we can
// accumulate the paused duration when resuming.  The HMI state variable
// timer_start_stop is set to 3 to indicate a paused condition.
static void pauseCure() {
  if (cureState == STATE_RUNNING) {
    cureState = STATE_PAUSED;
    pause_start_ms = millis();
    writeInt(&timer_start_stopPacket, 3);
  }
}

// Resume a paused cure.  The time spent paused since pauseCure() is
// accumulated so that the elapsed time excludes this period.  The state
// returns to RUNNING and the HMI state variable timer_start_stop is set
// back to 1.
static void resumeCure() {
  if (cureState == STATE_PAUSED) {
    uint64_t now = millis();
    accumulated_pause_ms += now - pause_start_ms;
    cureState = STATE_RUNNING;
    writeInt(&timer_start_stopPacket, 1);
  }
}

// Stop the timer and reset all counters.  This function is called
// explicitly when the user presses stop or when the target duration is
// reached.  It resets the state to IDLE and writes zeros back to the HMI
// for the time counter and progress bar.
static void stopCure() {
  cureState = STATE_IDLE;
  writeInt(&time_curandoPacket, 0);
  writeInt(&progress_permillePacket, 0);
  writeInt(&timer_start_stopPacket, 0);
}

void setup() {
  Serial.begin(115200);
  HMIserial.begin(HMI_BAUD, SERIAL_8N1, HMI_RX, HMI_TX);

  // Allow time for the HMI to boot.  Without this delay the first
  // messages may be missed on some devices.
  delay(800);

  // Populate preset durations on the HMI.  These writes ensure that
  // pre_cure_1..pre_cure_7 variables reflect the defaults contained in
  // pre_cure_values[].  If the HMI is configured to allow editing of
  // these values the ESP32 will subsequently overwrite these arrays when
  // packets for the corresponding addresses are received.
  writeInt(&pre_cure_1Packet, pre_cure_values[0]);
  writeInt(&pre_cure_2Packet, pre_cure_values[1]);
  writeInt(&pre_cure_3Packet, pre_cure_values[2]);
  writeInt(&pre_cure_4Packet, pre_cure_values[3]);
  writeInt(&pre_cure_5Packet, pre_cure_values[4]);
  writeInt(&pre_cure_6Packet, pre_cure_values[5]);
  writeInt(&pre_cure_7Packet, pre_cure_values[6]);

  // Initialise the HMI variables for the timer.  selected_pre_cure
  // initially zero indicates that no duration is selected.  The timer
  // state is idle and both time_curando and progress are zero.
  writeInt(&selected_pre_curePacket, 0);
  stopCure();

  // Optionally fill language list and render initial UI.  This mirrors
  // functionality from the original MVP.ino.  It is retained here in
  // case the project still relies on language selection and rendering.
  HMI_FillLanguageList();
  lumen_write(&langPacket, 1); // Default to Portuguese (1)
  HMI_RenderAll(LANG_PT);
}

void loop() {
  // Process incoming Lumen packets from the display.  We extract each
  // packet and act on those relevant to our logic.  Unrelated packets
  // (e.g. for strings or other UI elements) are ignored.
  lumen_packet_t pkt;
  while (lumen_read_packet_compat(pkt)) {
    uint16_t addr = pkt.address;
    int32_t value = 0;
    // Extract a numeric value from the packet regardless of its type
    switch (pkt.type) {
      case kS32: value = pkt.data._s32; break;
      case kU32: value = (int32_t)pkt.data._u32; break;
      case kS16: value = (int32_t)pkt.data._s16; break;
      case kU16: value = (int32_t)pkt.data._u16; break;
      case kS8:  value = (int32_t)pkt.data._s8;  break;
      case kU8:  value = (int32_t)pkt.data._u8;  break;
      default: break;
    }

    if (addr == selected_pre_cureAddress) {
      // User has chosen a new duration.  Update the target time and echo
      // the value back to the HMI.  A value <= 0 effectively disables
      // the timer until a valid duration is set.
      target_time_s = (value > 0) ? (uint32_t)value : 0;
      writeInt(&selected_pre_curePacket, target_time_s);
    } else if (addr == timer_start_stopAddress) {
      // The display is commanding the timer: 0=Stop, 1=Start/Resume,
      // 2=Single (treated as Start), 3=Pause.  We interpret and act
      // accordingly on our internal state machine.
      if (value == 0) {
        stopCure();
      } else if (value == 1 || value == 2) {
        if (cureState == STATE_IDLE) {
          startCure();
        } else if (cureState == STATE_PAUSED) {
          resumeCure();
        }
      } else if (value == 3) {
        pauseCure();
      }
    } else if (addr == pre_cure_1Address) {
      pre_cure_values[0] = (value > 0) ? (uint32_t)value : pre_cure_values[0];
    } else if (addr == pre_cure_2Address) {
      pre_cure_values[1] = (value > 0) ? (uint32_t)value : pre_cure_values[1];
    } else if (addr == pre_cure_3Address) {
      pre_cure_values[2] = (value > 0) ? (uint32_t)value : pre_cure_values[2];
    } else if (addr == pre_cure_4Address) {
      pre_cure_values[3] = (value > 0) ? (uint32_t)value : pre_cure_values[3];
    } else if (addr == pre_cure_5Address) {
      pre_cure_values[4] = (value > 0) ? (uint32_t)value : pre_cure_values[4];
    } else if (addr == pre_cure_6Address) {
      pre_cure_values[5] = (value > 0) ? (uint32_t)value : pre_cure_values[5];
    } else if (addr == pre_cure_7Address) {
      pre_cure_values[6] = (value > 0) ? (uint32_t)value : pre_cure_values[6];
    }
  }

  // Update timing and progress while running
  if (cureState == STATE_RUNNING) {
    uint64_t now = millis();
    uint64_t elapsed_ms = now - start_ms - accumulated_pause_ms;
    uint32_t elapsed_s = elapsed_ms / 1000UL;

    // Update the time counter only when it changes to reduce bus traffic
    if ((int32_t)elapsed_s != last_time_reported) {
      last_time_reported = (int32_t)elapsed_s;
      writeInt(&time_curandoPacket, last_time_reported);
    }

    // Compute progress in permille (0–1000).  Avoid overflow by using
    // 64‑bit arithmetic.  Cap at 1000 and update only when changed.
    if (target_time_s > 0) {
      int32_t progress = (int32_t)((elapsed_ms * 1000ULL) / (target_time_s * 1000ULL));
      if (progress > 1000) progress = 1000;
      if (progress != last_progress_reported) {
        last_progress_reported = progress;
        writeInt(&progress_permillePacket, progress);
      }

      // If the elapsed time meets or exceeds the target, finish the cure
      if (elapsed_s >= target_time_s) {
        // Send final values (exactly target seconds and 1000 permille)
        writeInt(&time_curandoPacket, (int32_t)target_time_s);
        writeInt(&progress_permillePacket, 1000);
        stopCure();
      }
    }
  }

  // Optionally tick the acknowledgement handler.  If your project
  // relies on ACK responses to ensure transmission reliability you can
  // uncomment the line below and call lumen_ack_trigger() periodically
  // with the elapsed milliseconds since the last call.
  // lumen_ack_trigger(10);
}
