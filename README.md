# MVP_VV

A referência para a interface gráfica pode ser encontrada em <https://tela-magica-tft.lovable.app/>.

Visão geral
O MVP_VV demonstra um sistema de pós‑cura dentária baseado em ESP32 que comunica com uma HMI criada no UnicView Studio por meio do Lumen Protocol. O firmware inicializa a serial, lê a configuração persistida em SPIFFS, envia textos para a HMI e reage a eventos que alteram o idioma exibido.

Estrutura do projeto
Caminho	Função
MVP/MVP.ino	Sketch principal: configura UART2, monta o SPIFFS, carrega /config.json, renderiza textos e trata pacotes da HMI
MVP/user_variables.h	Lista os endereços dos widgets e variáveis da HMI (labels, variável de idioma, lista de idiomas etc.)
MVP/hmi_bindings.h	Associa cada endereço da HMI a um StringId que identifica a string a ser traduzida
MVP/hmi_renderer.cpp	Funções utilitárias para escrever strings/inteiros na HMI, preencher a lista de idiomas e renderizar telas completas de acordo com o idioma corrente
MVP/smartcure_translations.h	Enumera idiomas (Language) e identificadores de texto (StringId), além de agrupar tabelas com as traduções em memória
MVP/config.json	Configuração persistente que armazena o último idioma selecionado
en.json, pt.json, es.json, de.json	Arquivos de referência das traduções; os dados são espelhados no firmware para uso imediato
MVP/LumenProtocol.*	Biblioteca gerada pelo UnicView para implementação do Lumen Protocol na plataforma Arduino/ESP32
Controle de Cura
A HMI controla o ciclo de cura selecionando um preset e comandando o temporizador. As variáveis usadas nessa troca são:

* **`selected_pre_cure` (138)** – preset de pré‑cura escolhido pela HMI.
* **`timer_start_stop` (140)** – comando do temporizador: `0`=stop, `1`=run, `3`=pause.
* **`time_curando` (139)** – tempo decorrido de cura devolvido pelo ESP32.
* **`progress_permille` (141)** – progresso em permilagem (0–1000) para alimentar a barra de progresso.

Para exibir a barra de progresso, crie a `progress_permille` como *User Variable* do tipo **S32** no endereço **141**, com faixa de 0 a 1000. Os presets `pre_cure_1…7` podem ser atualizados diretamente pela HMI para ajustar os tempos de cada etapa.
Fluxo de execução
Inicialização

UART2 é configurada nos pinos GPIO16 (RX) e GPIO17 (TX), com baud 115200, 8N1, sem CRC/Ack

SPIFFS é montado e, em seguida, a lista de idiomas da HMI é preenchida com os rótulos de cada idioma suportado

Carregamento do idioma

O firmware lê /config.json e extrai o índice do idioma (EN, PT, ES, DE); caso não exista, assume português como padrão

Loop principal

Pacotes Lumen são lidos continuamente. Ao receber eventos nos endereços da lista de idiomas ou da variável Lang, o código aplica o novo idioma, renderiza todos os textos associados e salva a escolha se necessário. Além disso, mudanças em `timer_start_stop` disparam o temporizador de cura, que atualiza `time_curando` e `progress_permille` enquanto o ciclo estiver em execução.

Renderização

HMI_RenderAll escreve os textos traduzidos nas telas Home e Settings; HMI_SyncLangVarToHMI mantém a variável Lang (endereço 123) sincronizada com o idioma atual

Conjunto de traduções
As traduções são mantidas em arrays C++ para acesso rápido e são indexadas por enums:

Language define os quatro idiomas atualmente suportados

StringId enumera cada texto exibido na interface, permitindo mapeamentos consistentes em todas as telas

Além disso, os arquivos JSON na raiz (en.json, pt.json, etc.) servem como referência legível e podem ser usados para gerar ou validar as tabelas internas de tradução.

Persistência e arquivos externos
A seleção de idioma é persistida em /config.json, garantindo que o usuário retorne ao último idioma utilizado em reinicializações posteriores

O projeto requer os arquivos LumenProtocol.c e LumenProtocol.h gerados pelo UnicView Studio (via Communication Settings → Code Template), que devem ser colocados na pasta do sketch

Como expandir
Adicionar novos textos ou telas

Criar novos StringId em smartcure_translations.h, inserir as traduções correspondentes e mapear o endereço da HMI em user_variables.h.

Associar o endereço ao StringId em hmi_bindings.h e chamar a função de renderização adequada em hmi_renderer.cpp.

Suportar mais idiomas

Incluir o novo idioma no enum Language, atualizar as tabelas de tradução e ajustar HMI_FillLanguageList para preencher a lista com a nova opção.

Persistir outras configurações

Reaproveitar o padrão de leitura/escrita de config.json para armazenar outros parâmetros de sistema.

Ajustar o temporizador de cura

O controle via `timer_start_stop` pode ser estendido para novos modos de operação ou etapas adicionais de cura, reutilizando `progress_permille` para exibir o avanço de cada fase.

Integração física

Seguir o padrão de ligação: RX2/TX2 do ESP32 ao UART0 do display e GND comum; conferir a configuração da porta serial e do protocolo no software da HMI

Referências adicionais
O repositório contém um diagrama interativo da interface em <https://tela-magica-tft.lovable.app/>.

Mais detalhes de instalação e teste estão descritos em MVP/README.txt, incluindo fluxo de teste e dicas de suporte

Este documento consolida os principais componentes e o fluxo de funcionamento do protótipo, servindo como base para futuras expansões de funcionalidades, telas e idiomas.
