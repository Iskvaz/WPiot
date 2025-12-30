/*
 * Exemplo Controle de Pinos - WhatsApp ESP32
 *
 * Este exemplo demonstra como controlar pinos digitais do ESP32 via WhatsApp:
 * - Ligar/desligar LEDs
 * - Controlar relés
 * - Ler status de entradas digitais (botões, sensores)
 * - Menu interativo com status em tempo real
 *
 * Conexões Sugeridas:
 * - LED 1: GPIO 2 (LED onboard)
 * - LED 2: GPIO 4
 * - Relé 1: GPIO 5
 * - Relé 2: GPIO 18
 * - Botão 1: GPIO 19 (pull-up interno)
 * - Botão 2: GPIO 21 (pull-up interno)
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES WIFI ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ========== CONFIGURAÇÕES API ==========
const char* apiUrl = "https://app.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== DEFINIÇÃO DOS PINOS ==========
// Saídas Digitais (LEDs/Relés)
#define LED1_PIN 2
#define LED2_PIN 4
#define RELE1_PIN 5
#define RELE2_PIN 18

// Entradas Digitais (Botões/Sensores)
#define BTN1_PIN 19
#define BTN2_PIN 21

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// ========== VARIÁVEIS ==========
bool led1Status = false;
bool led2Status = false;
bool rele1Status = false;
bool rele2Status = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  WhatsApp ESP32 - Controle de Pinos");
  Serial.println("========================================\n");

  // Configurar pinos de saída
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(RELE1_PIN, OUTPUT);
  pinMode(RELE2_PIN, OUTPUT);

  // Configurar pinos de entrada com pull-up
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);

  // Inicializar todas as saídas desligadas
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(RELE1_PIN, LOW);
  digitalWrite(RELE2_PIN, LOW);

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
  whatsapp.setPollInterval(3000);

  Serial.println("\n✓ Sistema pronto!\n");
  Serial.println("Comandos disponíveis:");
  Serial.println("  menu - Ver menu de comandos");
  Serial.println("  status - Ver status dos pinos");
  Serial.println("  led1 on/off - Controlar LED 1");
  Serial.println("  led2 on/off - Controlar LED 2");
  Serial.println("  rele1 on/off - Controlar Relé 1");
  Serial.println("  rele2 on/off - Controlar Relé 2");
  Serial.println();
}

String getStatusEmoji(bool status) {
  return status ? "🟢 ON" : "🔴 OFF";
}

String getStatusMessage() {
  String msg = "📊 *Status dos Dispositivos*\n\n";

  msg += "*SAÍDAS DIGITAIS:*\n";
  msg += "💡 LED 1: " + getStatusEmoji(led1Status) + "\n";
  msg += "💡 LED 2: " + getStatusEmoji(led2Status) + "\n";
  msg += "🔌 Relé 1: " + getStatusEmoji(rele1Status) + "\n";
  msg += "🔌 Relé 2: " + getStatusEmoji(rele2Status) + "\n\n";

  msg += "*ENTRADAS DIGITAIS:*\n";
  bool btn1 = digitalRead(BTN1_PIN) == LOW;
  bool btn2 = digitalRead(BTN2_PIN) == LOW;
  msg += "🔘 Botão 1: " + getStatusEmoji(btn1) + "\n";
  msg += "🔘 Botão 2: " + getStatusEmoji(btn2) + "\n";

  return msg;
}

String getMenuMessage() {
  String msg = "🎛️ *Menu de Controle*\n\n";
  msg += "*Comandos Disponíveis:*\n\n";
  msg += "📋 *status* - Ver status atual\n";
  msg += "💡 *led1 on* - Ligar LED 1\n";
  msg += "💡 *led1 off* - Desligar LED 1\n";
  msg += "💡 *led2 on* - Ligar LED 2\n";
  msg += "💡 *led2 off* - Desligar LED 2\n";
  msg += "🔌 *rele1 on* - Ligar Relé 1\n";
  msg += "🔌 *rele1 off* - Desligar Relé 1\n";
  msg += "🔌 *rele2 on* - Ligar Relé 2\n";
  msg += "🔌 *rele2 off* - Desligar Relé 2\n";
  msg += "🔄 *todos on* - Ligar tudo\n";
  msg += "⭕ *todos off* - Desligar tudo\n";

  return msg;
}

void processCommand(String from, String cmd) {
  cmd.toLowerCase();
  cmd.trim();

  Serial.printf("Comando recebido: '%s'\n", cmd.c_str());

  // Menu
  if (cmd == "menu" || cmd == "ajuda" || cmd == "help") {
    whatsapp.sendText(from, getMenuMessage());
  }
  // Status
  else if (cmd == "status" || cmd == "estado") {
    whatsapp.sendText(from, getStatusMessage());
  }
  // LED 1
  else if (cmd == "led1 on" || cmd == "led 1 on") {
    digitalWrite(LED1_PIN, HIGH);
    led1Status = true;
    whatsapp.sendText(from, "💡 LED 1 ligado!");
  }
  else if (cmd == "led1 off" || cmd == "led 1 off") {
    digitalWrite(LED1_PIN, LOW);
    led1Status = false;
    whatsapp.sendText(from, "💡 LED 1 desligado!");
  }
  // LED 2
  else if (cmd == "led2 on" || cmd == "led 2 on") {
    digitalWrite(LED2_PIN, HIGH);
    led2Status = true;
    whatsapp.sendText(from, "💡 LED 2 ligado!");
  }
  else if (cmd == "led2 off" || cmd == "led 2 off") {
    digitalWrite(LED2_PIN, LOW);
    led2Status = false;
    whatsapp.sendText(from, "💡 LED 2 desligado!");
  }
  // Relé 1
  else if (cmd == "rele1 on" || cmd == "rele 1 on" || cmd == "relé1 on") {
    digitalWrite(RELE1_PIN, HIGH);
    rele1Status = true;
    whatsapp.sendText(from, "🔌 Relé 1 ligado!");
  }
  else if (cmd == "rele1 off" || cmd == "rele 1 off" || cmd == "relé1 off") {
    digitalWrite(RELE1_PIN, LOW);
    rele1Status = false;
    whatsapp.sendText(from, "🔌 Relé 1 desligado!");
  }
  // Relé 2
  else if (cmd == "rele2 on" || cmd == "rele 2 on" || cmd == "relé2 on") {
    digitalWrite(RELE2_PIN, HIGH);
    rele2Status = true;
    whatsapp.sendText(from, "🔌 Relé 2 ligado!");
  }
  else if (cmd == "rele2 off" || cmd == "rele 2 off" || cmd == "relé2 off") {
    digitalWrite(RELE2_PIN, LOW);
    rele2Status = false;
    whatsapp.sendText(from, "🔌 Relé 2 desligado!");
  }
  // Todos
  else if (cmd == "todos on" || cmd == "tudo on" || cmd == "ligar tudo") {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(RELE1_PIN, HIGH);
    digitalWrite(RELE2_PIN, HIGH);
    led1Status = led2Status = rele1Status = rele2Status = true;
    whatsapp.sendText(from, "🟢 Todos os dispositivos ligados!");
  }
  else if (cmd == "todos off" || cmd == "tudo off" || cmd == "desligar tudo") {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(RELE1_PIN, LOW);
    digitalWrite(RELE2_PIN, LOW);
    led1Status = led2Status = rele1Status = rele2Status = false;
    whatsapp.sendText(from, "🔴 Todos os dispositivos desligados!");
  }
  // Comando não reconhecido
  else {
    String msg = "❌ Comando não reconhecido!\n\n";
    msg += "Digite *menu* para ver os comandos disponíveis.";
    whatsapp.sendText(from, msg);
  }
}

void loop() {
  // Receber mensagens
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      Serial.printf("De: %s - Msg: %s\n", messages[i].from.c_str(), messages[i].text.c_str());

      // Processar comando
      processCommand(messages[i].from, messages[i].text);
    }
  }

  delay(3000);
}
