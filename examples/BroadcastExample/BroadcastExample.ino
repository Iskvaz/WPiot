/*
 * Exemplo BroadcastExample - WhatsApp ESP32
 *
 * Este exemplo demonstra como:
 * - Enviar mensagens para múltiplos contatos simultaneamente (broadcast)
 * - Criar listas de distribuição personalizadas
 * - Enviar notificações e alertas em massa
 * - Gerenciar diferentes grupos de contatos
 * - Personalizar mensagens por contato
 *
 * Casos de uso:
 * - Alertas de segurança para família
 * - Notificações de sistema para administradores
 * - Relatórios para equipe técnica
 * - Avisos automáticos de eventos
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

// ========== LISTAS DE CONTATOS ==========

// Lista 1: Família (números com DDD do Brasil)
// Formato: 5511999999999 (código país + DDD + número)
const char* listaFamilia[] = {
  "5511999999999",  // Pessoa 1
  "5511888888888",  // Pessoa 2
  "5511777777777"   // Pessoa 3
};
const int tamanhoListaFamilia = 3;

// Lista 2: Administradores do Sistema
const char* listaAdmins[] = {
  "5511999999999",  // Admin 1
  "5511888888888"   // Admin 2
};
const int tamanhoListaAdmins = 2;

// Lista 3: Equipe Técnica
const char* listaTecnica[] = {
  "5511999999999",  // Técnico 1
  "5511888888888",  // Técnico 2
  "5511777777777",  // Técnico 3
  "5511666666666"   // Técnico 4
};
const int tamanhoListaTecnica = 4;

// Lista 4: Contatos VIP (notificações prioritárias)
const char* listaVIP[] = {
  "5511999999999"   // VIP 1
};
const int tamanhoListaVIP = 1;

// ========== CONFIGURAÇÕES DE HARDWARE ==========
#define LED_ALERTA 2       // LED para alertas
#define SENSOR_TEMP 34     // Sensor de temperatura (analógico)
#define SENSOR_MOVIMENTO 15 // Sensor PIR

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// ========== VARIÁVEIS GLOBAIS ==========
unsigned long ultimoBroadcast = 0;
const unsigned long INTERVALO_BROADCAST = 3600000;  // 1 hora

float temperatura = 0;
bool movimentoDetectado = false;
int contadorAlertas = 0;

// ========== FUNÇÕES DE BROADCAST ==========

// Envia mensagem para uma lista específica de contatos
void enviarBroadcast(const char* lista[], int tamanho, String mensagem) {
  Serial.println("\n📢 Iniciando broadcast...");
  Serial.println("Destinatários: " + String(tamanho));

  int enviados = 0;
  int falhas = 0;

  for (int i = 0; i < tamanho; i++) {
    Serial.print("Enviando para " + String(lista[i]) + "... ");

    if (whatsapp.sendText(lista[i], mensagem)) {
      Serial.println("✓ Enviado");
      enviados++;
      digitalWrite(LED_ALERTA, HIGH);
      delay(100);
      digitalWrite(LED_ALERTA, LOW);
    } else {
      Serial.println("✗ Falhou");
      falhas++;
    }

    // Pequeno delay entre envios para não sobrecarregar
    delay(1000);
  }

  Serial.println("\n📊 Resultado do broadcast:");
  Serial.println("✓ Enviados: " + String(enviados));
  Serial.println("✗ Falhas: " + String(falhas));
}

// Envia mensagem personalizada para cada contato
void enviarBroadcastPersonalizado(const char* lista[], int tamanho, String mensagemBase) {
  Serial.println("\n📢 Iniciando broadcast personalizado...");

  for (int i = 0; i < tamanho; i++) {
    // Personaliza a mensagem com o número do contato
    String mensagem = mensagemBase + "\n\n_Enviado para: " + String(lista[i]) + "_";

    Serial.print("Enviando para " + String(lista[i]) + "... ");

    if (whatsapp.sendText(lista[i], mensagem)) {
      Serial.println("✓");
    } else {
      Serial.println("✗");
    }

    delay(1000);
  }
}

// Envia alerta crítico para todas as listas
void enviarAlertaGeral(String titulo, String mensagem) {
  String alertaCompleto = "🚨 *ALERTA: " + titulo + "*\n\n";
  alertaCompleto += mensagem;
  alertaCompleto += "\n\n⏰ " + String(millis() / 1000) + "s";

  Serial.println("\n🚨 ENVIANDO ALERTA GERAL!");

  // Enviar para todas as listas
  Serial.println("\n→ Enviando para VIPs...");
  enviarBroadcast(listaVIP, tamanhoListaVIP, alertaCompleto);

  Serial.println("\n→ Enviando para Admins...");
  enviarBroadcast(listaAdmins, tamanhoListaAdmins, alertaCompleto);

  Serial.println("\n→ Enviando para Técnicos...");
  enviarBroadcast(listaTecnica, tamanhoListaTecnica, alertaCompleto);

  Serial.println("\n✅ Alerta geral enviado para todos!");

  // Piscar LED
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_ALERTA, HIGH);
    delay(100);
    digitalWrite(LED_ALERTA, LOW);
    delay(100);
  }
}

// Envia relatório diário
void enviarRelatorioDiario() {
  String relatorio = "📊 *Relatório Diário - ESP32*\n\n";
  relatorio += "📅 Data: " + String(millis() / 86400000) + " dias\n";
  relatorio += "⏱️ Uptime: " + String(millis() / 1000) + "s\n";
  relatorio += "🚨 Alertas gerados: " + String(contadorAlertas) + "\n";
  relatorio += "🌡️ Temperatura atual: " + String(temperatura, 1) + "°C\n";
  relatorio += "💾 Memória livre: " + String(ESP.getFreeHeap()) + " bytes\n";
  relatorio += "📶 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  relatorio += "\n✅ Sistema operando normalmente";

  // Enviar apenas para admins
  enviarBroadcast(listaAdmins, tamanhoListaAdmins, relatorio);
}

// ========== FUNÇÕES DE MONITORAMENTO ==========

void verificarTemperatura() {
  // Simula leitura de temperatura (substitua por sensor real)
  int leitura = analogRead(SENSOR_TEMP);
  temperatura = (leitura / 4095.0) * 100.0;  // Converte para 0-100°C

  // Alerta de temperatura alta
  if (temperatura > 40.0) {
    String alerta = "Temperatura crítica detectada!\n\n";
    alerta += "🌡️ Temperatura: " + String(temperatura, 1) + "°C\n";
    alerta += "⚠️ Limite: 40.0°C\n\n";
    alerta += "Ação recomendada: Verificar sistema de resfriamento";

    enviarAlertaGeral("Temperatura Alta", alerta);
    contadorAlertas++;
  }
}

void verificarMovimento() {
  movimentoDetectado = digitalRead(SENSOR_MOVIMENTO);

  if (movimentoDetectado) {
    String alerta = "Movimento detectado no ambiente monitorado!\n\n";
    alerta += "📍 Localização: Área de Segurança\n";
    alerta += "⏰ Horário: " + String(millis() / 1000) + "s\n\n";
    alerta += "Verifique as câmeras de segurança.";

    // Enviar apenas para família e admins
    Serial.println("\n🚨 Movimento detectado!");
    enviarBroadcast(listaFamilia, tamanhoListaFamilia, "🚨 " + alerta);
    enviarBroadcast(listaAdmins, tamanhoListaAdmins, "🚨 " + alerta);

    contadorAlertas++;

    // Aguardar 1 minuto antes de enviar novo alerta de movimento
    delay(60000);
  }
}

// ========== PROCESSAMENTO DE COMANDOS ==========

void processarComando(String from, String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  Serial.println("\n📥 Comando recebido: " + cmd);

  if (cmd == "menu" || cmd == "/menu") {
    String menu = "📋 *Menu - Broadcast System*\n\n";
    menu += "*Testes:*\n";
    menu += "• /teste familia - Testa lista família\n";
    menu += "• /teste admins - Testa lista admins\n";
    menu += "• /teste tecnica - Testa lista técnica\n";
    menu += "• /teste geral - Testa todas as listas\n\n";
    menu += "*Relatórios:*\n";
    menu += "• /relatorio - Relatório atual\n";
    menu += "• /status - Status do sistema\n\n";
    menu += "*Alertas:*\n";
    menu += "• /alerta teste - Simula alerta\n";
    menu += "• /temp - Leitura temperatura\n";
    menu += "• /movimento - Status movimento";

    whatsapp.sendText(from, menu);
  }

  else if (cmd == "teste familia" || cmd == "/teste familia") {
    enviarBroadcast(listaFamilia, tamanhoListaFamilia,
                   "✅ Teste de broadcast para família!\nSistema funcionando normalmente.");
    whatsapp.sendText(from, "✓ Broadcast enviado para lista FAMÍLIA");
  }

  else if (cmd == "teste admins" || cmd == "/teste admins") {
    enviarBroadcast(listaAdmins, tamanhoListaAdmins,
                   "✅ Teste de broadcast para admins!\nSistema funcionando normalmente.");
    whatsapp.sendText(from, "✓ Broadcast enviado para lista ADMINS");
  }

  else if (cmd == "teste tecnica" || cmd == "/teste tecnica") {
    enviarBroadcast(listaTecnica, tamanhoListaTecnica,
                   "✅ Teste de broadcast para equipe técnica!\nSistema funcionando normalmente.");
    whatsapp.sendText(from, "✓ Broadcast enviado para lista TÉCNICA");
  }

  else if (cmd == "teste geral" || cmd == "/teste geral") {
    enviarAlertaGeral("Teste", "Este é um teste do sistema de broadcast.\nTodas as listas estão sendo notificadas.");
    whatsapp.sendText(from, "✓ Alerta geral enviado para TODAS as listas");
  }

  else if (cmd == "relatorio" || cmd == "/relatorio") {
    enviarRelatorioDiario();
    whatsapp.sendText(from, "✓ Relatório enviado para admins");
  }

  else if (cmd == "status" || cmd == "/status") {
    String status = "📊 *Status do Sistema*\n\n";
    status += "✅ Online\n";
    status += "🌡️ Temp: " + String(temperatura, 1) + "°C\n";
    status += "🚨 Alertas: " + String(contadorAlertas) + "\n";
    status += "⏱️ Uptime: " + String(millis() / 1000) + "s";

    whatsapp.sendText(from, status);
  }

  else if (cmd == "alerta teste" || cmd == "/alerta teste") {
    enviarAlertaGeral("Teste", "Este é um alerta de teste do sistema.");
    whatsapp.sendText(from, "✓ Alerta de teste enviado");
  }

  else if (cmd == "temp" || cmd == "/temp") {
    verificarTemperatura();
    String msg = "🌡️ Temperatura: " + String(temperatura, 1) + "°C";
    whatsapp.sendText(from, msg);
  }

  else if (cmd == "movimento" || cmd == "/movimento") {
    String msg = "🚶 Movimento: " + String(movimentoDetectado ? "Detectado" : "Nenhum");
    whatsapp.sendText(from, msg);
  }

  else {
    whatsapp.sendText(from, "❓ Comando não reconhecido.\nEnvie *menu* para ver comandos.");
  }
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("  WhatsApp ESP32 - Broadcast System");
  Serial.println("========================================\n");

  // Configurar pinos
  pinMode(LED_ALERTA, OUTPUT);
  pinMode(SENSOR_MOVIMENTO, INPUT);
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

  // Verificar conexão
  if (whatsapp.isConnected()) {
    Serial.println("✓ WhatsApp conectado!");
  } else {
    Serial.println("✗ WhatsApp não conectado!");
  }

  Serial.println("\n========================================");
  Serial.println("LISTAS DE BROADCAST CONFIGURADAS");
  Serial.println("========================================");
  Serial.println("📋 Família: " + String(tamanhoListaFamilia) + " contatos");
  Serial.println("👨‍💼 Admins: " + String(tamanhoListaAdmins) + " contatos");
  Serial.println("🔧 Técnica: " + String(tamanhoListaTecnica) + " contatos");
  Serial.println("⭐ VIP: " + String(tamanhoListaVIP) + " contatos");
  Serial.println("\n⚠️ IMPORTANTE: Atualize os números nas listas!");
  Serial.println("   Formato: 5511999999999 (país + DDD + número)");

  Serial.println("\n========================================");
  Serial.println("Sistema pronto! Monitorando...");
  Serial.println("========================================\n");

  // Enviar mensagem de inicialização
  String msgInit = "🚀 *Sistema de Broadcast Iniciado*\n\n";
  msgInit += "O sistema ESP32 está online e monitorando.\n";
  msgInit += "Envie */menu* para ver comandos disponíveis.";

  // Descomentar após configurar os números
  // enviarBroadcast(listaAdmins, tamanhoListaAdmins, msgInit);

  Serial.println("💡 Dica: Configure os números nas listas e descomente");
  Serial.println("    a linha de broadcast no setup().\n");
}

// ========== LOOP ==========

void loop() {
  // Obter e processar mensagens
  WhatsAppMessage messages[10];
  int count = whatsapp.getMessages(messages, 10);

  if (count > 0) {
    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      processarComando(messages[i].from, messages[i].text);
    }
  }

  // Monitoramento contínuo
  verificarTemperatura();
  verificarMovimento();

  // Relatório periódico
  if (millis() - ultimoBroadcast > INTERVALO_BROADCAST) {
    enviarRelatorioDiario();
    ultimoBroadcast = millis();
  }

  delay(5000);
}
