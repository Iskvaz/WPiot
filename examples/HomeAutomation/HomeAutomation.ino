/*
 * Exemplo Automação Residencial - WhatsApp ESP32
 *
 * Sistema completo de automação residencial controlado por WhatsApp:
 * - Controle de iluminação (5 cômodos)
 * - Controle de temperatura/climatização
 * - Monitoramento de portas e janelas
 * - Sistema de irrigação automática
 * - Alarme e sensores de presença
 * - Cenas predefinidas (Sair, Chegar, Dormir, Acordar)
 *
 * Conexões:
 * Iluminação:
 * - Sala: GPIO 2
 * - Quarto: GPIO 4
 * - Cozinha: GPIO 5
 * - Banheiro: GPIO 18
 * - Garagem: GPIO 19
 *
 * Climatização:
 * - Ventilador: GPIO 21
 * - Ar Condicionado: GPIO 22
 *
 * Irrigação:
 * - Bomba: GPIO 23
 *
 * Sensores:
 * - Porta Principal: GPIO 15 (Reed Switch)
 * - Janela Sala: GPIO 16 (Reed Switch)
 * - PIR Sala: GPIO 17
 * - DHT22: GPIO 25
 * - Umidade Solo: GPIO 34 (Analógico)
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"
#include "DHT.h"

// ========== CONFIGURAÇÕES ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://app.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== PINOS - ILUMINAÇÃO ==========
#define LUZ_SALA 2
#define LUZ_QUARTO 4
#define LUZ_COZINHA 5
#define LUZ_BANHEIRO 18
#define LUZ_GARAGEM 19

// ========== PINOS - CLIMATIZAÇÃO ==========
#define VENTILADOR 21
#define AR_CONDICIONADO 22

// ========== PINOS - IRRIGAÇÃO ==========
#define BOMBA_AGUA 23

// ========== PINOS - SENSORES ==========
#define DHT_PIN 25
#define DHT_TYPE DHT22
#define PORTA_PIN 15
#define JANELA_PIN 16
#define PIR_PIN 17
#define SOLO_PIN 34

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);
DHT dht(DHT_PIN, DHT_TYPE);

// ========== ESTADO DOS DISPOSITIVOS ==========
struct {
  bool sala, quarto, cozinha, banheiro, garagem;
} luzes = {false, false, false, false, false};

struct {
  bool ventilador, arCondicionado;
} clima = {false, false};

bool bombaAgua = false;
bool alarmeAtivo = false;

// ========== VARIÁVEIS DE AUTOMAÇÃO ==========
unsigned long ultimaIrrigacao = 0;
const unsigned long INTERVALO_IRRIGACAO = 3600000; // 1 hora
const unsigned long TEMPO_IRRIGACAO = 30000;       // 30 segundos

float temperatura = 0;
float umidade = 0;
int umidadeSolo = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  WhatsApp ESP32 - Smart Home      ║");
  Serial.println("║  Sistema de Automação Residencial ║");
  Serial.println("╚════════════════════════════════════╝\n");

  // Configurar saídas
  pinMode(LUZ_SALA, OUTPUT);
  pinMode(LUZ_QUARTO, OUTPUT);
  pinMode(LUZ_COZINHA, OUTPUT);
  pinMode(LUZ_BANHEIRO, OUTPUT);
  pinMode(LUZ_GARAGEM, OUTPUT);
  pinMode(VENTILADOR, OUTPUT);
  pinMode(AR_CONDICIONADO, OUTPUT);
  pinMode(BOMBA_AGUA, OUTPUT);

  // Configurar entradas
  pinMode(PORTA_PIN, INPUT_PULLUP);
  pinMode(JANELA_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  // Inicializar todas as saídas desligadas
  digitalWrite(LUZ_SALA, LOW);
  digitalWrite(LUZ_QUARTO, LOW);
  digitalWrite(LUZ_COZINHA, LOW);
  digitalWrite(LUZ_BANHEIRO, LOW);
  digitalWrite(LUZ_GARAGEM, LOW);
  digitalWrite(VENTILADOR, LOW);
  digitalWrite(AR_CONDICIONADO, LOW);
  digitalWrite(BOMBA_AGUA, LOW);

  // Inicializar sensores
  dht.begin();

  // Conectar WiFi
  Serial.print("🌐 Conectando ao WiFi");
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

  Serial.println("\n✅ Sistema pronto para automação!");
  Serial.println("\nComandos rápidos:");
  Serial.println("  menu - Ver menu completo");
  Serial.println("  status - Status da casa");
  Serial.println("  sair - Cena 'Sair de Casa'");
  Serial.println("  chegar - Cena 'Chegar em Casa'");
  Serial.println("  dormir - Cena 'Dormir'");
  Serial.println();
}

void controlarLuz(int pin, bool &estado, bool ligar, String nome, String from) {
  digitalWrite(pin, ligar ? HIGH : LOW);
  estado = ligar;
  String msg = "💡 " + nome + " " + (ligar ? "ligada" : "apagada") + "!";
  whatsapp.sendText(from, msg);
  Serial.println(msg);
}

void cenaSairCasa(String from) {
  // Apagar todas as luzes
  digitalWrite(LUZ_SALA, LOW);
  digitalWrite(LUZ_QUARTO, LOW);
  digitalWrite(LUZ_COZINHA, LOW);
  digitalWrite(LUZ_BANHEIRO, LOW);
  luzes = {false, false, false, false, false};

  // Desligar climatização
  digitalWrite(VENTILADOR, LOW);
  digitalWrite(AR_CONDICIONADO, LOW);
  clima = {false, false};

  // Ativar alarme
  alarmeAtivo = true;

  String msg = "🚪 *Cena: Sair de Casa*\n\n";
  msg += "✅ Todas as luzes apagadas\n";
  msg += "✅ Climatização desligada\n";
  msg += "✅ Alarme ativado\n";
  msg += "🔒 Casa segura!\n";

  whatsapp.sendText(from, msg);
  Serial.println("🚪 Cena 'Sair de Casa' ativada");
}

void cenaChegar(String from) {
  // Ligar luz da sala e garagem
  digitalWrite(LUZ_SALA, HIGH);
  digitalWrite(LUZ_GARAGEM, HIGH);
  luzes.sala = true;
  luzes.garagem = true;

  // Desativar alarme
  alarmeAtivo = false;

  String msg = "🏠 *Cena: Chegar em Casa*\n\n";
  msg += "✅ Luz da sala ligada\n";
  msg += "✅ Luz da garagem ligada\n";
  msg += "✅ Alarme desativado\n";
  msg += "😊 Bem-vindo(a)!\n";

  whatsapp.sendText(from, msg);
  Serial.println("🏠 Cena 'Chegar em Casa' ativada");
}

void cenaDormir(String from) {
  // Apagar todas as luzes exceto quarto (modo noturno)
  digitalWrite(LUZ_SALA, LOW);
  digitalWrite(LUZ_COZINHA, LOW);
  digitalWrite(LUZ_BANHEIRO, LOW);
  digitalWrite(LUZ_GARAGEM, LOW);
  digitalWrite(LUZ_QUARTO, HIGH);
  luzes = {false, true, false, false, false};

  // Ligar ventilador, desligar ar
  digitalWrite(VENTILADOR, HIGH);
  digitalWrite(AR_CONDICIONADO, LOW);
  clima = {true, false};

  // Ativar alarme
  alarmeAtivo = true;

  String msg = "😴 *Cena: Dormir*\n\n";
  msg += "✅ Luzes apagadas (exceto quarto)\n";
  msg += "✅ Ventilador ligado\n";
  msg += "✅ Alarme ativado\n";
  msg += "🌙 Boa noite!\n";

  whatsapp.sendText(from, msg);
  Serial.println("😴 Cena 'Dormir' ativada");
}

void cenaAcordar(String from) {
  // Ligar luz do quarto e banheiro
  digitalWrite(LUZ_QUARTO, HIGH);
  digitalWrite(LUZ_BANHEIRO, HIGH);
  luzes.quarto = true;
  luzes.banheiro = true;

  // Desligar climatização noturna
  digitalWrite(VENTILADOR, LOW);
  clima.ventilador = false;

  // Desativar alarme
  alarmeAtivo = false;

  String msg = "☀️ *Cena: Acordar*\n\n";
  msg += "✅ Luz do quarto ligada\n";
  msg += "✅ Luz do banheiro ligada\n";
  msg += "✅ Alarme desativado\n";
  msg += "😊 Bom dia!\n";

  whatsapp.sendText(from, msg);
  Serial.println("☀️ Cena 'Acordar' ativada");
}

String getStatusCasa() {
  String msg = "🏠 *Status da Casa*\n\n";

  msg += "*💡 ILUMINAÇÃO:*\n";
  msg += "  Sala: " + String(luzes.sala ? "🟢 ON" : "🔴 OFF") + "\n";
  msg += "  Quarto: " + String(luzes.quarto ? "🟢 ON" : "🔴 OFF") + "\n";
  msg += "  Cozinha: " + String(luzes.cozinha ? "🟢 ON" : "🔴 OFF") + "\n";
  msg += "  Banheiro: " + String(luzes.banheiro ? "🟢 ON" : "🔴 OFF") + "\n";
  msg += "  Garagem: " + String(luzes.garagem ? "🟢 ON" : "🔴 OFF") + "\n\n";

  msg += "*🌡️ CLIMATIZAÇÃO:*\n";
  msg += "  Temperatura: " + String(temperatura, 1) + "°C\n";
  msg += "  Umidade: " + String(umidade, 1) + "%\n";
  msg += "  Ventilador: " + String(clima.ventilador ? "🟢 ON" : "🔴 OFF") + "\n";
  msg += "  Ar Cond.: " + String(clima.arCondicionado ? "🟢 ON" : "🔴 OFF") + "\n\n";

  msg += "*💧 IRRIGAÇÃO:*\n";
  msg += "  Umidade Solo: " + String(umidadeSolo) + "%\n";
  msg += "  Bomba: " + String(bombaAgua ? "🟢 ON" : "🔴 OFF") + "\n\n";

  msg += "*🔒 SEGURANÇA:*\n";
  bool porta = digitalRead(PORTA_PIN) == LOW;
  bool janela = digitalRead(JANELA_PIN) == LOW;
  bool movimento = digitalRead(PIR_PIN) == HIGH;
  msg += "  Porta: " + String(porta ? "🔓 Aberta" : "🔒 Fechada") + "\n";
  msg += "  Janela: " + String(janela ? "🔓 Aberta" : "🔒 Fechada") + "\n";
  msg += "  Movimento: " + String(movimento ? "🚨 Detectado" : "✅ Normal") + "\n";
  msg += "  Alarme: " + String(alarmeAtivo ? "🚨 Ativo" : "🔓 Desativado") + "\n";

  return msg;
}

String getMenuPrincipal() {
  String msg = "🎛️ *Menu Principal*\n\n";
  msg += "*🎬 CENAS:*\n";
  msg += "  🚪 sair - Sair de casa\n";
  msg += "  🏠 chegar - Chegar em casa\n";
  msg += "  😴 dormir - Ir dormir\n";
  msg += "  ☀️ acordar - Acordar\n\n";
  msg += "*💡 ILUMINAÇÃO:*\n";
  msg += "  sala on/off\n";
  msg += "  quarto on/off\n";
  msg += "  cozinha on/off\n";
  msg += "  banheiro on/off\n";
  msg += "  garagem on/off\n";
  msg += "  todas on/off\n\n";
  msg += "*🌡️ CLIMA:*\n";
  msg += "  ventilador on/off\n";
  msg += "  ar on/off\n\n";
  msg += "*💧 IRRIGAÇÃO:*\n";
  msg += "  irrigar - Irrigar agora\n\n";
  msg += "*📊 INFO:*\n";
  msg += "  status - Status da casa\n";
  return msg;
}

void processCommand(String from, String cmd) {
  cmd.toLowerCase();
  cmd.trim();

  Serial.printf("📱 Comando: '%s'\n", cmd.c_str());

  // Menu e Status
  if (cmd == "menu" || cmd == "ajuda") {
    whatsapp.sendText(from, getMenuPrincipal());
  }
  else if (cmd == "status") {
    whatsapp.sendText(from, getStatusCasa());
  }
  // Cenas
  else if (cmd == "sair") cenaSairCasa(from);
  else if (cmd == "chegar") cenaChegar(from);
  else if (cmd == "dormir") cenaDormir(from);
  else if (cmd == "acordar") cenaAcordar(from);
  // Iluminação
  else if (cmd == "sala on") controlarLuz(LUZ_SALA, luzes.sala, true, "Luz da sala", from);
  else if (cmd == "sala off") controlarLuz(LUZ_SALA, luzes.sala, false, "Luz da sala", from);
  else if (cmd == "quarto on") controlarLuz(LUZ_QUARTO, luzes.quarto, true, "Luz do quarto", from);
  else if (cmd == "quarto off") controlarLuz(LUZ_QUARTO, luzes.quarto, false, "Luz do quarto", from);
  else if (cmd == "cozinha on") controlarLuz(LUZ_COZINHA, luzes.cozinha, true, "Luz da cozinha", from);
  else if (cmd == "cozinha off") controlarLuz(LUZ_COZINHA, luzes.cozinha, false, "Luz da cozinha", from);
  else if (cmd == "banheiro on") controlarLuz(LUZ_BANHEIRO, luzes.banheiro, true, "Luz do banheiro", from);
  else if (cmd == "banheiro off") controlarLuz(LUZ_BANHEIRO, luzes.banheiro, false, "Luz do banheiro", from);
  else if (cmd == "garagem on") controlarLuz(LUZ_GARAGEM, luzes.garagem, true, "Luz da garagem", from);
  else if (cmd == "garagem off") controlarLuz(LUZ_GARAGEM, luzes.garagem, false, "Luz da garagem", from);
  else if (cmd == "todas on") {
    digitalWrite(LUZ_SALA, HIGH);
    digitalWrite(LUZ_QUARTO, HIGH);
    digitalWrite(LUZ_COZINHA, HIGH);
    digitalWrite(LUZ_BANHEIRO, HIGH);
    digitalWrite(LUZ_GARAGEM, HIGH);
    luzes = {true, true, true, true, true};
    whatsapp.sendText(from, "💡 Todas as luzes ligadas!");
  }
  else if (cmd == "todas off") {
    digitalWrite(LUZ_SALA, LOW);
    digitalWrite(LUZ_QUARTO, LOW);
    digitalWrite(LUZ_COZINHA, LOW);
    digitalWrite(LUZ_BANHEIRO, LOW);
    digitalWrite(LUZ_GARAGEM, LOW);
    luzes = {false, false, false, false, false};
    whatsapp.sendText(from, "💡 Todas as luzes apagadas!");
  }
  // Climatização
  else if (cmd == "ventilador on" || cmd == "vent on") {
    digitalWrite(VENTILADOR, HIGH);
    clima.ventilador = true;
    whatsapp.sendText(from, "🌀 Ventilador ligado!");
  }
  else if (cmd == "ventilador off" || cmd == "vent off") {
    digitalWrite(VENTILADOR, LOW);
    clima.ventilador = false;
    whatsapp.sendText(from, "🌀 Ventilador desligado!");
  }
  else if (cmd == "ar on") {
    digitalWrite(AR_CONDICIONADO, HIGH);
    clima.arCondicionado = true;
    whatsapp.sendText(from, "❄️ Ar condicionado ligado!");
  }
  else if (cmd == "ar off") {
    digitalWrite(AR_CONDICIONADO, LOW);
    clima.arCondicionado = false;
    whatsapp.sendText(from, "❄️ Ar condicionado desligado!");
  }
  // Irrigação
  else if (cmd == "irrigar") {
    digitalWrite(BOMBA_AGUA, HIGH);
    bombaAgua = true;
    whatsapp.sendText(from, "💧 Iniciando irrigação por 30 segundos...");
    delay(30000);
    digitalWrite(BOMBA_AGUA, LOW);
    bombaAgua = false;
    whatsapp.sendText(from, "✅ Irrigação concluída!");
    ultimaIrrigacao = millis();
  }
  else {
    whatsapp.sendText(from, "❌ Comando não reconhecido!\n\nDigite *menu* para ver os comandos.");
  }
}

void loop() {
  // Ler sensores
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  int soloValue = analogRead(SOLO_PIN);
  umidadeSolo = map(soloValue, 0, 4095, 100, 0);

  // Irrigação automática
  if (umidadeSolo < 30 && !bombaAgua && (millis() - ultimaIrrigacao > INTERVALO_IRRIGACAO)) {
    Serial.println("💧 Irrigação automática iniciada (solo seco)");
    digitalWrite(BOMBA_AGUA, HIGH);
    bombaAgua = true;
    delay(TEMPO_IRRIGACAO);
    digitalWrite(BOMBA_AGUA, LOW);
    bombaAgua = false;
    ultimaIrrigacao = millis();
  }

  // Receber mensagens
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns)\n", count);
    for (int i = 0; i < count; i++) {
      processCommand(messages[i].from, messages[i].text);
    }
  }

  delay(3000);
}
