#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <limits.h>
#include "LumenProtocol.h"

#include "user_variables.h"
#include "hmi_bindings.h"
#include "hmi_renderer.h"
#include "smartcure_translations.h"

// Config
#define HMI_RX     16
#define HMI_TX     17
#define HMI_BAUD   115200
#define DEBUG_SNIFF 1   // 1 = mostra todos os pacotes recebidos; 0 = silencioso

HardwareSerial HMIserial(2);

// Transporte Lumen
extern "C" void lumen_write_bytes(uint8_t *data, uint32_t length){ HMIserial.write(data, length); }
extern "C" uint16_t lumen_get_byte(){ return HMIserial.available() ? HMIserial.read() : DATA_NULL; }

// ====== Lógica de cura ======
const uint16_t selected_pre_cureAddress = 138;
const uint16_t time_curandoAddress      = 139;
const uint16_t timer_start_stopAddress  = 140;
const uint16_t progress_permilleAddress = 141; // extra para barra de progresso

enum CureState { STATE_IDLE = 0, STATE_RUNNING = 1, STATE_PAUSED = 2 };
static CureState cureState = STATE_IDLE;

static uint32_t target_time_s = 0;
static uint64_t start_ms = 0;
static uint64_t pause_start_ms = 0;
static uint64_t accumulated_pause_ms = 0;

static int32_t last_time_reported = -1;
static int32_t last_progress_reported = -1;

static uint32_t pre_cure_values[7] = {6, 15, 30, 60, 90, 120, 180};

static lumen_packet_t selected_pre_curePacket = { selected_pre_cureAddress, kS32 };
static lumen_packet_t time_curandoPacket     = { time_curandoAddress,     kS32 };
static lumen_packet_t timer_start_stopPacket = { timer_start_stopAddress, kS32 };
static lumen_packet_t progress_permillePacket= { progress_permilleAddress,kS32 };

static inline void writeInt(lumen_packet_t* packet, int32_t value){
  packet->type = kS32;
  packet->data._s32 = value;
  lumen_write_packet(packet);
}

static void startCure(){
  if (target_time_s == 0) return;
  cureState = STATE_RUNNING;
  start_ms = millis();
  accumulated_pause_ms = 0;
  last_time_reported = -1;
  last_progress_reported = -1;
  writeInt(&timer_start_stopPacket, 1);
}

static void pauseCure(){
  if (cureState == STATE_RUNNING){
    cureState = STATE_PAUSED;
    pause_start_ms = millis();
    writeInt(&timer_start_stopPacket, 3);
  }
}

static void resumeCure(){
  if (cureState == STATE_PAUSED){
    uint64_t now = millis();
    accumulated_pause_ms += now - pause_start_ms;
    cureState = STATE_RUNNING;
    writeInt(&timer_start_stopPacket, 1);
  }
}

static void stopCure(){
  cureState = STATE_IDLE;
  writeInt(&time_curandoPacket, 0);
  writeInt(&progress_permillePacket, 0);
  writeInt(&timer_start_stopPacket, 0);
}

// Estado do idioma
static Language currentLang = LANG_PT;

// ==== Utilitários de idioma ====
static inline Language mapLangVar(int32_t v){
  switch (v){ case 0: return LANG_EN; case 1: return LANG_PT; case 2: return LANG_ES; case 3: return LANG_DE; default: return LANG_EN; }
}
static inline int32_t unmapLangVar(Language L){
  switch (L){ case LANG_PT: return 1; case LANG_ES: return 2; case LANG_DE: return 3; case LANG_EN: default: return 0; }
}

// ==== Persistência (config.json) ====
static bool readFileToString(const char* path, String &out){
  File f = SPIFFS.open(path, "r");
  if (!f) return false;
  out.reserve(f.size() + 16); out = "";
  while (f.available()) out += (char)f.read();
  f.close(); return true;
}
static int32_t parseLangIndexFromJson(const String& json){
  String lower = json; lower.toLowerCase();
  int pos = lower.indexOf("\"lang\""); if (pos < 0) pos = lower.indexOf("'lang'");
  if (pos < 0) return -1;
  int colon = lower.indexOf(':', pos); if (colon < 0) return -1;
  int q1 = lower.indexOf('"', colon), q1a = lower.indexOf('\'', colon);
  int q=-1, qend=-1;
  if (q1>=0 && (q1a<0 || q1<q1a)) { q=q1; qend=lower.indexOf('"', q+1); }
  else if (q1a>=0){ q=q1a; qend=lower.indexOf('\'', q+1); }
  if (q<0 || qend<=q) return -1;
  String val = lower.substring(q+1, qend); val.trim();
  if (val=="en") return 0; if (val=="pt") return 1; if (val=="es") return 2; if (val=="de") return 3;
  return -1;
}

// ==== Aplicar idioma + render ====
static void applyLanguageIdx(int32_t idx, bool mirrorToHMI){
  idx = constrain(idx, 0, 3);
  currentLang = mapLangVar(idx);
  if (mirrorToHMI) HMI_SyncLangVarToHMI(currentLang);  // espelha 123
  HMI_RenderAll(currentLang);                          // atualiza tudo que estiver mapeado
  Serial.printf("[LANG] aplicado=%ld (espelhado=%s)\n", (long)idx, mirrorToHMI?"sim":"nao");
}

// ==== Compat: leitura de pacotes (seu Lumen não tem read_packet) ====
static bool lumen_read_packet_compat(lumen_packet_t &out){
  if (lumen_available() > 0){
    lumen_packet_t* p = lumen_get_first_packet();
    if (p){ out = *p; return true; }
  }
  return false;
}

// Extrator tolerante (funciona mesmo com type==0)
static int32_t extractIndexLoose(const lumen_packet_t& p){
  switch (p.type){
    case kS32: return p.data._s32;
    case kU32: return (int32_t)p.data._u32;
    case kS16: return (int32_t)p.data._s16;
    case kU16: return (int32_t)p.data._u16;
    case kS8:  return (int32_t)p.data._s8;
    case kU8:  return (int32_t)p.data._u8;
    default: break;
  }
  int32_t cands[6] = {
    p.data._s32, (int32_t)p.data._u32, (int32_t)p.data._s16,
    (int32_t)p.data._u16, (int32_t)p.data._s8, (int32_t)p.data._u8
  };
  for (int i=0;i<6;i++){ if (cands[i] >= 0 && cands[i] <= 3) return cands[i]; }
  const char* s = (const char*)p.data._string;
  if (s && s[0]>='0' && s[0]<='9'){ int v = atoi(s); if (v>=0 && v<=3) return v; }
  return INT32_MIN;
}

// ==== Setup / Loop ====
void setup(){
  Serial.begin(115200);
  HMIserial.begin(HMI_BAUD, SERIAL_8N1, HMI_RX, HMI_TX);

  if (!SPIFFS.begin(true)) Serial.println("SPIFFS mount falhou; seguindo com defaults.");

  delay(800);                 // HMI sobe
  HMI_FillLanguageList();     // popula 126

  lumen_write(&langPacket, 0);                 // idioma default = inglês
  lumen_write(&txt_start_curePacket, "Start Cure");

  // Preenche presets de cura e zera estado
  writeInt(&pre_cure_1Packet, pre_cure_values[0]);
  writeInt(&pre_cure_2Packet, pre_cure_values[1]);
  writeInt(&pre_cure_3Packet, pre_cure_values[2]);
  writeInt(&pre_cure_4Packet, pre_cure_values[3]);
  writeInt(&pre_cure_5Packet, pre_cure_values[4]);
  writeInt(&pre_cure_6Packet, pre_cure_values[5]);
  writeInt(&pre_cure_7Packet, pre_cure_values[6]);
  writeInt(&selected_pre_curePacket, 0);
  stopCure();

  // Carrega idioma inicial
  int32_t cfgIdx = -1; String js;
  if (readFileToString("/config.json", js)){
    cfgIdx = parseLangIndexFromJson(js);
    Serial.printf("config.json: lang=%ld (%s)\n", (long)cfgIdx,
      cfgIdx==0?"EN":cfgIdx==1?"PT":cfgIdx==2?"ES":cfgIdx==3?"DE":"?");
  } else {
    Serial.println("config.json não encontrado; usando PT");
    cfgIdx = 1;
  }
  applyLanguageIdx(cfgIdx < 0 ? 1 : cfgIdx, /*mirrorToHMI=*/true);
  Serial.println("HMI pronta.");
}

void loop(){
  lumen_packet_t pkt;
  while (lumen_read_packet_compat(pkt)) {
#if DEBUG_SNIFF
    Serial.printf("[RX] addr=%u type=%u S32=%ld U32=%lu S16=%d U16=%u S8=%d U8=%u STR=\"%s\"\n",
      pkt.address, (unsigned)pkt.type,
      (long)pkt.data._s32, (unsigned long)pkt.data._u32,
      (int)pkt.data._s16, (unsigned)pkt.data._u16,
      (int)pkt.data._s8, (unsigned)pkt.data._u8,
      (const char*)pkt.data._string);
#endif

    uint16_t addr = pkt.address;
    int32_t value = 0;
    switch (pkt.type){
      case kS32: value = pkt.data._s32; break;
      case kU32: value = (int32_t)pkt.data._u32; break;
      case kS16: value = (int32_t)pkt.data._s16; break;
      case kU16: value = (int32_t)pkt.data._u16; break;
      case kS8:  value = (int32_t)pkt.data._s8;  break;
      case kU8:  value = (int32_t)pkt.data._u8;  break;
      default: break;
    }

    if (addr == langListPacket.address || addr == langPacket.address){
      int32_t idx = extractIndexLoose(pkt);
      if (idx != INT32_MIN){
        bool mirror = (addr == langListPacket.address); // vindo da lista, espelha 123
        applyLanguageIdx(constrain(idx,0,3), mirror);
      } else {
        Serial.println("[EVT] pacote de idioma sem valor reconhecível.");
      }
    } else if (addr == selected_pre_cureAddress){
      target_time_s = (value > 0) ? (uint32_t)value : 0;
      writeInt(&selected_pre_curePacket, target_time_s);
    } else if (addr == timer_start_stopAddress){
      if (value == 0){
        stopCure();
      } else if (value == 1 || value == 2){
        if (cureState == STATE_IDLE) startCure();
        else if (cureState == STATE_PAUSED) resumeCure();
      } else if (value == 3){
        pauseCure();
      }
    } else if (addr == pre_cure_1Address){
      pre_cure_values[0] = (value > 0) ? (uint32_t)value : pre_cure_values[0];
    } else if (addr == pre_cure_2Address){
      pre_cure_values[1] = (value > 0) ? (uint32_t)value : pre_cure_values[1];
    } else if (addr == pre_cure_3Address){
      pre_cure_values[2] = (value > 0) ? (uint32_t)value : pre_cure_values[2];
    } else if (addr == pre_cure_4Address){
      pre_cure_values[3] = (value > 0) ? (uint32_t)value : pre_cure_values[3];
    } else if (addr == pre_cure_5Address){
      pre_cure_values[4] = (value > 0) ? (uint32_t)value : pre_cure_values[4];
    } else if (addr == pre_cure_6Address){
      pre_cure_values[5] = (value > 0) ? (uint32_t)value : pre_cure_values[5];
    } else if (addr == pre_cure_7Address){
      pre_cure_values[6] = (value > 0) ? (uint32_t)value : pre_cure_values[6];
    }
  }

  if (cureState == STATE_RUNNING){
    uint64_t now = millis();
    uint64_t elapsed_ms = now - start_ms - accumulated_pause_ms;
    uint32_t elapsed_s = elapsed_ms / 1000UL;

    if ((int32_t)elapsed_s != last_time_reported){
      last_time_reported = (int32_t)elapsed_s;
      writeInt(&time_curandoPacket, last_time_reported);
    }

    if (target_time_s > 0){
      int32_t progress = (int32_t)((elapsed_ms * 1000ULL) / (target_time_s * 1000ULL));
      if (progress > 1000) progress = 1000;
      if (progress != last_progress_reported){
        last_progress_reported = progress;
        writeInt(&progress_permillePacket, progress);
      }

      if (elapsed_s >= target_time_s){
        writeInt(&time_curandoPacket, (int32_t)target_time_s);
        writeInt(&progress_permillePacket, 1000);
        stopCure();
      }
    }
  }
}
