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

