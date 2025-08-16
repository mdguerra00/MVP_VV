#pragma once
#include <stddef.h>
#include "hmi_bindings.h"
#include "smartcure_translations.h"

// Preenche lista 126 com "English, Português, Español, Deutsch"
void HMI_FillLanguageList();

// Render genérico (liga bindings)
void HMI_RenderBindings(Language L, const HmiBinding* B, size_t N);

// Atalhos de telas
void HMI_RenderHome(Language L);
void HMI_RenderSettings(Language L);

// Render tudo que já estiver mapeado (chamado ao trocar idioma)
void HMI_RenderAll(Language L);

// Espelha o índice do idioma na var 123 (0..3)
void HMI_SyncLangVarToHMI(Language L);
