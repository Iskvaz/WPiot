/*
 * Exemplo GroupMessaging - WhatsApp ESP32
 *
 * Este exemplo demonstra como:
 * - Enviar mensagens para grupos do WhatsApp
 * - Receber mensagens de grupos
 * - Identificar remetente e grupo
 * - Responder apenas no grupo ou em privado
 * - Criar comandos específicos para grupos
 *
 * IMPORTANTE: Para enviar mensagens em grupos, você precisa:
 * 1. Ter a instância WhatsApp conectada
 * 2. Estar participando do grupo
 * 3. Obter o ID do grupo (veja instruções abaixo)
 *
 * Como obter o ID do grupo:
 * - Envie qualquer mensagem no grupo via WhatsApp
 * - Execute este código e veja no Serial Monitor
 * - O ID terá formato: 123456789@g.us
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========
// WiFi
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// API WhatsApp
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";                 // Sua API Key
const char* instanceKey = "minha-instancia";        // Nome da sua instância

// IDs dos grupos (obtenha executando o código uma vez)
// Formato: 123456789@g.us (ou apenas 123456789, o @g.us é adicionado automaticamente)
const char* grupoFamilia = "123456789@g.us";       // Grupo da família
const char* grupoTrabalho = "987654321@g.us";      // Grupo do trabalho
const char* grupoIoT = "555555555@g.us";           // Grupo de IoT/Automação

// Configurações de hardware
#define LED_STATUS 2    // LED para indicar status do sistema

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// ========== VARIÁVEIS GLOBAIS ==========
unsigned long ultimoRelatorio = 0;
const unsigned long INTERVALO_RELATORIO = 3600000;  // 1 hora em ms

int contadorMensagens = 0;
float temperaturaSimulada = 25.0;

// ========== FUNÇÕES AUXILIARES ==========

// Verifica se a mensagem veio de um grupo
bool isGroupMessage(String from) {
  return from.indexOf("@g.us") != -1;
}

// Extrai o número do remetente de uma mensagem de grupo
// Formato: 5511999999999@c.us (na maioria dos casos o from já vem assim)
String extractSender(String from) {
  // Se for grupo, o from pode vir como numero@c.us ou grupo@g.us
  // Dependendo da API, pode precisar extrair de outra forma
  return from;
}

// Envia mensagem para todos os grupos configurados
void broadcastToGroups(String message) {
  Serial.println("\n📢 Enviando broadcast para grupos...");

  if (whatsapp.sendTextToGroup(grupoFamilia, message)) {
    Serial.println("✓ Enviado para: Grupo Família");
  }

  if (whatsapp.sendTextToGroup(grupoTrabalho, message)) {
    Serial.println("✓ Enviado para: Grupo Trabalho");
  }

  if (whatsapp.sendTextToGroup(grupoIoT, message)) {
    Serial.println("✓ Enviado para: Grupo IoT");
  }
}

// Envia relatório periódico para grupos
void enviarRelatorioGrupos() {
  String relatorio = "📊 *Relatório do Sistema ESP32*\n\n";
  relatorio += "⏰ Uptime: " + String(millis() / 1000) + "s\n";
  relatorio += "📨 Mensagens processadas: " + String(contadorMensagens) + "\n";
  relatorio += "🌡️ Temperatura simulada: " + String(temperaturaSimulada, 1) + "°C\n";
  relatorio += "📶 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  relatorio += "💾 Heap livre: " + String(ESP.getFreeHeap()) + " bytes\n";
  relatorio += "\n✅ Sistema funcionando normalmente";

  // Enviar apenas para grupo de IoT
  whatsapp.sendTextToGroup(grupoIoT, relatorio);
  Serial.println("✓ Relatório enviado para grupo IoT");
}

// ========== PROCESSAMENTO DE COMANDOS ==========

void processarComando(String from, String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  bool isGroup = isGroupMessage(from);

  Serial.println("\n📥 Processando comando: " + cmd);
  Serial.println("De: " + from);
  Serial.println("É grupo: " + String(isGroup ? "Sim" : "Não"));

  // ========== COMANDOS GERAIS ==========

  if (cmd == "menu" || cmd == "/menu") {
    String menu = "📋 *Menu de Comandos - Grupos*\n\n";
    menu += "*Informações:*\n";
    menu += "• /status - Status do sistema\n";
    menu += "• /relatorio - Relatório completo\n";
    menu += "• /temp - Temperatura atual\n";
    menu += "• /uptime - Tempo online\n\n";
    menu += "*Controle:*\n";
    menu += "• /led on - Liga LED\n";
    menu += "• /led off - Desliga LED\n\n";
    menu += "*Testes:*\n";
    menu += "• /ping - Testa conexão\n";
    menu += "• /echo [texto] - Repete texto\n";
    menu += "• /broadcast [texto] - Envia para todos os grupos\n\n";
    menu += "Prefixo / é opcional";

    whatsapp.sendText(from, menu);
  }

  else if (cmd == "status" || cmd == "/status") {
    String status = "📊 *Status do Sistema*\n\n";
    status += "✅ Sistema: Online\n";
    status += "📶 WiFi: Conectado (" + String(WiFi.RSSI()) + " dBm)\n";
    status += "⏱️ Uptime: " + String(millis() / 1000) + "s\n";
    status += "💾 Memória livre: " + String(ESP.getFreeHeap()) + " bytes\n";
    status += "💡 LED: " + String(digitalRead(LED_STATUS) ? "Ligado" : "Desligado");

    whatsapp.sendText(from, status);
  }

  else if (cmd == "relatorio" || cmd == "/relatorio") {
    enviarRelatorioGrupos();

    // Confirmar no grupo que enviou o comando
    if (isGroup) {
      whatsapp.sendText(from, "✅ Relatório completo enviado!");
    }
  }

  else if (cmd == "temp" || cmd == "/temp") {
    temperaturaSimulada = random(200, 350) / 10.0;  // Simula entre 20-35°C

    String msg = "🌡️ *Temperatura*\n\n";
    msg += "Atual: " + String(temperaturaSimulada, 1) + "°C";

    whatsapp.sendText(from, msg);
  }

  else if (cmd == "uptime" || cmd == "/uptime") {
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    String uptime = "⏱️ *Tempo Online*\n\n";
    uptime += String(days) + " dias, ";
    uptime += String(hours % 24) + " horas, ";
    uptime += String(minutes % 60) + " minutos";

    whatsapp.sendText(from, uptime);
  }

  else if (cmd == "ping" || cmd == "/ping") {
    whatsapp.sendText(from, "🏓 Pong! Sistema respondendo.");
  }

  // ========== CONTROLE DE LED ==========

  else if (cmd == "led on" || cmd == "/led on") {
    digitalWrite(LED_STATUS, HIGH);
    whatsapp.sendText(from, "💡 LED ligado!");
  }

  else if (cmd == "led off" || cmd == "/led off") {
    digitalWrite(LED_STATUS, LOW);
    whatsapp.sendText(from, "💡 LED desligado!");
  }

  // ========== COMANDOS COM PARÂMETROS ==========

  else if (cmd.startsWith("echo ") || cmd.startsWith("/echo ")) {
    String texto = cmd;
    texto.replace("echo ", "");
    texto.replace("/echo ", "");

    whatsapp.sendText(from, "🔊 " + texto);
  }

  else if (cmd.startsWith("broadcast ") || cmd.startsWith("/broadcast ")) {
    String texto = cmd;
    texto.replace("broadcast ", "");
    texto.replace("/broadcast ", "");

    broadcastToGroups("📢 *Mensagem Broadcast*\n\n" + texto);

    if (isGroup) {
      whatsapp.sendText(from, "✅ Mensagem enviada para todos os grupos!");
    }
  }

  // ========== COMANDO NÃO RECONHECIDO ==========

  else {
    whatsapp.sendText(from, "❓ Comando não reconhecido.\nEnvie *menu* para ver os comandos disponíveis.");
  }
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("   WhatsApp ESP32 - Group Messaging");
  Serial.println("========================================\n");

  // Configurar LED
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);

  // Conectar WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // Piscar LED enquanto conecta
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
  }

  digitalWrite(LED_STATUS, HIGH);
  Serial.println("\n✓ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar WhatsApp
  whatsapp.begin();
  whatsapp.setPollInterval(5000);  // Verificar mensagens a cada 5 segundos

  // Verificar conexão
  Serial.println("\nVerificando conexão com WhatsApp...");
  if (whatsapp.isConnected()) {
    Serial.println("✓ WhatsApp conectado!");
    Serial.print("Número: ");
    Serial.println(whatsapp.getPhoneNumber());
  } else {
    Serial.println("✗ WhatsApp não conectado!");
    Serial.println("Verifique se a instância está ativa na API");
  }

  Serial.println("\n========================================");
  Serial.println("CONFIGURAÇÃO DE GRUPOS");
  Serial.println("========================================");
  Serial.println("\nPara obter o ID dos grupos:");
  Serial.println("1. Envie qualquer mensagem no grupo via WhatsApp");
  Serial.println("2. O ID aparecerá no Serial Monitor quando o ESP32 receber");
  Serial.println("3. Copie o ID e atualize as variáveis no código:");
  Serial.println("   - grupoFamilia");
  Serial.println("   - grupoTrabalho");
  Serial.println("   - grupoIoT");
  Serial.println("\nFormato do ID: 123456789@g.us");

  Serial.println("\n========================================");
  Serial.println("Sistema pronto! Aguardando mensagens...");
  Serial.println("========================================\n");

  // Enviar mensagem de inicialização para grupos
  String msgInit = "🚀 *ESP32 Sistema Online*\n\n";
  msgInit += "Sistema de monitoramento iniciado!\n";
  msgInit += "Envie */menu* para ver comandos disponíveis.";

  // Descomentar após configurar os IDs dos grupos
  // broadcastToGroups(msgInit);

  Serial.println("💡 Dica: Descomente a linha 'broadcastToGroups' no setup()");
  Serial.println("    após configurar os IDs dos grupos.\n");
}

// ========== LOOP ==========

void loop() {
  // Buffer para armazenar mensagens recebidas
  WhatsAppMessage messages[10];

  // Obter novas mensagens
  int count = whatsapp.getMessages(messages, 10);

  if (count > 0) {
    Serial.printf("\n📩 %d nova(s) mensagem(ns) recebida(s)!\n\n", count);

    for (int i = 0; i < count; i++) {
      contadorMensagens++;

      Serial.println("==================== MENSAGEM " + String(i + 1) + " ====================");
      Serial.print("ID: ");
      Serial.println(messages[i].id);
      Serial.print("De: ");
      Serial.println(messages[i].from);
      Serial.print("Texto: ");
      Serial.println(messages[i].text);
      Serial.print("Timestamp: ");
      Serial.println(messages[i].timestamp);

      // Identificar se é grupo
      if (isGroupMessage(messages[i].from)) {
        Serial.println("📱 Tipo: GRUPO");
        Serial.println("\n⚠️ IMPORTANTE: Copie o ID acima e configure no código!");
        Serial.println("   Exemplo: const char* meuGrupo = \"" + messages[i].from + "\";");
      } else {
        Serial.println("📱 Tipo: PRIVADO");
      }

      Serial.println("====================================================\n");

      // Processar comando
      processarComando(messages[i].from, messages[i].text);
    }
  }

  // Enviar relatório periódico
  if (millis() - ultimoRelatorio > INTERVALO_RELATORIO) {
    enviarRelatorioGrupos();
    ultimoRelatorio = millis();
  }

  // Aguardar 5 segundos antes de verificar novamente
  delay(5000);
}
