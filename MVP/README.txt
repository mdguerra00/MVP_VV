SmartCURE MVP (ESP32 + UnicView Studio via Lumen Protocol)
===========================================================

O que este pacote faz
---------------------
- Troca de idioma controlada pelo ESP32 (config salva em /config.json no SPIFFS).
- Atualiza os rótulos "Configurações/Settings" e "Iniciar/Start" na HMI.
- Escuta mudanças da variável `Lang` vindas da HMI (0=PT, 1=EN) e salva a escolha.

Arquivos incluídos
------------------
- SmartCURE_MVP_Lumen/SmartCURE_MVP_Lumen.ino  -> sketch principal
- SmartCURE_MVP_Lumen/example_config.json      -> exemplo de config (opcional)

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

Fluxo de teste
--------------
1) Compile e grave o sketch no ESP32.
2) Garanta que a HMI está com UART0 + Lumen Protocol (115200 8N1, sem CRC/Ack).
3) Na HMI, altere a variável `Lang` (ex.: em um botão PT/EN com "Set Variable").
4) Veja o rótulo mudar:
   - `txt_Config`: "Configurações" / "Settings"
   - `txt_Start` : "Iniciar" / "Start"
5) A mudança de idioma é salva em /config.json no ESP32.

Suporte
-------
Se as funções do Lumen geradas pelo UnicView tiverem nomes diferentes,
abra o arquivo lumen_protocol.h e ajuste as chamadas no .ino.
