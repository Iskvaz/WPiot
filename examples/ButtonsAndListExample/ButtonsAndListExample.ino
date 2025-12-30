/*
 * Exemplo Botões e Listas - WhatsApp ESP32
 *
 * Este exemplo demonstra como enviar:
 * - Mensagens com botões
 * - Mensagens com lista de opções
 * - Localização
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://app.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========== WhatsApp ESP32 - Botões e Listas ==========\n");

  // Conectar WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✓ WiFi conectado!");

  // Inicializar WhatsApp
  whatsapp.begin();
  Serial.println("✓ Sistema pronto!\n");
}

void sendButtonsExample(String to) {
  Serial.println("Enviando mensagem com botões...");

  String buttons[] = {
    "Opção 1",
    "Opção 2",
    "Opção 3"
  };

  if (whatsapp.sendButtons(to, "Escolha uma opção:", buttons, 3)) {
    Serial.println("✓ Botões enviados!");
  } else {
    Serial.println("✗ Erro ao enviar botões");
  }
}

void sendListExample(String to) {
  Serial.println("Enviando lista de opções...");

  // Criar seções
  String section1Rows[] = {"Pizza Margherita", "Pizza Calabresa", "Pizza Portuguesa"};
  String section2Rows[] = {"Coca-Cola", "Guaraná", "Suco Natural"};

  ListSection sections[2];

  // Seção 1 - Pizzas
  sections[0].title = "🍕 Pizzas";
  sections[0].rows = section1Rows;
  sections[0].rowCount = 3;

  // Seção 2 - Bebidas
  sections[1].title = "🥤 Bebidas";
  sections[1].rows = section2Rows;
  sections[1].rowCount = 3;

  if (whatsapp.sendList(to, "Faça seu pedido", "Cardápio", "Ver Cardápio", sections, 2)) {
    Serial.println("✓ Lista enviada!");
  } else {
    Serial.println("✗ Erro ao enviar lista");
  }
}

void sendLocationExample(String to) {
  Serial.println("Enviando localização...");

  // Exemplo: Coordenadas de São Paulo
  float latitude = -23.550520;
  float longitude = -46.633308;

  if (whatsapp.sendLocation(to, latitude, longitude)) {
    Serial.println("✓ Localização enviada!");
  } else {
    Serial.println("✗ Erro ao enviar localização");
  }
}

void processMessage(String from, String text) {
  text.toLowerCase();
  text.trim();

  if (text == "botoes" || text == "botões") {
    sendButtonsExample(from);
  }
  else if (text == "lista" || text == "cardapio" || text == "cardápio") {
    sendListExample(from);
  }
  else if (text == "local" || text == "localizacao" || text == "localização") {
    sendLocationExample(from);
  }
  else if (text == "menu") {
    String menu = "📱 *Exemplos Disponíveis*\n\n";
    menu += "Digite um dos comandos:\n\n";
    menu += "🔘 *botoes* - Enviar botões\n";
    menu += "📋 *lista* - Enviar lista de opções\n";
    menu += "📍 *local* - Enviar localização\n";

    whatsapp.sendText(from, menu);
  }
  else {
    whatsapp.sendText(from, "Digite *menu* para ver os exemplos disponíveis");
  }
}

void loop() {
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      Serial.printf("De: %s - Msg: %s\n", messages[i].from.c_str(), messages[i].text.c_str());

      // Processar mensagem
      processMessage(messages[i].from, messages[i].text);

      // Nota: Mensagens já foram removidas automaticamente
    }
  }

  delay(5000);
}
