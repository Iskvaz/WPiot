/*
 * Exemplo GroupManagement - WhatsApp ESP32
 *
 * Este exemplo demonstra como:
 * - Gerenciar grupos do WhatsApp
 * - Identificar mensagens de grupos vs privadas
 * - Criar comandos específicos para administração
 * - Monitorar atividades em grupos
 * - Enviar mensagens formatadas e relatórios
 * - Implementar controle de acesso por grupo
 *
 * IMPORTANTE:
 * - Sua instância precisa ser ADMIN do grupo para algumas operações
 * - Nem todas as operações podem ser feitas via API
 * - Este exemplo foca em monitoramento e envio de mensagens
 *
 * Casos de uso:
 * - Bot de monitoramento para grupos de automação
 * - Sistema de alertas para grupos de trabalho
 * - Moderação automatizada
 * - Estatísticas e relatórios de grupo
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========
// WiFi
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// API WhatsApp
const char* apiUrl = "https://app.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== GRUPOS GERENCIADOS ==========

// Estrutura para armazenar informações de grupo
struct GrupoInfo {
  String id;
  String nome;
  bool monitorarAtividade;
  bool responderComandos;
  int totalMensagens;
  unsigned long ultimaMensagem;
};

// Configurar grupos gerenciados
// Obtenha os IDs enviando mensagens nos grupos e vendo no Serial Monitor
GrupoInfo grupos[] = {
  {"123456789@g.us", "Automação Residencial", true, true, 0, 0},
  {"987654321@g.us", "IoT Brasil", true, true, 0, 0},
  {"555555555@g.us", "Equipe Técnica", true, true, 0, 0}
};
const int numGrupos = 3;

// Lista de administradores autorizados (podem executar todos os comandos)
const char* adminsAutorizados[] = {
  "5511999999999",  // Admin 1
  "5511888888888"   // Admin 2
};
const int numAdmins = 2;

// ========== HARDWARE ==========
#define LED_ATIVIDADE 2
#define LED_ALERTA 4

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// ========== VARIÁVEIS GLOBAIS ==========
unsigned long ultimoRelatorio = 0;
const unsigned long INTERVALO_RELATORIO = 21600000;  // 6 horas

int totalMensagensProcessadas = 0;
int totalComandosExecutados = 0;

// ========== FUNÇÕES AUXILIARES ==========

// Verifica se é mensagem de grupo
bool isGrupo(String from) {
  return from.indexOf("@g.us") != -1;
}

// Busca informações do grupo pelo ID
GrupoInfo* getGrupoInfo(String grupoId) {
  for (int i = 0; i < numGrupos; i++) {
    if (grupos[i].id == grupoId) {
      return &grupos[i];
    }
  }
  return nullptr;
}

// Verifica se usuário é admin autorizado
bool isAdmin(String numero) {
  // Remove sufixos comuns
  numero.replace("@c.us", "");
  numero.replace("@s.whatsapp.net", "");

  for (int i = 0; i < numAdmins; i++) {
    if (numero.indexOf(adminsAutorizados[i]) != -1) {
      return true;
    }
  }
  return false;
}

// Verifica se o grupo aceita comandos
bool grupoAceitaComandos(String grupoId) {
  GrupoInfo* grupo = getGrupoInfo(grupoId);
  return grupo != nullptr && grupo->responderComandos;
}

// Atualiza estatísticas do grupo
void atualizarEstatisticas(String grupoId) {
  GrupoInfo* grupo = getGrupoInfo(grupoId);
  if (grupo != nullptr) {
    grupo->totalMensagens++;
    grupo->ultimaMensagem = millis();
  }
}

// ========== FUNÇÕES DE RESPOSTA ==========

// Envia menu de comandos
void enviarMenu(String para, bool isGrupoMsg) {
  String menu = "📋 *Menu de Comandos*\n\n";

  if (isGrupoMsg) {
    menu += "*Comandos de Grupo:*\n";
    menu += "• /stats - Estatísticas do grupo\n";
    menu += "• /sobre - Sobre este bot\n";
    menu += "• /help - Ajuda\n\n";
  }

  menu += "*Informações do Sistema:*\n";
  menu += "• /status - Status do ESP32\n";
  menu += "• /uptime - Tempo online\n";
  menu += "• /memoria - Uso de memória\n";
  menu += "• /wifi - Info WiFi\n\n";

  menu += "*Admin (apenas autorizados):*\n";
  menu += "• /relatorio - Relatório completo\n";
  menu += "• /broadcast [msg] - Envia para todos os grupos\n";
  menu += "• /listar grupos - Lista grupos gerenciados\n\n";

  menu += "_Prefixo / é opcional_";

  whatsapp.sendText(para, menu);
}

// Envia estatísticas do grupo
void enviarStatsGrupo(String grupoId) {
  GrupoInfo* grupo = getGrupoInfo(grupoId);

  if (grupo == nullptr) {
    whatsapp.sendText(grupoId, "❌ Este grupo não está na lista de gerenciados.");
    return;
  }

  String stats = "📊 *Estatísticas - " + grupo->nome + "*\n\n";
  stats += "📨 Mensagens processadas: " + String(grupo->totalMensagens) + "\n";

  if (grupo->ultimaMensagem > 0) {
    unsigned long tempoDesdeUltima = (millis() - grupo->ultimaMensagem) / 1000;
    stats += "⏰ Última mensagem: há " + String(tempoDesdeUltima) + "s\n";
  }

  stats += "📍 ID do grupo: `" + grupo->id + "`\n";
  stats += "✅ Status: " + String(grupo->monitorarAtividade ? "Monitorando" : "Inativo") + "\n";
  stats += "🤖 Comandos: " + String(grupo->responderComandos ? "Ativados" : "Desativados");

  whatsapp.sendText(grupoId, stats);
}

// Envia relatório completo (apenas admins)
void enviarRelatorioCompleto(String para) {
  String relatorio = "📊 *Relatório Completo do Sistema*\n\n";

  // Informações do sistema
  relatorio += "*Sistema ESP32:*\n";
  relatorio += "⏱️ Uptime: " + String(millis() / 1000) + "s\n";
  relatorio += "💾 Heap livre: " + String(ESP.getFreeHeap()) + " bytes\n";
  relatorio += "📶 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  relatorio += "📨 Total processadas: " + String(totalMensagensProcessadas) + "\n";
  relatorio += "⚡ Comandos executados: " + String(totalComandosExecutados) + "\n\n";

  // Estatísticas dos grupos
  relatorio += "*Grupos Gerenciados:*\n";
  for (int i = 0; i < numGrupos; i++) {
    relatorio += "\n• " + grupos[i].nome + "\n";
    relatorio += "  └ Mensagens: " + String(grupos[i].totalMensagens) + "\n";
    relatorio += "  └ Status: " + String(grupos[i].monitorarAtividade ? "✅ Ativo" : "⏸️ Pausado");

    if (i < numGrupos - 1) relatorio += "\n";
  }

  whatsapp.sendText(para, relatorio);
}

// Lista grupos gerenciados
void listarGrupos(String para) {
  String lista = "📋 *Grupos Gerenciados*\n\n";

  for (int i = 0; i < numGrupos; i++) {
    lista += String(i + 1) + ". " + grupos[i].nome + "\n";
    lista += "   └ ID: `" + grupos[i].id + "`\n";
    lista += "   └ Mensagens: " + String(grupos[i].totalMensagens) + "\n";
    lista += "   └ Status: " + String(grupos[i].monitorarAtividade ? "✅" : "⏸️") + "\n";

    if (i < numGrupos - 1) lista += "\n";
  }

  whatsapp.sendText(para, lista);
}

// Envia broadcast para todos os grupos
void enviarBroadcastGrupos(String mensagem, String remetente) {
  String msgFormatada = "📢 *Mensagem Broadcast*\n\n";
  msgFormatada += mensagem;
  msgFormatada += "\n\n_Enviado via ESP32 Bot_";

  Serial.println("\n📢 Enviando broadcast para grupos...");

  int enviados = 0;
  for (int i = 0; i < numGrupos; i++) {
    if (grupos[i].responderComandos) {
      if (whatsapp.sendTextToGroup(grupos[i].id, msgFormatada)) {
        Serial.println("✓ Enviado para: " + grupos[i].nome);
        enviados++;
        delay(1000);  // Delay entre envios
      }
    }
  }

  String confirmacao = "✅ Broadcast enviado para " + String(enviados) + " grupos!";
  whatsapp.sendText(remetente, confirmacao);
}

// ========== PROCESSAMENTO DE COMANDOS ==========

void processarComando(String from, String cmd, String remetenteOriginal) {
  cmd.trim();
  cmd.toLowerCase();

  bool isGrupoMsg = isGrupo(from);
  bool isAdminUser = isAdmin(remetenteOriginal);

  Serial.println("\n📥 Comando: " + cmd);
  Serial.println("De: " + from);
  Serial.println("Tipo: " + String(isGrupoMsg ? "Grupo" : "Privado"));
  Serial.println("Admin: " + String(isAdminUser ? "Sim" : "Não"));

  totalComandosExecutados++;

  // Verificar se grupo aceita comandos
  if (isGrupoMsg && !grupoAceitaComandos(from)) {
    Serial.println("⚠️ Grupo não aceita comandos");
    return;
  }

  // ========== COMANDOS GERAIS ==========

  if (cmd == "menu" || cmd == "/menu" || cmd == "help" || cmd == "/help") {
    enviarMenu(from, isGrupoMsg);
  }

  else if (cmd == "sobre" || cmd == "/sobre") {
    String sobre = "🤖 *ESP32 WhatsApp Bot*\n\n";
    sobre += "Bot de gerenciamento e monitoramento de grupos.\n\n";
    sobre += "*Recursos:*\n";
    sobre += "• Monitoramento de atividade\n";
    sobre += "• Estatísticas em tempo real\n";
    sobre += "• Sistema de comandos\n";
    sobre += "• Relatórios automáticos\n\n";
    sobre += "Desenvolvido com WhatsAppESP32";

    whatsapp.sendText(from, sobre);
  }

  else if (cmd == "stats" || cmd == "/stats") {
    if (isGrupoMsg) {
      enviarStatsGrupo(from);
    } else {
      whatsapp.sendText(from, "ℹ️ Este comando só funciona em grupos.");
    }
  }

  else if (cmd == "status" || cmd == "/status") {
    String status = "📊 *Status do Sistema*\n\n";
    status += "✅ ESP32: Online\n";
    status += "📱 WhatsApp: Conectado\n";
    status += "📶 WiFi: " + String(WiFi.RSSI()) + " dBm\n";
    status += "⏱️ Uptime: " + String(millis() / 1000) + "s\n";
    status += "📨 Processadas: " + String(totalMensagensProcessadas);

    whatsapp.sendText(from, status);
  }

  else if (cmd == "uptime" || cmd == "/uptime") {
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    String uptime = "⏱️ *Tempo Online*\n\n";
    uptime += String(days) + "d ";
    uptime += String(hours % 24) + "h ";
    uptime += String(minutes % 60) + "m ";
    uptime += String(seconds % 60) + "s";

    whatsapp.sendText(from, uptime);
  }

  else if (cmd == "memoria" || cmd == "/memoria") {
    String mem = "💾 *Memória*\n\n";
    mem += "Livre: " + String(ESP.getFreeHeap()) + " bytes\n";
    mem += "Total: " + String(ESP.getHeapSize()) + " bytes\n";
    mem += "Uso: " + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + "%";

    whatsapp.sendText(from, mem);
  }

  else if (cmd == "wifi" || cmd == "/wifi") {
    String wifi = "📶 *Informações WiFi*\n\n";
    wifi += "SSID: " + String(WiFi.SSID()) + "\n";
    wifi += "IP: " + WiFi.localIP().toString() + "\n";
    wifi += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    wifi += "Qualidade: ";

    int rssi = WiFi.RSSI();
    if (rssi > -50) wifi += "Excelente 📶";
    else if (rssi > -60) wifi += "Bom 📶";
    else if (rssi > -70) wifi += "Regular 📶";
    else wifi += "Fraco 📶";

    whatsapp.sendText(from, wifi);
  }

  // ========== COMANDOS ADMIN ==========

  else if (cmd == "relatorio" || cmd == "/relatorio") {
    if (isAdminUser) {
      enviarRelatorioCompleto(from);
    } else {
      whatsapp.sendText(from, "❌ Acesso negado. Apenas administradores.");
    }
  }

  else if (cmd == "listar grupos" || cmd == "/listar grupos") {
    if (isAdminUser) {
      listarGrupos(from);
    } else {
      whatsapp.sendText(from, "❌ Acesso negado. Apenas administradores.");
    }
  }

  else if (cmd.startsWith("broadcast ") || cmd.startsWith("/broadcast ")) {
    if (isAdminUser) {
      String mensagem = cmd;
      mensagem.replace("broadcast ", "");
      mensagem.replace("/broadcast ", "");
      enviarBroadcastGrupos(mensagem, from);
    } else {
      whatsapp.sendText(from, "❌ Acesso negado. Apenas administradores.");
    }
  }

  // ========== COMANDO NÃO RECONHECIDO ==========

  else {
    if (!isGrupoMsg) {  // Só responde erro em privado para não poluir grupo
      whatsapp.sendText(from, "❓ Comando não reconhecido.\nEnvie */menu* para ver comandos.");
    }
  }
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("  WhatsApp ESP32 - Group Management");
  Serial.println("========================================\n");

  // Configurar LEDs
  pinMode(LED_ATIVIDADE, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);
  digitalWrite(LED_ATIVIDADE, LOW);
  digitalWrite(LED_ALERTA, LOW);

  // Conectar WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✓ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar WhatsApp
  whatsapp.begin();
  whatsapp.setPollInterval(5000);

  if (whatsapp.isConnected()) {
    Serial.println("✓ WhatsApp conectado!");
  } else {
    Serial.println("✗ WhatsApp não conectado!");
  }

  // Mostrar configuração
  Serial.println("\n========================================");
  Serial.println("CONFIGURAÇÃO");
  Serial.println("========================================");
  Serial.println("\nGrupos gerenciados: " + String(numGrupos));
  for (int i = 0; i < numGrupos; i++) {
    Serial.println("  " + String(i + 1) + ". " + grupos[i].nome);
  }

  Serial.println("\nAdmins autorizados: " + String(numAdmins));
  for (int i = 0; i < numAdmins; i++) {
    Serial.println("  " + String(i + 1) + ". " + String(adminsAutorizados[i]));
  }

  Serial.println("\n⚠️ IMPORTANTE:");
  Serial.println("1. Atualize os IDs dos grupos no array 'grupos[]'");
  Serial.println("2. Configure os números dos admins em 'adminsAutorizados[]'");
  Serial.println("3. Para obter IDs de grupos, envie mensagens neles");

  Serial.println("\n========================================");
  Serial.println("Sistema pronto! Monitorando grupos...");
  Serial.println("========================================\n");
}

// ========== LOOP ==========

void loop() {
  WhatsAppMessage messages[10];
  int count = whatsapp.getMessages(messages, 10);

  if (count > 0) {
    digitalWrite(LED_ATIVIDADE, HIGH);

    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      totalMensagensProcessadas++;

      // Atualizar estatísticas se for grupo gerenciado
      if (isGrupo(messages[i].from)) {
        atualizarEstatisticas(messages[i].from);
      }

      // Log da mensagem
      Serial.println("\n--- Mensagem " + String(i + 1) + " ---");
      Serial.println("De: " + messages[i].from);
      Serial.println("Texto: " + messages[i].text);

      // Processar apenas se começar com / ou for comando conhecido
      String texto = messages[i].text;
      texto.trim();
      texto.toLowerCase();

      if (texto.startsWith("/") ||
          texto == "menu" || texto == "help" || texto == "status" ||
          texto == "sobre" || texto == "stats") {
        processarComando(messages[i].from, messages[i].text, messages[i].from);
      }
    }

    digitalWrite(LED_ATIVIDADE, LOW);
  }

  // Relatório periódico
  if (millis() - ultimoRelatorio > INTERVALO_RELATORIO) {
    Serial.println("\n⏰ Enviando relatório periódico...");

    for (int i = 0; i < numAdmins; i++) {
      enviarRelatorioCompleto(adminsAutorizados[i]);
      delay(1000);
    }

    ultimoRelatorio = millis();
  }

  delay(5000);
}
