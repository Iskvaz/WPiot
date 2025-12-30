/*
 * Exemplo Monitoramento de Sensores - WhatsApp ESP32
 *
 * Este exemplo demonstra como monitorar sensores analógicos e digitais,
 * enviando alertas automáticos via WhatsApp quando valores ultrapassam limites.
 *
 * Sensores Suportados:
 * - DHT22 (Temperatura e Umidade)
 * - LDR (Sensor de Luminosidade)
 * - Sensor de Solo (Umidade do solo)
 * - Potenciômetro (Entrada analógica genérica)
 * - PIR (Sensor de movimento)
 *
 * Conexões Sugeridas:
 * - DHT22: GPIO 4
 * - LDR: GPIO 34 (ADC1_CH6)
 * - Solo: GPIO 35 (ADC1_CH7)
 * - Potenciômetro: GPIO 32 (ADC1_CH4)
 * - PIR: GPIO 15
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"
#include "DHT.h"

// ========== CONFIGURAÇÕES WIFI ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ========== CONFIGURAÇÕES API ==========
const char* apiUrl = "https://app.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== DEFINIÇÃO DOS PINOS ==========
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define SOLO_PIN 35
#define POT_PIN 32
#define PIR_PIN 15

// ========== LIMITES DE ALERTA ==========
#define TEMP_MAX 30.0      // Temperatura máxima (°C)
#define TEMP_MIN 15.0      // Temperatura mínima (°C)
#define UMID_MIN 30.0      // Umidade mínima (%)
#define LUZ_MIN 20         // Luminosidade mínima (%)
#define SOLO_MIN 30        // Umidade do solo mínima (%)

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);
DHT dht(DHT_PIN, DHT_TYPE);

// ========== VARIÁVEIS ==========
float temperatura = 0;
float umidade = 0;
int luminosidade = 0;
int umidadeSolo = 0;
int potenciometro = 0;
bool movimento = false;

unsigned long lastSensorRead = 0;
unsigned long lastAlertCheck = 0;
const unsigned long SENSOR_INTERVAL = 5000;    // Ler sensores a cada 5s
const unsigned long ALERT_INTERVAL = 60000;    // Verificar alertas a cada 1min

String numeroAlerta = "";  // Número para enviar alertas automáticos

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  WhatsApp ESP32 - Monitor de Sensores");
  Serial.println("========================================\n");

  // Configurar pinos
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(SOLO_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

  // Inicializar DHT
  dht.begin();

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
  Serial.println("  leituras - Ver leituras atuais");
  Serial.println("  alertas on - Ativar alertas automáticos");
  Serial.println("  alertas off - Desativar alertas");
  Serial.println("  relatorio - Relatório completo");
  Serial.println();
}

void lerSensores() {
  // Ler DHT22
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();

  // Ler sensores analógicos (0-4095)
  int ldrValue = analogRead(LDR_PIN);
  int soloValue = analogRead(SOLO_PIN);
  potenciometro = analogRead(POT_PIN);

  // Converter para porcentagem
  luminosidade = map(ldrValue, 0, 4095, 0, 100);
  umidadeSolo = map(soloValue, 0, 4095, 100, 0);  // Invertido: menor valor = mais úmido

  // Ler PIR
  movimento = digitalRead(PIR_PIN) == HIGH;

  // Debug no Serial
  Serial.println("\n--- Leituras dos Sensores ---");
  Serial.printf("Temperatura: %.1f°C\n", temperatura);
  Serial.printf("Umidade: %.1f%%\n", umidade);
  Serial.printf("Luminosidade: %d%%\n", luminosidade);
  Serial.printf("Umidade Solo: %d%%\n", umidadeSolo);
  Serial.printf("Potenciômetro: %d\n", potenciometro);
  Serial.printf("Movimento: %s\n", movimento ? "SIM" : "NÃO");
  Serial.println("-----------------------------\n");
}

String getLeiturasMessage() {
  String msg = "📊 *Leituras dos Sensores*\n\n";

  if (!isnan(temperatura)) {
    msg += "🌡️ *Temperatura:* " + String(temperatura, 1) + "°C\n";
  } else {
    msg += "🌡️ *Temperatura:* Erro de leitura\n";
  }

  if (!isnan(umidade)) {
    msg += "💧 *Umidade Ar:* " + String(umidade, 1) + "%\n";
  } else {
    msg += "💧 *Umidade Ar:* Erro de leitura\n";
  }

  msg += "☀️ *Luminosidade:* " + String(luminosidade) + "%\n";
  msg += "🌱 *Umidade Solo:* " + String(umidadeSolo) + "%\n";
  msg += "🎚️ *Potenciômetro:* " + String(potenciometro) + "/4095\n";
  msg += "🚶 *Movimento:* " + String(movimento ? "Detectado" : "Não detectado") + "\n";

  return msg;
}

String getRelatorioMessage() {
  String msg = "📋 *Relatório Completo*\n\n";

  msg += "*SENSORES ANALÓGICOS:*\n";
  msg += "🌡️ Temperatura: " + String(temperatura, 1) + "°C ";
  if (temperatura > TEMP_MAX) msg += "⚠️ ALTA\n";
  else if (temperatura < TEMP_MIN) msg += "❄️ BAIXA\n";
  else msg += "✅ Normal\n";

  msg += "💧 Umidade: " + String(umidade, 1) + "% ";
  if (umidade < UMID_MIN) msg += "⚠️ BAIXA\n";
  else msg += "✅ Normal\n";

  msg += "☀️ Luminosidade: " + String(luminosidade) + "% ";
  if (luminosidade < LUZ_MIN) msg += "🌙 Escuro\n";
  else msg += "✅ Claro\n";

  msg += "🌱 Umidade Solo: " + String(umidadeSolo) + "% ";
  if (umidadeSolo < SOLO_MIN) msg += "⚠️ SECO\n";
  else msg += "✅ Úmido\n";

  msg += "\n*SENSORES DIGITAIS:*\n";
  msg += "🚶 Movimento: " + String(movimento ? "Detectado 🚨" : "Não detectado ✅") + "\n";

  msg += "\n*SISTEMA:*\n";
  msg += "📡 WiFi: Conectado ✅\n";
  msg += "💾 RAM Livre: " + String(ESP.getFreeHeap() / 1024) + " KB\n";
  msg += "⏱️ Uptime: " + String(millis() / 1000) + " s\n";

  return msg;
}

void verificarAlertas() {
  if (numeroAlerta == "") return;  // Alertas desativados

  bool alertaEnviado = false;
  String msg = "⚠️ *ALERTA AUTOMÁTICO*\n\n";

  // Verificar temperatura
  if (temperatura > TEMP_MAX) {
    msg += "🔥 Temperatura ALTA: " + String(temperatura, 1) + "°C\n";
    alertaEnviado = true;
  } else if (temperatura < TEMP_MIN) {
    msg += "❄️ Temperatura BAIXA: " + String(temperatura, 1) + "°C\n";
    alertaEnviado = true;
  }

  // Verificar umidade
  if (umidade < UMID_MIN) {
    msg += "💧 Umidade BAIXA: " + String(umidade, 1) + "%\n";
    alertaEnviado = true;
  }

  // Verificar luminosidade
  if (luminosidade < LUZ_MIN) {
    msg += "🌙 Ambiente ESCURO: " + String(luminosidade) + "%\n";
    alertaEnviado = true;
  }

  // Verificar solo
  if (umidadeSolo < SOLO_MIN) {
    msg += "🌱 Solo SECO: " + String(umidadeSolo) + "%\n";
    alertaEnviado = true;
  }

  // Verificar movimento
  if (movimento) {
    msg += "🚨 MOVIMENTO DETECTADO!\n";
    alertaEnviado = true;
  }

  if (alertaEnviado) {
    whatsapp.sendText(numeroAlerta, msg);
    Serial.println("⚠️ Alerta enviado!");
  }
}

void processCommand(String from, String cmd) {
  cmd.toLowerCase();
  cmd.trim();

  Serial.printf("Comando recebido: '%s'\n", cmd.c_str());

  if (cmd == "leituras" || cmd == "sensores" || cmd == "status") {
    whatsapp.sendText(from, getLeiturasMessage());
  }
  else if (cmd == "relatorio" || cmd == "relatório" || cmd == "report") {
    whatsapp.sendText(from, getRelatorioMessage());
  }
  else if (cmd == "alertas on" || cmd == "alertas ativar") {
    numeroAlerta = from;
    whatsapp.sendText(from, "✅ Alertas automáticos ATIVADOS!\n\nVocê receberá alertas quando:\n🔥 Temperatura > 30°C\n❄️ Temperatura < 15°C\n💧 Umidade < 30%\n🌙 Luminosidade < 20%\n🌱 Solo < 30%\n🚨 Movimento detectado");
  }
  else if (cmd == "alertas off" || cmd == "alertas desativar") {
    numeroAlerta = "";
    whatsapp.sendText(from, "🔕 Alertas automáticos DESATIVADOS!");
  }
  else if (cmd == "menu" || cmd == "ajuda" || cmd == "help") {
    String msg = "📱 *Menu de Comandos*\n\n";
    msg += "📊 *leituras* - Leituras atuais\n";
    msg += "📋 *relatorio* - Relatório completo\n";
    msg += "🔔 *alertas on* - Ativar alertas\n";
    msg += "🔕 *alertas off* - Desativar alertas\n";
    whatsapp.sendText(from, msg);
  }
  else {
    whatsapp.sendText(from, "❌ Comando não reconhecido!\n\nDigite *menu* para ver os comandos.");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Ler sensores periodicamente
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;
    lerSensores();
  }

  // Verificar alertas periodicamente
  if (currentMillis - lastAlertCheck >= ALERT_INTERVAL) {
    lastAlertCheck = currentMillis;
    verificarAlertas();
  }

  // Receber mensagens WhatsApp
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      Serial.printf("De: %s - Msg: %s\n", messages[i].from.c_str(), messages[i].text.c_str());
      processCommand(messages[i].from, messages[i].text);
    }
  }

  delay(3000);
}
