/*
 * Exemplo AccessControl - WhatsApp ESP32
 *
 * Sistema de controle de acesso com lista de números permitidos
 *
 * Este exemplo demonstra como:
 * - Definir um administrador fixo no código
 * - Armazenar lista de números permitidos na Flash (NVS)
 * - Validar TODAS as mensagens antes de processar
 * - Gerenciar lista de permitidos via comandos WhatsApp
 * - Persistir dados mesmo após restart
 * - Negar acesso automaticamente para não autorizados
 *
 * FUNCIONALIDADES:
 *
 * 1. ADMIN (hardcoded):
 *    - Número fixo no código
 *    - Acesso total ao sistema
 *    - Pode gerenciar lista de permitidos
 *
 * 2. LISTA DE PERMITIDOS:
 *    - Armazenada na Flash (NVS/Preferences)
 *    - Persiste após restart
 *    - Admin gerencia via WhatsApp
 *
 * 3. VALIDAÇÃO:
 *    - Toda mensagem é verificada
 *    - Apenas admin e permitidos podem interagir
 *    - Outros recebem "acesso negado"
 *
 * COMANDOS ADMIN:
 * - /adicionar [numero] - Adiciona número à lista
 * - /remover [numero] - Remove número da lista
 * - /listar - Lista todos os permitidos
 * - /limpar - Limpa toda a lista
 * - /total - Total de números permitidos
 *
 * COMANDOS USUÁRIOS PERMITIDOS:
 * - /menu - Menu de comandos
 * - /status - Status do sistema
 * - /info - Informações do bot
 * - /ping - Teste de conexão
 * - /help - Ajuda
 */

#include <WiFi.h>
#include <Preferences.h>
#include "WhatsAppESP32.h"

// ========== CONFIGURAÇÕES ==========

// WiFi
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// API WhatsApp
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// ========== ADMINISTRADOR (FIXO NO CÓDIGO) ==========
const char* ADMIN_NUMBER = "5511999999999";  // ← CONFIGURE SEU NÚMERO AQUI

// ========== CONFIGURAÇÕES DE ARMAZENAMENTO ==========
#define NAMESPACE "accessctrl"      // Namespace para Preferences
#define MAX_ALLOWED 50              // Máximo de números permitidos

// ========== HARDWARE ==========
#define LED_STATUS 2                // LED de status
#define LED_ACCESS_GRANTED 4        // LED verde - acesso permitido
#define LED_ACCESS_DENIED 5         // LED vermelho - acesso negado

// ========== OBJETOS ==========
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);
Preferences preferences;

// ========== VARIÁVEIS GLOBAIS ==========
int totalPermitidos = 0;
int tentativasNegadas = 0;
int comandosExecutados = 0;

// ========== FUNÇÕES DE GERENCIAMENTO DE ACESSO ==========

// Normaliza número (remove caracteres especiais e espaços)
String normalizeNumber(String numero) {
  numero.trim();
  numero.replace(" ", "");
  numero.replace("-", "");
  numero.replace("(", "");
  numero.replace(")", "");
  numero.replace("+", "");
  numero.replace("@c.us", "");
  numero.replace("@s.whatsapp.net", "");
  return numero;
}

// Verifica se é o administrador
bool isAdmin(String numero) {
  String normalized = normalizeNumber(numero);
  String adminNormalized = normalizeNumber(String(ADMIN_NUMBER));
  return normalized == adminNormalized;
}

// Carrega lista de permitidos da Flash
void loadAllowedList() {
  preferences.begin(NAMESPACE, false);
  totalPermitidos = preferences.getInt("total", 0);
  preferences.end();

  Serial.println("\n📋 Lista de permitidos carregada da Flash");
  Serial.println("Total de números: " + String(totalPermitidos));
}

// Salva total na Flash
void saveTotalAllowed() {
  preferences.begin(NAMESPACE, false);
  preferences.putInt("total", totalPermitidos);
  preferences.end();
}

// Verifica se número está na lista de permitidos
bool isAllowed(String numero) {
  // Admin sempre tem acesso
  if (isAdmin(numero)) {
    return true;
  }

  String normalized = normalizeNumber(numero);

  preferences.begin(NAMESPACE, true);  // Read-only

  for (int i = 0; i < totalPermitidos; i++) {
    String key = "num_" + String(i);
    String saved = preferences.getString(key.c_str(), "");

    if (normalizeNumber(saved) == normalized) {
      preferences.end();
      return true;
    }
  }

  preferences.end();
  return false;
}

// Adiciona número à lista de permitidos
bool addAllowed(String numero) {
  String normalized = normalizeNumber(numero);

  // Verifica se já existe
  if (isAllowed(normalized)) {
    return false;  // Já existe
  }

  // Verifica limite
  if (totalPermitidos >= MAX_ALLOWED) {
    return false;  // Lista cheia
  }

  // Adiciona
  preferences.begin(NAMESPACE, false);
  String key = "num_" + String(totalPermitidos);
  preferences.putString(key.c_str(), normalized);
  preferences.end();

  totalPermitidos++;
  saveTotalAllowed();

  Serial.println("✓ Número adicionado: " + normalized);
  return true;
}

// Remove número da lista
bool removeAllowed(String numero) {
  String normalized = normalizeNumber(numero);

  // Não pode remover o admin
  if (isAdmin(normalized)) {
    return false;
  }

  preferences.begin(NAMESPACE, false);

  // Encontrar e remover
  int foundIndex = -1;
  for (int i = 0; i < totalPermitidos; i++) {
    String key = "num_" + String(i);
    String saved = preferences.getString(key.c_str(), "");

    if (normalizeNumber(saved) == normalized) {
      foundIndex = i;
      break;
    }
  }

  if (foundIndex == -1) {
    preferences.end();
    return false;  // Não encontrado
  }

  // Reorganizar array (mover todos uma posição para trás)
  for (int i = foundIndex; i < totalPermitidos - 1; i++) {
    String keyFrom = "num_" + String(i + 1);
    String keyTo = "num_" + String(i);
    String value = preferences.getString(keyFrom.c_str(), "");
    preferences.putString(keyTo.c_str(), value);
  }

  // Remover último elemento
  String lastKey = "num_" + String(totalPermitidos - 1);
  preferences.remove(lastKey.c_str());

  preferences.end();

  totalPermitidos--;
  saveTotalAllowed();

  Serial.println("✓ Número removido: " + normalized);
  return true;
}

// Lista todos os números permitidos
String listAllowed() {
  String lista = "📋 *Números Permitidos*\n\n";

  if (totalPermitidos == 0) {
    lista += "_Nenhum número cadastrado_\n\n";
  } else {
    preferences.begin(NAMESPACE, true);

    for (int i = 0; i < totalPermitidos; i++) {
      String key = "num_" + String(i);
      String numero = preferences.getString(key.c_str(), "");
      lista += String(i + 1) + ". +" + numero + "\n";
    }

    preferences.end();
    lista += "\n";
  }

  lista += "👤 Admin: +" + String(ADMIN_NUMBER) + "\n";
  lista += "📊 Total: " + String(totalPermitidos) + "/" + String(MAX_ALLOWED);

  return lista;
}

// Limpa toda a lista
void clearAllowed() {
  preferences.begin(NAMESPACE, false);

  // Remove todas as chaves
  for (int i = 0; i < totalPermitidos; i++) {
    String key = "num_" + String(i);
    preferences.remove(key.c_str());
  }

  preferences.end();

  totalPermitidos = 0;
  saveTotalAllowed();

  Serial.println("✓ Lista de permitidos limpa");
}

// ========== PROCESSAMENTO DE COMANDOS ==========

void processarComando(String from, String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  bool isAdminUser = isAdmin(from);

  Serial.println("\n📥 Comando: " + cmd);
  Serial.println("De: " + from);
  Serial.println("É Admin: " + String(isAdminUser ? "Sim" : "Não"));

  comandosExecutados++;

  // ========== COMANDOS ADMIN ==========

  if (isAdminUser) {

    // ADICIONAR NÚMERO
    if (cmd.startsWith("adicionar ") || cmd.startsWith("/adicionar ")) {
      String numero = cmd;
      numero.replace("adicionar ", "");
      numero.replace("/adicionar ", "");
      numero.trim();

      if (numero.length() < 10) {
        whatsapp.sendText(from, "❌ Número inválido.\n\nFormato: /adicionar 5511999999999");
        return;
      }

      if (addAllowed(numero)) {
        digitalWrite(LED_ACCESS_GRANTED, HIGH);
        String msg = "✅ *Número Adicionado*\n\n";
        msg += "📱 +" + normalizeNumber(numero) + "\n";
        msg += "📊 Total: " + String(totalPermitidos) + "/" + String(MAX_ALLOWED);
        whatsapp.sendText(from, msg);
        delay(500);
        digitalWrite(LED_ACCESS_GRANTED, LOW);
      } else {
        whatsapp.sendText(from, "❌ Não foi possível adicionar.\n\nMotivos possíveis:\n• Número já cadastrado\n• Lista cheia (máx " + String(MAX_ALLOWED) + ")");
      }
      return;
    }

    // REMOVER NÚMERO
    else if (cmd.startsWith("remover ") || cmd.startsWith("/remover ")) {
      String numero = cmd;
      numero.replace("remover ", "");
      numero.replace("/remover ", "");
      numero.trim();

      if (removeAllowed(numero)) {
        digitalWrite(LED_ACCESS_DENIED, HIGH);
        String msg = "✅ *Número Removido*\n\n";
        msg += "📱 +" + normalizeNumber(numero) + "\n";
        msg += "📊 Total: " + String(totalPermitidos) + "/" + String(MAX_ALLOWED);
        whatsapp.sendText(from, msg);
        delay(500);
        digitalWrite(LED_ACCESS_DENIED, LOW);
      } else {
        whatsapp.sendText(from, "❌ Número não encontrado ou não pode ser removido.");
      }
      return;
    }

    // LISTAR PERMITIDOS
    else if (cmd == "listar" || cmd == "/listar") {
      whatsapp.sendText(from, listAllowed());
      return;
    }

    // LIMPAR LISTA
    else if (cmd == "limpar" || cmd == "/limpar") {
      clearAllowed();
      whatsapp.sendText(from, "✅ *Lista Limpa*\n\nTodos os números foram removidos.\nApenas o admin tem acesso agora.");
      return;
    }

    // TOTAL
    else if (cmd == "total" || cmd == "/total") {
      String msg = "📊 *Estatísticas de Acesso*\n\n";
      msg += "✅ Permitidos: " + String(totalPermitidos) + "/" + String(MAX_ALLOWED) + "\n";
      msg += "🚫 Tentativas negadas: " + String(tentativasNegadas) + "\n";
      msg += "⚡ Comandos executados: " + String(comandosExecutados);
      whatsapp.sendText(from, msg);
      return;
    }
  }

  // ========== COMANDOS GERAIS (ADMIN E PERMITIDOS) ==========

  if (cmd == "menu" || cmd == "/menu") {
    String menu = "📋 *Menu de Comandos*\n\n";

    if (isAdminUser) {
      menu += "*Comandos Admin:*\n";
      menu += "• /adicionar [numero] - Adiciona permitido\n";
      menu += "• /remover [numero] - Remove permitido\n";
      menu += "• /listar - Lista permitidos\n";
      menu += "• /limpar - Limpa lista\n";
      menu += "• /total - Estatísticas\n\n";
    }

    menu += "*Comandos Gerais:*\n";
    menu += "• /menu - Este menu\n";
    menu += "• /status - Status do sistema\n";
    menu += "• /info - Info do bot\n";
    menu += "• /ping - Testa conexão\n";
    menu += "• /help - Ajuda";

    whatsapp.sendText(from, menu);
    return;
  }

  else if (cmd == "status" || cmd == "/status") {
    String status = "📊 *Status do Sistema*\n\n";
    status += "✅ ESP32: Online\n";
    status += "📱 WhatsApp: Conectado\n";
    status += "📶 WiFi: " + String(WiFi.RSSI()) + " dBm\n";
    status += "⏱️ Uptime: " + String(millis() / 1000) + "s\n";
    status += "💾 Heap livre: " + String(ESP.getFreeHeap()) + " bytes\n\n";
    status += "🔒 *Controle de Acesso:*\n";
    status += "✅ Permitidos: " + String(totalPermitidos) + "\n";
    status += "🚫 Negados: " + String(tentativasNegadas) + "\n";
    status += "⚡ Comandos: " + String(comandosExecutados);

    whatsapp.sendText(from, status);
    return;
  }

  else if (cmd == "info" || cmd == "/info") {
    String info = "🤖 *Bot de Controle de Acesso*\n\n";
    info += "*Funcionalidades:*\n";
    info += "• Sistema de whitelist\n";
    info += "• Lista salva na Flash\n";
    info += "• Gerenciamento remoto\n";
    info += "• Validação automática\n\n";
    info += "*Seu Acesso:*\n";
    info += isAdminUser ? "👑 Administrador" : "✅ Usuário Permitido";

    whatsapp.sendText(from, info);
    return;
  }

  else if (cmd == "ping" || cmd == "/ping") {
    whatsapp.sendText(from, "🏓 Pong!\n\nSistema respondendo normalmente.");
    return;
  }

  else if (cmd == "help" || cmd == "/help") {
    String help = "❓ *Ajuda*\n\n";
    help += "Este bot usa sistema de controle de acesso.\n\n";

    if (isAdminUser) {
      help += "*Como adicionar números:*\n";
      help += "```/adicionar 5511999999999```\n\n";
      help += "*Como remover:*\n";
      help += "```/remover 5511999999999```\n\n";
      help += "Use */listar* para ver todos.";
    } else {
      help += "Você tem acesso aos comandos básicos.\n";
      help += "Use */menu* para ver a lista completa.";
    }

    whatsapp.sendText(from, help);
    return;
  }

  // COMANDO NÃO RECONHECIDO
  else {
    whatsapp.sendText(from, "❓ Comando não reconhecido.\n\nEnvie */menu* para ver os comandos disponíveis.");
  }
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("   WhatsApp ESP32 - Access Control");
  Serial.println("========================================\n");

  // Configurar LEDs
  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ACCESS_GRANTED, OUTPUT);
  pinMode(LED_ACCESS_DENIED, OUTPUT);
  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_ACCESS_GRANTED, LOW);
  digitalWrite(LED_ACCESS_DENIED, LOW);

  // Conectar WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
  }

  digitalWrite(LED_STATUS, HIGH);
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

  // Carregar lista de permitidos da Flash
  loadAllowedList();

  Serial.println("\n========================================");
  Serial.println("CONFIGURAÇÃO DE ACESSO");
  Serial.println("========================================");
  Serial.println("\n👑 Admin: +" + String(ADMIN_NUMBER));
  Serial.println("✅ Números permitidos: " + String(totalPermitidos) + "/" + String(MAX_ALLOWED));
  Serial.println("\n⚠️ IMPORTANTE: Configure o ADMIN_NUMBER no código!");
  Serial.println("   Linha 40: const char* ADMIN_NUMBER = \"seu_numero\";");

  Serial.println("\n========================================");
  Serial.println("Sistema pronto! Monitorando...");
  Serial.println("========================================\n");

  // Piscar LEDs (teste)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_ACCESS_GRANTED, HIGH);
    delay(100);
    digitalWrite(LED_ACCESS_GRANTED, LOW);
    digitalWrite(LED_ACCESS_DENIED, HIGH);
    delay(100);
    digitalWrite(LED_ACCESS_DENIED, LOW);
  }
}

// ========== LOOP ==========

void loop() {
  WhatsAppMessage messages[10];
  int count = whatsapp.getMessages(messages, 10);

  if (count > 0) {
    digitalWrite(LED_STATUS, HIGH);

    Serial.printf("\n📩 %d mensagem(ns) recebida(s)\n", count);

    for (int i = 0; i < count; i++) {
      String from = messages[i].from;
      String text = messages[i].text;

      Serial.println("\n--- Mensagem " + String(i + 1) + " ---");
      Serial.println("De: " + from);
      Serial.println("Texto: " + text);

      // ========== VALIDAÇÃO DE ACESSO ==========

      if (!isAllowed(from)) {
        // ACESSO NEGADO
        tentativasNegadas++;

        Serial.println("🚫 ACESSO NEGADO!");

        // Piscar LED vermelho
        digitalWrite(LED_ACCESS_DENIED, HIGH);

        String numeroSolicitante = normalizeNumber(from);

        String msg = "🚫 *Acesso Negado*\n\n";
        msg += "Você não tem permissão para usar este bot.\n\n";
        msg += "📱 Seu número: +" + numeroSolicitante + "\n\n";
        msg += "Solicite ao administrador adicionar o número:\n";
        msg += "`" + numeroSolicitante + "`";

        whatsapp.sendText(from, msg);

        delay(1000);
        digitalWrite(LED_ACCESS_DENIED, LOW);

        continue;  // Pula para próxima mensagem
      }

      // ========== ACESSO PERMITIDO ==========

      Serial.println("✅ Acesso permitido");

      // Piscar LED verde
      digitalWrite(LED_ACCESS_GRANTED, HIGH);
      delay(200);
      digitalWrite(LED_ACCESS_GRANTED, LOW);

      // Processar comando
      processarComando(from, text);
    }

    digitalWrite(LED_STATUS, LOW);
  }

  delay(5000);
}
