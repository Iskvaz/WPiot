/*
 * Exemplo Básico - WhatsApp ESP32
 *
 * Este exemplo demonstra como:
 * - Conectar à rede WiFi
 * - Inicializar a biblioteca WhatsApp
 * - Receber mensagens
 * - Responder automaticamente
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========
// WiFi
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// API WhatsApp
const char* apiUrl = "https://app.wpiot.com.br";  // IP do servidor da API
const char* apiKey = "SUA_API_KEY";                 // Sua API Key
const char* instanceKey = "minha-instancia";        // Nome da sua instância

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("  WhatsApp ESP32 - Exemplo Básico");
  Serial.println("========================================\n");

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
  Serial.println("Sistema pronto! Aguardando mensagens...");
  Serial.println("========================================\n");
}

void loop() {
  // Buffer para armazenar mensagens recebidas
  WhatsAppMessage messages[10];

  // Obter novas mensagens
  int count = whatsapp.getMessages(messages, 10);

  if (count > 0) {
    Serial.printf("\n📩 %d nova(s) mensagem(ns) recebida(s)!\n\n", count);

    for (int i = 0; i < count; i++) {
      Serial.println("------------------");
      Serial.print("De: ");
      Serial.println(messages[i].from);
      Serial.print("Mensagem: ");
      Serial.println(messages[i].text);
      Serial.println("------------------\n");

      // Responder automaticamente
      String response = "Olá! Recebi sua mensagem: \"" + messages[i].text + "\"";

      if (whatsapp.sendText(messages[i].from, response)) {
        Serial.println("✓ Resposta enviada com sucesso!");
      } else {
        Serial.println("✗ Erro ao enviar resposta");
      }

      // Nota: Mensagens já foram removidas da fila automaticamente ao serem lidas
    }
  }

  // Aguardar 5 segundos antes de verificar novamente
  delay(5000);
}
