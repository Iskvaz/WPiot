/*
 * Exemplo Menu Interativo - WhatsApp ESP32
 *
 * Este exemplo demonstra como criar um menu interativo
 * que responde a comandos específicos do usuário
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== DEFINIÇÕES ==========
#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // LED onboard na maioria das placas ESP32
#endif

// ========== CONFIGURAÇÕES ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========== WhatsApp ESP32 - Menu Interativo ==========\n");

  // Conectar WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✓ WiFi conectado!");

  // Configurar LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Inicializar WhatsApp
  whatsapp.begin();
  whatsapp.setPollInterval(3000);

  Serial.println("✓ Sistema pronto!\n");
}

void processCommand(String from, String command) {
  command.toLowerCase();
  command.trim();

  Serial.print("Processando comando: ");
  Serial.println(command);

  if (command == "menu" || command == "ajuda" || command == "help") {
    // Enviar menu principal
    String menu = "🤖 *Menu de Comandos*\n\n";
    menu += "Digite um dos comandos abaixo:\n\n";
    menu += "📋 *menu* - Exibir este menu\n";
    menu += "📊 *status* - Ver status do ESP32\n";
    menu += "🌡️ *temperatura* - Ler temperatura\n";
    menu += "💡 *led on* - Ligar LED\n";
    menu += "💡 *led off* - Desligar LED\n";
    menu += "ℹ️ *info* - Informações do sistema\n";

    whatsapp.sendText(from, menu);
  }
  else if (command == "status") {
    String status = "✅ *Status do ESP32*\n\n";
    status += "📶 WiFi: Conectado\n";
    status += "📱 WhatsApp: Online\n";
    status += "🔋 Uptime: " + String(millis() / 1000) + "s\n";
    status += "💾 RAM livre: " + String(ESP.getFreeHeap() / 1024) + " KB";

    whatsapp.sendText(from, status);
  }
  else if (command == "temperatura") {
    // Simular leitura de sensor
    float temp = random(200, 350) / 10.0;  // 20.0 a 35.0

    String msg = "🌡️ *Temperatura Atual*\n\n";
    msg += "Temperatura: " + String(temp, 1) + "°C";

    whatsapp.sendText(from, msg);
  }
  else if (command == "led on") {
    digitalWrite(LED_BUILTIN, HIGH);
    whatsapp.sendText(from, "💡 LED ligado com sucesso!");
  }
  else if (command == "led off") {
    digitalWrite(LED_BUILTIN, LOW);
    whatsapp.sendText(from, "💡 LED desligado com sucesso!");
  }
  else if (command == "info") {
    String info = "ℹ️ *Informações do Sistema*\n\n";
    info += "Chip: " + String(ESP.getChipModel()) + "\n";
    info += "Cores: " + String(ESP.getChipCores()) + "\n";
    info += "Frequência: " + String(ESP.getCpuFreqMHz()) + " MHz\n";
    info += "Flash: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB\n";
    info += "SDK: " + String(ESP.getSdkVersion());

    whatsapp.sendText(from, info);
  }
  else {
    // Comando não reconhecido
    String msg = "❌ Comando não reconhecido!\n\n";
    msg += "Digite *menu* para ver os comandos disponíveis.";

    whatsapp.sendText(from, msg);
  }
}

void loop() {
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      Serial.printf("De: %s - Msg: %s\n", messages[i].from.c_str(), messages[i].text.c_str());

      // Processar comando
      processCommand(messages[i].from, messages[i].text);

      // Nota: Mensagens já foram removidas automaticamente
    }
  }

  delay(3000);
}
