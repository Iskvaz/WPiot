/*
 * Exemplo Sistema de Alarme - WhatsApp ESP32
 *
 * Sistema completo de alarme residencial com notificações WhatsApp:
 * - Múltiplas zonas de detecção
 * - Sirene e LED de alerta
 * - Armar/desarmar via WhatsApp
 * - Notificações automáticas de invasão
 * - Histórico de eventos
 * - Modo teste para verificação dos sensores
 *
 * Zonas de Segurança:
 * - Zona 1: Porta Principal (Reed Switch) - GPIO 15
 * - Zona 2: Janela Sala (Reed Switch) - GPIO 16
 * - Zona 3: Janela Quarto (Reed Switch) - GPIO 17
 * - Zona 4: Sensor Movimento Sala (PIR) - GPIO 18
 * - Zona 5: Sensor Movimento Garagem (PIR) - GPIO 19
 *
 * Atuadores:
 * - Sirene: GPIO 2
 * - LED Status: GPIO 4 (Verde=Desarmado, Vermelho=Armado, Piscando=Alerta)
 * - Buzzer Feedback: GPIO 5
 */

#include <WiFi.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== PINOS - SENSORES (ZONAS) ==========
#define ZONA1_PIN 15  // Porta Principal
#define ZONA2_PIN 16  // Janela Sala
#define ZONA3_PIN 17  // Janela Quarto
#define ZONA4_PIN 18  // PIR Sala
#define ZONA5_PIN 19  // PIR Garagem

// ========== PINOS - ATUADORES ==========
#define SIRENE_PIN 2
#define LED_STATUS_PIN 4
#define BUZZER_PIN 5

// ========== CONFIGURAÇÕES DO ALARME ==========
#define TEMPO_ATIVAR 30000      // 30s para sair de casa
#define TEMPO_DISPARAR 15000    // 15s para desativar ao entrar
#define TEMPO_SIRENE 120000     // 2 minutos de sirene

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// ========== VARIÁVEIS DO SISTEMA ==========
enum EstadoAlarme {
  DESARMADO,
  ATIVANDO,
  ARMADO,
  ALERTA,
  DISPARADO
};

EstadoAlarme estadoAtual = DESARMADO;
String nomesZonas[] = {"Porta Principal", "Janela Sala", "Janela Quarto", "PIR Sala", "PIR Garagem"};
int pinsZonas[] = {ZONA1_PIN, ZONA2_PIN, ZONA3_PIN, ZONA4_PIN, ZONA5_PIN};
bool zonasAtivas[] = {true, true, true, true, true};

unsigned long tempoMudancaEstado = 0;
unsigned long tempoUltimoAlerta = 0;
String numeroAutorizado = "";
int zonaDisparada = -1;

struct Evento {
  String timestamp;
  String descricao;
};

Evento historico[10];
int historicoIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║    🚨 SISTEMA DE ALARME ESP32      ║");
  Serial.println("║  Monitoramento via WhatsApp        ║");
  Serial.println("╚════════════════════════════════════╝\n");

  // Configurar sensores
  for (int i = 0; i < 5; i++) {
    pinMode(pinsZonas[i], INPUT_PULLUP);
  }

  // Configurar atuadores
  pinMode(SIRENE_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Inicializar desligados
  digitalWrite(SIRENE_PIN, LOW);
  digitalWrite(LED_STATUS_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Conectar WiFi
  Serial.print("🌐 Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✓ WiFi conectado!");

  // Inicializar WhatsApp
  whatsapp.begin();
  whatsapp.setPollInterval(2000);

  // Beep de inicialização
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  Serial.println("\n✅ Sistema de alarme pronto!");
  Serial.println("Digite 'menu' no WhatsApp para ver os comandos\n");
}

void adicionarEvento(String descricao) {
  historico[historicoIndex].timestamp = String(millis() / 1000) + "s";
  historico[historicoIndex].descricao = descricao;
  historicoIndex = (historicoIndex + 1) % 10;
  Serial.println("📝 Evento: " + descricao);
}

void beep(int vezes) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < vezes - 1) delay(100);
  }
}

void armarAlarme(String from) {
  if (estadoAtual != DESARMADO) {
    whatsapp.sendText(from, "⚠️ Alarme já está armado ou em processo de ativação!");
    return;
  }

  estadoAtual = ATIVANDO;
  tempoMudancaEstado = millis();
  numeroAutorizado = from;

  String msg = "⏱️ *Ativando Alarme*\n\n";
  msg += "Você tem 30 segundos para sair.\n\n";
  msg += "*Zonas Ativas:*\n";
  for (int i = 0; i < 5; i++) {
    if (zonasAtivas[i]) {
      msg += "✅ " + nomesZonas[i] + "\n";
    }
  }

  whatsapp.sendText(from, msg);
  adicionarEvento("Alarme ativando - 30s");

  // Beeps de aviso
  beep(3);
}

void desarmarAlarme(String from) {
  if (estadoAtual == DESARMADO) {
    whatsapp.sendText(from, "ℹ️ Alarme já está desarmado!");
    return;
  }

  EstadoAlarme estadoAnterior = estadoAtual;
  estadoAtual = DESARMADO;
  digitalWrite(SIRENE_PIN, LOW);
  digitalWrite(LED_STATUS_PIN, LOW);

  String msg = "✅ *Alarme Desarmado*\n\n";
  if (estadoAnterior == DISPARADO) {
    msg += "Sirene desativada.\n";
  }
  msg += "Sistema em modo seguro.";

  whatsapp.sendText(from, msg);
  adicionarEvento("Alarme desarmado por " + from);
  beep(2);
}

String getStatus() {
  String msg = "🚨 *Status do Sistema*\n\n";

  msg += "*ESTADO:* ";
  switch (estadoAtual) {
    case DESARMADO:
      msg += "🟢 Desarmado\n";
      break;
    case ATIVANDO:
      msg += "🟡 Ativando (" + String((TEMPO_ATIVAR - (millis() - tempoMudancaEstado)) / 1000) + "s)\n";
      break;
    case ARMADO:
      msg += "🔴 Armado\n";
      break;
    case ALERTA:
      msg += "🟠 Alerta - Zona violada\n";
      break;
    case DISPARADO:
      msg += "🚨 DISPARADO!\n";
      break;
  }

  msg += "\n*ZONAS DE SEGURANÇA:*\n";
  for (int i = 0; i < 5; i++) {
    bool status = digitalRead(pinsZonas[i]) == LOW;
    String icone = zonasAtivas[i] ? (status ? "🔴" : "🟢") : "⚪";
    msg += icone + " " + nomesZonas[i];
    if (!zonasAtivas[i]) msg += " (Desativada)";
    msg += "\n";
  }

  msg += "\n*SISTEMA:*\n";
  msg += "📱 WhatsApp: Conectado\n";
  msg += "🔋 Uptime: " + String(millis() / 60000) + " min\n";

  return msg;
}

String getHistorico() {
  String msg = "📋 *Histórico de Eventos*\n\n";

  for (int i = 0; i < 10; i++) {
    int idx = (historicoIndex + i) % 10;
    if (historico[idx].descricao != "") {
      msg += historico[idx].timestamp + " - " + historico[idx].descricao + "\n";
    }
  }

  if (msg == "📋 *Histórico de Eventos*\n\n") {
    msg += "Nenhum evento registrado.";
  }

  return msg;
}

String getMenu() {
  String msg = "🎛️ *Menu do Sistema de Alarme*\n\n";
  msg += "*CONTROLE:*\n";
  msg += "🔴 *armar* - Armar alarme\n";
  msg += "🟢 *desarmar* - Desarmar alarme\n\n";
  msg += "*CONFIGURAÇÃO:*\n";
  msg += "⚙️ *zona1-5 on/off* - Ativar/desativar zona\n";
  msg += "🧪 *teste* - Testar sensores\n\n";
  msg += "*INFORMAÇÕES:*\n";
  msg += "📊 *status* - Ver status atual\n";
  msg += "📋 *historico* - Ver histórico\n";
  return msg;
}

void testarSensores(String from) {
  String msg = "🧪 *Teste de Sensores*\n\n";

  for (int i = 0; i < 5; i++) {
    bool status = digitalRead(pinsZonas[i]) == LOW;
    msg += nomesZonas[i] + ": ";
    msg += status ? "🔴 ATIVADO\n" : "🟢 Normal\n";
  }

  msg += "\n✅ Teste concluído!";
  whatsapp.sendText(from, msg);
}

void verificarZonas() {
  if (estadoAtual != ARMADO) return;

  for (int i = 0; i < 5; i++) {
    if (!zonasAtivas[i]) continue;

    bool violacao = digitalRead(pinsZonas[i]) == LOW;

    if (violacao) {
      Serial.println("🚨 ZONA VIOLADA: " + nomesZonas[i]);

      estadoAtual = ALERTA;
      zonaDisparada = i;
      tempoMudancaEstado = millis();

      String msg = "⚠️ *ALERTA!*\n\n";
      msg += "Zona violada: *" + nomesZonas[i] + "*\n\n";
      msg += "Você tem 15 segundos para desarmar!\n";
      msg += "Digite: *desarmar*";

      whatsapp.sendText(numeroAutorizado, msg);
      adicionarEvento("ALERTA - " + nomesZonas[i]);

      // LED piscando
      for (int j = 0; j < 5; j++) {
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(200);
        digitalWrite(LED_STATUS_PIN, LOW);
        delay(200);
      }

      break;
    }
  }
}

void processCommand(String from, String cmd) {
  cmd.toLowerCase();
  cmd.trim();

  Serial.printf("📱 Comando: '%s' de %s\n", cmd.c_str(), from.c_str());

  if (cmd == "menu" || cmd == "ajuda") {
    whatsapp.sendText(from, getMenu());
  }
  else if (cmd == "status") {
    whatsapp.sendText(from, getStatus());
  }
  else if (cmd == "historico" || cmd == "histórico") {
    whatsapp.sendText(from, getHistorico());
  }
  else if (cmd == "armar") {
    armarAlarme(from);
  }
  else if (cmd == "desarmar") {
    desarmarAlarme(from);
  }
  else if (cmd == "teste") {
    testarSensores(from);
  }
  else if (cmd.startsWith("zona")) {
    int zona = cmd.charAt(4) - '1';
    bool ativar = cmd.indexOf("on") != -1;

    if (zona >= 0 && zona < 5) {
      zonasAtivas[zona] = ativar;
      String msg = nomesZonas[zona] + " " + (ativar ? "ativada" : "desativada") + "!";
      whatsapp.sendText(from, msg);
      adicionarEvento(msg);
    }
  }
  else {
    whatsapp.sendText(from, "❌ Comando não reconhecido!\n\nDigite *menu*");
  }
}

void loop() {
  // Máquina de estados
  switch (estadoAtual) {
    case ATIVANDO:
      if (millis() - tempoMudancaEstado >= TEMPO_ATIVAR) {
        estadoAtual = ARMADO;
        digitalWrite(LED_STATUS_PIN, HIGH);
        whatsapp.sendText(numeroAutorizado, "🔴 *Alarme Armado!*\n\nTodas as zonas ativas estão sendo monitoradas.");
        adicionarEvento("Alarme ARMADO");
        beep(1);
      }
      break;

    case ARMADO:
      verificarZonas();
      break;

    case ALERTA:
      if (millis() - tempoMudancaEstado >= TEMPO_DISPARAR) {
        estadoAtual = DISPARADO;
        digitalWrite(SIRENE_PIN, HIGH);
        tempoMudancaEstado = millis();

        String msg = "🚨 *ALARME DISPARADO!*\n\n";
        msg += "Zona: *" + nomesZonas[zonaDisparada] + "*\n\n";
        msg += "Sirene ativada!\n";
        msg += "Digite *desarmar* para desativar.";

        whatsapp.sendText(numeroAutorizado, msg);
        adicionarEvento("ALARME DISPARADO - " + nomesZonas[zonaDisparada]);
      }
      break;

    case DISPARADO:
      // Piscar LED
      digitalWrite(LED_STATUS_PIN, (millis() / 500) % 2);

      // Desligar sirene após tempo limite
      if (millis() - tempoMudancaEstado >= TEMPO_SIRENE) {
        digitalWrite(SIRENE_PIN, LOW);
        adicionarEvento("Sirene desligada (timeout)");
      }
      break;
  }

  // Receber mensagens
  WhatsAppMessage messages[5];
  int count = whatsapp.getMessages(messages, 5);

  if (count > 0) {
    for (int i = 0; i < count; i++) {
      processCommand(messages[i].from, messages[i].text);
    }
  }

  delay(2000);
}
