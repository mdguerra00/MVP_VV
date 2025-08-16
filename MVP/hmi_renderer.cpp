#include <Arduino.h>
#include <string.h>
#include "LumenProtocol.h"
#include "hmi_addresses.h"
#include "hmi_bindings.h"
#include "hmi_renderer.h"
#include "smartcure_translations.h"

// ===== Helpers de escrita =====
static bool HMI_WriteString(uint16_t addr, const char* text) {
  if (!text) text = "";
  const size_t len = strlen(text) + 1;
  uint32_t sent = 0;
  if (len <= MAX_STRING_SIZE) {
    lumen_packet_t p = { addr, kString };
    memset(p.data._string, 0, sizeof(p.data._string));
    memcpy(p.data._string, text, len);
    sent = lumen_write_packet(&p);
  } else {
    sent = lumen_write(addr, (uint8_t*)text, (uint32_t)len);
  }
  if (sent == 0) {
    Serial.printf("[HMI] Failed to write string addr=%u\n", addr);
    return false;
  }
  return true;
}

static bool HMI_WriteS32(uint16_t addr, int32_t v) {
  lumen_packet_t p = { addr, kS32 };
  p.data._s32 = v;
  uint32_t sent = lumen_write_packet(&p);
  if (sent == 0) {
    Serial.printf("[HMI] Failed to write S32 addr=%u\n", addr);
    return false;
  }
  return true;
}

static bool HMI_WriteListItem(uint16_t listAddr, uint16_t index, const char* text) {
  if (!text) text = "";
  const uint32_t len = (uint32_t)strlen(text) + 1;
  uint32_t sent = 0;
  if (len <= MAX_STRING_SIZE) {
    sent = lumen_write_variable_list(listAddr, index, (uint8_t*)text, len);
  } else {
    uint8_t buf[MAX_STRING_SIZE];
    memcpy(buf, text, MAX_STRING_SIZE - 1);
    buf[MAX_STRING_SIZE - 1] = '\0';
    sent = lumen_write_variable_list(listAddr, index, buf, MAX_STRING_SIZE);
  }
  if (sent == 0) {
    Serial.printf("[HMI] Failed to write list item addr=%u idx=%u\n", listAddr, index);
    return false;
  }
  delay(2);
  return true;
}

static void HMI_ClearListTail(uint16_t listAddr, uint16_t fromIndex, uint16_t toIndex) {
  static const char empty[] = "";
  for (uint16_t i = fromIndex; i <= toIndex; ++i) {
    lumen_write_variable_list(listAddr, i, (uint8_t*)empty, 1);
    delay(2);
  }
}

// ===== API =====
void HMI_FillLanguageList() {
  HMI_WriteListItem(ADDR_LIST_LANG, 0, getString(LANG_EN, ID_SETTINGS_LANGUAGE_EN)); // English
  HMI_WriteListItem(ADDR_LIST_LANG, 1, getString(LANG_EN, ID_SETTINGS_LANGUAGE_PT)); // Português
  HMI_WriteListItem(ADDR_LIST_LANG, 2, getString(LANG_EN, ID_SETTINGS_LANGUAGE_ES)); // Español
  HMI_WriteListItem(ADDR_LIST_LANG, 3, getString(LANG_EN, ID_SETTINGS_LANGUAGE_DE)); // Deutsch
  if (4 < MAX_LIST_SIZE) HMI_ClearListTail(ADDR_LIST_LANG, 4, MAX_LIST_SIZE - 1);
}

void HMI_RenderBindings(Language L, const HmiBinding* B, size_t N) {
  for (size_t i=0; i<N; ++i) {
    HMI_WriteString(B[i].addr, getString(L, B[i].id));
  }
}

void HMI_RenderHome(Language L) {
  HMI_RenderBindings(L, HOME_BINDINGS, sizeof(HOME_BINDINGS)/sizeof(HOME_BINDINGS[0]));
}

void HMI_RenderSettings(Language L) {
  HMI_RenderBindings(L, SETTINGS_BINDINGS, sizeof(SETTINGS_BINDINGS)/sizeof(SETTINGS_BINDINGS[0]));
}

void HMI_RenderAll(Language L) {
  HMI_RenderHome(L);
  HMI_RenderSettings(L);
}

void HMI_SyncLangVarToHMI(Language L) {
  int idx = (L==LANG_EN)?0 : (L==LANG_PT)?1 : (L==LANG_ES)?2 : 3;
  HMI_WriteS32(ADDR_LANG_VAR, idx);
}
