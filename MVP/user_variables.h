#pragma once

#include "LumenProtocol.h"
#include <stdint.h>
#include <string.h>

// UnicView (User Variables / Widgets) addresses
static const uint16_t ADDR_MAIN_SCREEN       = 121; // S32 (current screen ID)
static const uint16_t ADDR_TXT_CONFIG        = 122; // String
static const uint16_t ADDR_LANG_VAR          = 123; // S32/U16 -> 0=EN,1=PT,2=ES,3=DE
static const uint16_t ADDR_TXT_START         = 124; // String
static const uint16_t ADDR_TXT_LANG          = 125; // String
static const uint16_t ADDR_LIST_LANG         = 126; // Basic Text List (index 0..3)
static const uint16_t ADDR_TXT_ADMIN         = 127; // String (e.g.: "Admin")
static const uint16_t ADDR_TXT_SYSTEM        = 128; // String (e.g.: subtitle "System")
static const uint16_t ADDR_START_GLAZE_CURE  = 129; // "Start Glaze Cure" label
static const uint16_t ADDR_PRE_CURE_1        = 130; // Pre-cure step 1 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_2        = 131; // Pre-cure step 2 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_3        = 132; // Pre-cure step 3 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_4        = 133; // Pre-cure step 4 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_5        = 134; // Pre-cure step 5 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_6        = 135; // Pre-cure step 6 duration (s, HMI updatable)
static const uint16_t ADDR_PRE_CURE_7        = 136; // Pre-cure step 7 duration (s, HMI updatable)
static const uint16_t ADDR_TXT_SECONDS       = 137; // "Seconds" label
static const uint16_t ADDR_SELECTED_PRE_CURE = 138; // Selected pre-cure preset
static const uint16_t ADDR_TIME_CURANDO      = 139; // Elapsed curing time (s)
static const uint16_t ADDR_TIMER_START_STOP  = 140; // Timer control: 0=stop,1=start,3=pause
static const uint16_t ADDR_PROGRESS_PERMILLE = 141; // Progress bar (0-1000 permille)

static const uint16_t MAX_LIST_SIZE          = 10;

// Packet instances
static lumen_packet_t main_screenPacket       = { ADDR_MAIN_SCREEN, kS32 };   // Selected screen ID
static lumen_packet_t txt_configPacket        = { ADDR_TXT_CONFIG, kString }; // Config label text
static lumen_packet_t langPacket              = { ADDR_LANG_VAR, kS32 };      // Language setting
static lumen_packet_t txt_start_curePacket    = { ADDR_TXT_START, kString };  // Start cure label text
static lumen_packet_t txt_langPacket          = { ADDR_TXT_LANG, kString };   // Language label text
static lumen_packet_t list_langPacket         = { ADDR_LIST_LANG, kS32 };     // Language list index
static lumen_packet_t txt_adminPacket         = { ADDR_TXT_ADMIN, kString };  // Admin label text
static lumen_packet_t txt_systemPacket        = { ADDR_TXT_SYSTEM, kString }; // System label text
static lumen_packet_t start_glaze_curePacket  = { ADDR_START_GLAZE_CURE, kString }; // Start glaze cure label text
static lumen_packet_t pre_cure_1Packet        = { ADDR_PRE_CURE_1, kS32 };    // Pre-cure stage 1 time (HMI updatable)
static lumen_packet_t pre_cure_2Packet        = { ADDR_PRE_CURE_2, kS32 };    // Pre-cure stage 2 time (HMI updatable)
static lumen_packet_t pre_cure_3Packet        = { ADDR_PRE_CURE_3, kS32 };    // Pre-cure stage 3 time (HMI updatable)
static lumen_packet_t pre_cure_4Packet        = { ADDR_PRE_CURE_4, kS32 };    // Pre-cure stage 4 time (HMI updatable)
static lumen_packet_t pre_cure_5Packet        = { ADDR_PRE_CURE_5, kS32 };    // Pre-cure stage 5 time (HMI updatable)
static lumen_packet_t pre_cure_6Packet        = { ADDR_PRE_CURE_6, kS32 };    // Pre-cure stage 6 time (HMI updatable)
static lumen_packet_t pre_cure_7Packet        = { ADDR_PRE_CURE_7, kS32 };    // Pre-cure stage 7 time (HMI updatable)
static lumen_packet_t txt_secondsPacket       = { ADDR_TXT_SECONDS, kString }; // Seconds label text
static lumen_packet_t selected_pre_curePacket = { ADDR_SELECTED_PRE_CURE, kS32 }; // Selected pre-cure preset
static lumen_packet_t time_curandoPacket      = { ADDR_TIME_CURANDO, kS32 }; // Elapsed curing time (s)
static lumen_packet_t timer_start_stopPacket  = { ADDR_TIMER_START_STOP, kS32 }; // Timer control state
static lumen_packet_t progress_permillePacket = { ADDR_PROGRESS_PERMILLE, kS32 }; // Progress 0-1000 (permille)

// Helper functions for writing values to the HMI variables
inline void lumen_write(lumen_packet_t* p, int32_t value) {
  p->type = kS32;
  p->data._s32 = value;
  lumen_write_packet(p);
}

inline void lumen_write(lumen_packet_t* p, const char* value) {
  p->type = kString;
  if (!value) {
    p->data._string[0] = '\0';
  } else {
    strncpy(p->data._string, value, sizeof(p->data._string) - 1);
    p->data._string[sizeof(p->data._string) - 1] = '\0';
  }
  lumen_write_packet(p);
}

=======
#pragma once
#include <stdint.h>

// UnicView (endereços de User Variables / Widgets)
static const uint16_t ADDR_MAIN_SCREEN      = 121; // S32 (id de tela, se usar)
static const uint16_t ADDR_TXT_CONFIG       = 122; // String
static const uint16_t ADDR_LANG_VAR         = 123; // S32/U16 -> 0=EN,1=PT,2=ES,3=DE
static const uint16_t ADDR_TXT_START        = 124; // String
static const uint16_t ADDR_TXT_LANG         = 125; // String
static const uint16_t ADDR_LIST_LANG        = 126; // Basic Text List (índice 0..3)
static const uint16_t ADDR_TXT_ADMIN        = 127; // String (ex.: "Admin")
static const uint16_t ADDR_TXT_SYSTEM       = 128; // String (ex.: subtítulo Settings)

static const uint16_t MAX_LIST_SIZE         = 10;
=======
#pragma once

#include "LumenProtocol.h"
#include "hmi_addresses.h"
#include <string.h>

// Predefined packets for HMI user variables
static lumen_packet_t langPacket = { ADDR_LANG_VAR, kS32 };
static lumen_packet_t langListPacket = { ADDR_LIST_LANG, kS32 };
static lumen_packet_t txt_start_curePacket = { ADDR_TXT_START, kString };

// Overloaded helpers for writing values to the HMI variables
inline void lumen_write(lumen_packet_t* p, int32_t value) {
  p->type = kS32;
  p->data._s32 = value;
  lumen_write_packet(p);
}

inline void lumen_write(lumen_packet_t* p, const char* value) {
  p->type = kString;
  if (!value) {
    p->data._string[0] = '\0';
  } else {
    strncpy(p->data._string, value, sizeof(p->data._string) - 1);
    p->data._string[sizeof(p->data._string) - 1] = '\0';
  }
  lumen_write_packet(p);
}

=======
#ifndef USER_VARIABLES_H
#define USER_VARIABLES_H

#include "LumenProtocol.h"

// Address constants
const uint16_t main_screenAddress      = 121; // Current screen ID (default 0)
const uint16_t txt_configAddress       = 122; // "Config" label
const uint16_t langAddress             = 123; // Language selection (0=EN,1=PT,2=ES,3=DE)
const uint16_t txt_start_cureAddress   = 124; // "Start Cure" label
const uint16_t txt_langAddress         = 125; // "Language" label
const uint16_t list_langAddress        = 126; // Language list index
const uint16_t txt_adminAddress        = 127; // "Admin" label
const uint16_t txt_systemAddress       = 128; // "System" label
const uint16_t start_glaze_cureAddress = 129; // "Start Glaze Cure" label
const uint16_t pre_cure_1Address       = 130; // Pre-cure step 1 duration (s)
const uint16_t pre_cure_2Address       = 131; // Pre-cure step 2 duration (s)
const uint16_t pre_cure_3Address       = 132; // Pre-cure step 3 duration (s)
const uint16_t pre_cure_4Address       = 133; // Pre-cure step 4 duration (s)
const uint16_t pre_cure_5Address       = 134; // Pre-cure step 5 duration (s)
const uint16_t pre_cure_6Address       = 135; // Pre-cure step 6 duration (s)
const uint16_t pre_cure_7Address       = 136; // Pre-cure step 7 duration (s)
const uint16_t txt_secondsAddress      = 137; // "Seconds" label
const uint16_t selected_pre_cureAddress = 138; // Selected pre-cure duration (s)
const uint16_t time_curandoAddress      = 139; // Elapsed cure time (s)
const uint16_t timer_start_stopAddress  = 140; // Timer command/state (0=stop,1=start,3=pause)
const uint16_t progress_permilleAddress = 141; // Progress bar value (0–1000)

// Packet instances
static lumen_packet_t main_screenPacket      = { main_screenAddress, kS32 };   // Selected screen ID
static lumen_packet_t txt_configPacket       = { txt_configAddress, kString }; // Config label text
static lumen_packet_t langPacket             = { langAddress, kS32 };          // Language setting
static lumen_packet_t txt_start_curePacket   = { txt_start_cureAddress, kString }; // Start cure label text
static lumen_packet_t txt_langPacket         = { txt_langAddress, kString };   // Language label text
static lumen_packet_t list_langPacket        = { list_langAddress, kS32 };     // Language list index
static lumen_packet_t txt_adminPacket        = { txt_adminAddress, kString };  // Admin label text
static lumen_packet_t txt_systemPacket       = { txt_systemAddress, kString }; // System label text
static lumen_packet_t start_glaze_curePacket = { start_glaze_cureAddress, kString }; // Start glaze cure label text
static lumen_packet_t pre_cure_1Packet       = { pre_cure_1Address, kS32 };    // Pre-cure stage 1 time in seconds
static lumen_packet_t pre_cure_2Packet       = { pre_cure_2Address, kS32 };    // Pre-cure stage 2 time in seconds
static lumen_packet_t pre_cure_3Packet       = { pre_cure_3Address, kS32 };    // Pre-cure stage 3 time in seconds
static lumen_packet_t pre_cure_4Packet       = { pre_cure_4Address, kS32 };    // Pre-cure stage 4 time in seconds
static lumen_packet_t pre_cure_5Packet       = { pre_cure_5Address, kS32 };    // Pre-cure stage 5 time in seconds
static lumen_packet_t pre_cure_6Packet       = { pre_cure_6Address, kS32 };    // Pre-cure stage 6 time in seconds
static lumen_packet_t pre_cure_7Packet       = { pre_cure_7Address, kS32 };    // Pre-cure stage 7 time in seconds
static lumen_packet_t txt_secondsPacket      = { txt_secondsAddress, kString }; // Seconds label text
static lumen_packet_t selected_pre_curePacket = { selected_pre_cureAddress, kS32 }; // Selected pre-cure time in seconds
static lumen_packet_t time_curandoPacket     = { time_curandoAddress,      kS32 }; // Elapsed cure time in seconds
static lumen_packet_t timer_start_stopPacket = { timer_start_stopAddress,  kS32 }; // Timer state
static lumen_packet_t progress_permillePacket= { progress_permilleAddress, kS32 }; // Progress bar value (permille)

#endif // USER_VARIABLES_H
