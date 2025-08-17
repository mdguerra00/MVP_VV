#pragma once
#include "user_variables.h"
#include "smartcure_translations.h"

struct HmiBinding { uint16_t addr; StringId id; };

// ===== Tela Home =====
static const HmiBinding HOME_BINDINGS[] = {
  { ADDR_TXT_START,  ID_HOME_STARTCURE },   // 124
  { ADDR_TXT_CONFIG, ID_SETTINGS_TITLE },   // 122
  { ADDR_TXT_LANG,   ID_SETTINGS_LANGUAGE }, // 125
  { ADDR_TXT_ADMIN,  ID_HOME_ADMIN }, // 125
};

// ===== Tela Settings (exemplo) =====
static const HmiBinding SETTINGS_BINDINGS[] = {
  { ADDR_TXT_CONFIG, ID_SETTINGS_TITLE },     // 122
  { ADDR_TXT_SYSTEM, ID_HOME_MONITOR },  // 128 (use o que corresponder na sua HMI)
  { ADDR_TXT_LANG,   ID_SETTINGS_LANGUAGE },   // 125
};
