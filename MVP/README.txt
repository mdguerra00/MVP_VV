SmartCURE MVP (ESP32 + UnicView Studio via Lumen Protocol)
===========================================================

O que este pacote faz
---------------------
- Troca de idioma controlada pelo ESP32 (config salva em /config.json no SPIFFS).
- Atualiza os rótulos "Configurações/Settings" e "Iniciar/Start" na HMI.
- Escuta mudanças da variável `Lang` vindas da HMI (0=PT, 1=EN, 2=ES, 3=DE) e salva a escolha.
- Controle de tempo de cura (start/pause/resume/stop).
- Atualização automática de barra de progresso (0–1000).

Arquivos incluídos
------------------
- MVP/MVP.ino  -> sketch principal
- MVP/config.json      -> exemplo de config (opcional)

Arquivos que você deve adicionar (do UnicView Studio)
-----------------------------------------------------
1) No UnicView Studio, vá em:
   Communication Settings -> Code Template (Protocol: Lumen Protocol, Arduino/C)
2) Gere/exporte os arquivos:
   - lumen_protocol.h
   - lumen_protocol.c
3) Coloque esses dois arquivos na mesma pasta deste .ino

Mapeamento de variáveis (confirme no seu projeto UnicView)
-----------------------------------------------------------
- txt_Config  (Basic Text) -> Address 122  (Host -> HMI / Write)
- Lang        (Integer)    -> Address 123  (HMI -> Host / Auto-send on change)
- txt_Start   (Basic Text) -> Address 124  (Host -> HMI / Write)
- selected_pre_cure (Integer) -> Address 138 (HMI -> Host / Auto-send on change)
- time_curando     (Integer) -> Address 139 (Host -> HMI / Write)
- timer_start_stop (Integer) -> Address 140 (HMI -> Host / Auto-send on change)
- progress_permille (Integer) -> Address 141 (Host -> HMI / Write)

Variáveis do temporizador
-------------------------
- `selected_pre_cure` (138): duração escolhida do preset de pré-cura (segundos).
- `time_curando` (139): tempo decorrido enviado para a HMI (segundos).
- `timer_start_stop` (140): estado do contador (0=stop, 1=start/resume, 3=pause).
- `progress_permille` (141): barra de progresso de 0 a 1000.

Ligações físicas sugeridas
--------------------------
- ESP32 RX2 (GPIO16)  <- TX do display (UART0)
- ESP32 TX2 (GPIO17)  -> RX do display (UART0)
- GND comum
Baud: 115200, 8N1 (CRC/Ack desativados)

SPIFFS
------
O sketch cria /config.json se ele não existir. Opcionalmente você pode subir o
arquivo example_config.json renomeado para config.json usando a ferramenta de upload.

Conteúdo esperado de /config.json:
{
  "lang": "PT"
}
ou
{
  "lang": "EN"
}
ou
{
  "lang": "DE"
}

Fluxo de teste
--------------
1) Compile e grave o sketch no ESP32.
2) Garanta que a HMI está com UART0 + Lumen Protocol (115200 8N1, sem CRC/Ack).
3) Selecione um preset de pré-cura na HMI (`selected_pre_cure`).
4) Inicie o contador (`timer_start_stop` = 1) e observe `time_curando` contar em segundos.
5) Pause (`timer_start_stop` = 3) e retome (`timer_start_stop` = 1) para validar start/pause/resume.
6) Pare o contador (`timer_start_stop` = 0) e confira o reset dos valores.
7) Verifique se a barra de progresso acompanha o ciclo (`progress_permille` de 0 a 1000).

Suporte
-------
Se as funções do Lumen geradas pelo UnicView tiverem nomes diferentes,
abra o arquivo lumen_protocol.h e ajuste as chamadas no .ino.
