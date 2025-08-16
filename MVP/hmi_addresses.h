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
