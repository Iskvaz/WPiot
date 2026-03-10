/*
 * WhatsAppESP32 - Biblioteca para integração ESP32 com WhatsApp API
 *
 * Esta biblioteca permite que ESP32 se comunique com sua API WhatsApp,
 * recebendo mensagens e enviando respostas.
 *
 * Autor: WPIoT
 * Versão: 1.0.0
 */

#ifndef WHATSAPP_ESP32_H
#define WHATSAPP_ESP32_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ─── Configurações ajustáveis ───────────────────────────────────────
#define WHATSAPP_ESP32_VERSION "1.0.0"
#define DEFAULT_POLL_INTERVAL  5000   // ms entre polls
#define MAX_MESSAGES           10     // mensagens por request
#define HTTP_TIMEOUT           10000  // timeout HTTP em ms

// ─── Estruturas públicas ────────────────────────────────────────────

struct WhatsAppMessage {
    String id;
    String from;
    String text;
    unsigned long timestamp;
};

struct ListSection {
    String title;
    String* rows;
    int rowCount;
};

// ─── Classe principal ───────────────────────────────────────────────

class WhatsAppESP32 {
private:
    String        _url;
    String        _key;
    String        _inst;
    unsigned long _lastPoll;
    int           _interval;
    HTTPClient    _http;

    // Métodos internos (implementação distribuída como binário)
    String _buildEndpoint(const char* path);
    bool   _post(const String& endpoint, const String& payload, String& out);
    bool   _get(const String& endpoint, String& out);

public:
    // Construtor / Destrutor
    WhatsAppESP32(String url, String key, String instance);
    ~WhatsAppESP32();

    // Configuração
    void begin();
    void setPollInterval(int intervalMs);

    // Status
    bool   isConnected();
    String getPhoneNumber();
    int    getQueueSize();

    // Receber mensagens (removidas da fila automaticamente ao serem lidas)
    int getMessages(WhatsAppMessage* messages, int maxMessages = MAX_MESSAGES);

    // Enviar mensagens
    bool sendText(String to, String text);
    bool sendTextToGroup(String groupId, String text);
    bool sendButtons(String to, String text, String* buttons, int buttonCount);
    bool sendList(String to, String text, String title, String buttonText,
                  ListSection* sections, int sectionCount);
    bool sendLocation(String to, float latitude, float longitude);

    // Loop — chamar no loop() do Arduino
    void loop();
};

#endif // WHATSAPP_ESP32_H
