/*
 * Diagnóstico de Conexão - WhatsApp ESP32
 *
 * Este exemplo ajuda a diagnosticar problemas de conexão entre ESP32 e a API.
 * Use este exemplo PRIMEIRO antes de usar os outros.
 *
 * Verifica:
 * - Conexão WiFi
 * - IP obtido
 * - Ping para o servidor
 * - Conectividade HTTP
 * - Resposta da API
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ping.h>

// ========== CONFIGURE AQUI ==========
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// IMPORTANTE: Não esqueça do "http://" no início!
const char* apiUrl = "https://app.wpiot.com.br";  // Exemplo: https://app.wpiot.com.br
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "sua-instancia";

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  DIAGNÓSTICO DE CONEXÃO - WhatsApp ESP32  ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // ========== TESTE 1: WiFi ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TESTE 1: Conexão WiFi");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  Serial.print("SSID configurado: ");
  Serial.println(ssid);
  Serial.print("Conectando");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi CONECTADO com sucesso!");
    Serial.print("   IP do ESP32: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("   Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("   DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("   RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n❌ ERRO: Não foi possível conectar ao WiFi!");
    Serial.println("   Verifique:");
    Serial.println("   - SSID está correto?");
    Serial.println("   - Senha está correta?");
    Serial.println("   - WiFi está funcionando?");
    while(1) delay(1000);
  }

  delay(2000);

  // ========== TESTE 2: Configuração da API ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TESTE 2: Configuração da API");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  Serial.print("URL da API: ");
  Serial.println(apiUrl);

  // Verificar se começa com http://
  String urlStr = String(apiUrl);
  if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://")) {
    Serial.println("❌ ERRO: URL deve começar com http:// ou https://");
    Serial.println("   Exemplo correto: https://app.wpiot.com.br");
    while(1) delay(1000);
  } else {
    Serial.println("✅ Formato da URL está correto");
  }

  // Extrair IP e porta
  String hostPort = urlStr.substring(7); // Remove "http://"
  int slashPos = hostPort.indexOf('/');
  if (slashPos > 0) hostPort = hostPort.substring(0, slashPos);

  int colonPos = hostPort.indexOf(':');
  String host;
  int port = 80;

  if (colonPos > 0) {
    host = hostPort.substring(0, colonPos);
    port = hostPort.substring(colonPos + 1).toInt();
  } else {
    host = hostPort;
  }

  Serial.print("   Host: ");
  Serial.println(host);
  Serial.print("   Porta: ");
  Serial.println(port);

  delay(2000);

  // ========== TESTE 3: Ping ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TESTE 3: Ping para o servidor");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  Serial.print("Testando ping para ");
  Serial.print(host);
  Serial.println("...");

  IPAddress serverIP;
  if (WiFi.hostByName(host.c_str(), serverIP)) {
    Serial.print("   IP resolvido: ");
    Serial.println(serverIP);

    // Verificar se está na mesma rede
    IPAddress espIP = WiFi.localIP();
    IPAddress subnet = WiFi.subnetMask();

    bool sameNetwork = true;
    for (int i = 0; i < 4; i++) {
      if ((espIP[i] & subnet[i]) != (serverIP[i] & subnet[i])) {
        sameNetwork = false;
        break;
      }
    }

    if (sameNetwork) {
      Serial.println("✅ Servidor está na mesma rede local");
    } else {
      Serial.println("⚠️  AVISO: Servidor pode estar em rede diferente");
    }

    // Fazer ping
    bool success = Ping.ping(serverIP, 3);
    if (success) {
      Serial.println("✅ Ping OK - Servidor está acessível!");
    } else {
      Serial.println("❌ Ping FALHOU - Servidor não responde");
      Serial.println("   Possíveis causas:");
      Serial.println("   - Servidor está desligado");
      Serial.println("   - Firewall bloqueando ping");
      Serial.println("   - IP incorreto");
    }
  } else {
    Serial.println("❌ Não foi possível resolver o hostname");
  }

  delay(2000);

  // ========== TESTE 4: Conexão HTTP ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TESTE 4: Conexão HTTP");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  HTTPClient http;
  String testUrl = String(apiUrl) + "/health";

  Serial.print("Testando endpoint: ");
  Serial.println(testUrl);

  http.begin(testUrl);
  http.setTimeout(10000);

  int httpCode = http.GET();

  Serial.print("Código HTTP: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    Serial.println("✅ Conexão HTTP estabelecida!");

    if (httpCode == 200) {
      String response = http.getString();
      Serial.println("✅ API respondeu corretamente!");
      Serial.println("\nResposta da API:");
      Serial.println(response);
    } else if (httpCode == 404) {
      Serial.println("⚠️  Endpoint /health não encontrado");
      Serial.println("   A API pode não ter este endpoint");
      Serial.println("   Mas a conexão HTTP funciona!");
    } else {
      Serial.print("⚠️  Código HTTP inesperado: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.println("❌ ERRO na conexão HTTP!");
    Serial.print("   Erro: ");
    Serial.println(http.errorToString(httpCode));

    if (httpCode == -1) {
      Serial.println("\n   Possíveis causas:");
      Serial.println("   - API não está rodando");
      Serial.println("   - Porta incorreta");
      Serial.println("   - Firewall bloqueando");
    } else if (httpCode == -11) {
      Serial.println("\n   Erro: connection refused");
      Serial.println("   - Verifique se a API está rodando");
      Serial.println("   - Verifique a porta (padrão: 3000)");
    }
  }

  http.end();

  delay(2000);

  // ========== TESTE 5: Endpoint ESP32 ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TESTE 5: Endpoint ESP32 da API");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  String esp32Url = String(apiUrl) + "/esp32/status/" + instanceKey;

  Serial.print("Testando: ");
  Serial.println(esp32Url);

  http.begin(esp32Url);
  http.setTimeout(10000);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", apiKey);

  httpCode = http.GET();

  Serial.print("Código HTTP: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String response = http.getString();

    if (httpCode == 200) {
      Serial.println("✅ Endpoint ESP32 funcionando!");
      Serial.println("\nResposta:");
      Serial.println(response);
    } else if (httpCode == 401) {
      Serial.println("❌ API Key inválida!");
      Serial.println("   Verifique se a API Key está correta");
    } else if (httpCode == 404) {
      Serial.println("❌ Instância não encontrada!");
      Serial.println("   Verifique se o instanceKey está correto");
    } else {
      Serial.print("⚠️  Código: ");
      Serial.println(httpCode);
      Serial.println(response);
    }
  } else {
    Serial.println("❌ Erro na requisição");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();

  // ========== RESUMO FINAL ==========
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║           RESUMO DO DIAGNÓSTICO           ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("\nSe todos os testes passaram, a biblioteca deve funcionar!");
  Serial.println("\nSe algum teste falhou, siga as instruções acima.");
  Serial.println("\nPara mais ajuda, veja:");
  Serial.println("- esp32-library/examples/README.md");
  Serial.println("- esp32-library/README.md\n");
}

void loop() {
  // Nada no loop - diagnóstico roda apenas no setup
  delay(10000);
}
