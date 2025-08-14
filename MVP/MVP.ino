#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <string.h>
#include "LumenProtocol.h"
#include "smartcure_translations.h"  // UTF-8 dos seus JSONs

// ===== UART / HMI =====
#define HMI_RX   16
#define HMI_TX   17
#define HMI_BAUD 115200
HardwareSerial HMIserial(2);

// ===== Endereços na HMI =====
static const uint16_t ADDR_TXT_CONFIG  = 122; // Basic Text (String)
static const uint16_t ADDR_LANG_VAR    = 123; // SignedInt32: 0=EN,1=PT,2=ES (vamos só escrever)
static const uint16_t ADDR_TXT_START   = 124; // Basic Text (String)
static const uint16_t ADDR_TXT_LANG    = 125; // Basic Text (String)
static const uint16_t ADDR_LIST_LANG   = 126; // Basic Text List (StringList)
static const uint16_t MAX_LIST_SIZE    = 10;

// ===== Transporte Lumen =====
extern "C" void lumen_write_bytes(uint8_t *data, uint32_t length) { HMIserial.write(data, length); }
extern "C" uint16_t lumen_get_byte() { return HMIserial.available() ? HMIserial.read() : DATA_NULL; }

// ===== Helpers =====
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
  delay(3);
  return true;
}

static void HMI_ClearListTail(uint16_t listAddr, uint16_t fromIndex, uint16_t toIndex) {
  static const char empty[] = "";
  for (uint16_t i = fromIndex; i <= toIndex; ++i) {
    uint32_t sent = lumen_write_variable_list(listAddr, i, (uint8_t*)empty, 1);
    if (sent == 0) {
      Serial.printf("[HMI] Failed to clear list item addr=%u idx=%u\n", listAddr, i);
    }
    delay(2);
  }
}

static bool HMI_WriteS32(uint16_t addr, int32_t val) {
  lumen_packet_t p = { addr, kS32 };
  p.data._s32 = val;
  uint32_t sent = lumen_write_packet(&p);
  if (sent == 0) {
    Serial.printf("[HMI] Failed to write S32 addr=%u\n", addr);
    return false;
  }
  return true;
}

// ===== Idioma =====
static Language currentLang = LANG_PT; // default local até aplicarmos config
static inline Language mapLangVar(int32_t v) {
  switch (v) { case 0: return LANG_EN; case 1: return LANG_PT; case 2: return LANG_ES; default: return LANG_EN; }
}
static inline int32_t unmapLangVar(Language L) {
  switch (L) { case LANG_PT: return 1; case LANG_ES: return 2; case LANG_EN: default: return 0; }
}

static void HMI_FillLanguageList() {
  const char* nameEN = getString(LANG_EN, ID_SETTINGS_LANGUAGE_EN); // "English"
  const char* namePT = getString(LANG_EN, ID_SETTINGS_LANGUAGE_PT); // "Português"
  const char* nameES = getString(LANG_EN, ID_SETTINGS_LANGUAGE_ES); // "Español"
  HMI_WriteListItem(ADDR_LIST_LANG, 0, nameEN);
  HMI_WriteListItem(ADDR_LIST_LANG, 1, namePT);
  HMI_WriteListItem(ADDR_LIST_LANG, 2, nameES);
  if (3 < MAX_LIST_SIZE) HMI_ClearListTail(ADDR_LIST_LANG, 3, MAX_LIST_SIZE - 1);
}

static void HMI_RenderHome(Language L) {
  HMI_WriteString(ADDR_TXT_START,  getString(L, ID_HOME_STARTCURE));   // 124
  HMI_WriteString(ADDR_TXT_CONFIG, getString(L, ID_SETTINGS_TITLE));   // 122
  HMI_WriteString(ADDR_TXT_LANG,   getString(L, ID_SETTINGS_LANGUAGE));// 125
}

// ===== config.json (SPIFFS) =====
static bool readFileToString(const char* path, String &out) {
  File f = SPIFFS.open(path, "r");
  if (!f) return false;
  out.reserve(f.size() + 16);
  out = "";
  while (f.available()) out += (char)f.read();
  f.close();
  return true;
}
// Parser minimalista para {"lang":"EN"/"PT"/"ES"/"DE"}
static int32_t parseLangIndexFromJson(const String& json) {
  String lower = json; lower.toLowerCase();
  int pos = lower.indexOf("\"lang\"");
  if (pos < 0) pos = lower.indexOf("'lang'");
  if (pos < 0) return -1;
  int colon = lower.indexOf(':', pos);
  if (colon < 0) return -1;
  int q1 = lower.indexOf('"', colon), q1a = lower.indexOf('\'', colon);
  int q = -1, qend = -1;
  if (q1 >= 0 && (q1a < 0 || q1 < q1a)) { q = q1; qend = lower.indexOf('"', q + 1); }
  else if (q1a >= 0) { q = q1a; qend = lower.indexOf('\'', q + 1); }
  if (q < 0 || qend <= q) return -1;
  String val = lower.substring(q + 1, qend); val.trim();
  if (val == "en") return 0;
  if (val == "pt") return 1;
  if (val == "es") return 2;
  return -1;
}

// ===== Aplicação de idioma (sem ler 123) =====
static void applyLanguageIdx(int32_t idx, bool mirrorToHMI) {
  idx = constrain(idx, 0, 2);
  currentLang = mapLangVar(idx);
  if (mirrorToHMI && !HMI_WriteS32(ADDR_LANG_VAR, idx)) {
    Serial.println("[HMI] Failed to mirror language variable");
  }
  HMI_RenderHome(currentLang);
  Serial.printf("[LANG] aplicado=%ld (espelhado=%s)\n", (long)idx, mirrorToHMI?"sim":"nao");
}

// ===== Setup / Loop =====
void setup() {
  Serial.begin(115200);
  HMIserial.begin(HMI_BAUD, SERIAL_8N1, HMI_RX, HMI_TX);

  // Filesystem para ler config.json
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount falhou; seguindo com defaults.");
  }

  delay(800); // HMI inicializa

  HMI_FillLanguageList(); // 126

  // 1) lê config.json
  int32_t cfgIdx = -1;
  String js;
  if (readFileToString("/config.json", js)) {
    cfgIdx = parseLangIndexFromJson(js);
    Serial.printf("config.json: lang=%ld (%s)\n", (long)cfgIdx,
                  cfgIdx==0?"EN":cfgIdx==1?"PT":cfgIdx==2?"ES":"?");
  } else {
    Serial.println("config.json não encontrado; usando PT");
    cfgIdx = 1;
  }

  // 2) aplica e espelha 123
  applyLanguageIdx(cfgIdx < 0 ? 1 : cfgIdx, /*mirrorToHMI=*/true);

  Serial.println("HMI pronta.");
}

void loop() {
  // Eventos vindos da HMI
  while (lumen_available() > 0) {
    lumen_packet_t* p = lumen_get_first_packet();
    if (!p) break;

    // Preferimos evento da LISTA (126) como fonte de verdade da UI
    if (p->address == ADDR_LIST_LANG) {
      int32_t idx = 0;
      if (p->type == kS32)      idx = p->data._s32;
      else if (p->type == kU16) idx = (int32_t)p->data._u16;
      else if (p->type == kU32) idx = (int32_t)p->data._u32;
      else continue;
      idx = constrain(idx, 0, 2);
      // aplica e espelha 123
      applyLanguageIdx(idx, /*mirrorToHMI=*/true);
    }

    // Se por acaso vier evento da 123, também aceitamos:
    if (p->address == ADDR_LANG_VAR) {
      int32_t idx = 0;
      if (p->type == kS32)      idx = p->data._s32;
      else if (p->type == kU16) idx = (int32_t)p->data._u16;
      else if (p->type == kU32) idx = (int32_t)p->data._u32;
      else continue;
      idx = constrain(idx, 0, 2);
      // aplica, mas sem reescrever 123 (já veio dela)
      applyLanguageIdx(idx, /*mirrorToHMI=*/false);
    }
  }

  // (sem polling: não dependemos de leitura de 123)
}
