/*
 * WhatsAppESP32 - Biblioteca para integração ESP32 com WhatsApp API
 *
 * Esta biblioteca permite que ESP32 se comunique com sua API WhatsApp,
 * recebendo mensagens e enviando respostas.
 *
 * Autor: WhatsAPI ESP32
 * Versão: 1.0.0
 * Data: 2025-01-26
 */

#ifndef WHATSAPP_ESP32_H
#define WHATSAPP_ESP32_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configurações padrão
#define WHATSAPP_ESP32_VERSION "1.0.0"
#define DEFAULT_POLL_INTERVAL 5000  // 5 segundos
#define MAX_MESSAGES 10              // Máximo de mensagens por request
#define HTTP_TIMEOUT 10000           // 10 segundos timeout

// Estrutura para armazenar uma mensagem recebida
struct WhatsAppMessage {
    String id;
    String from;
    String text;
    unsigned long timestamp;
};

// Estrutura para seção de lista
struct ListSection {
    String title;
    String* rows;
    int rowCount;
};

class WhatsAppESP32 {
private:
    String apiUrl;          // URL base da API (ex: "https://app.wpiot.com.br")
    String apiKey;          // API Key para autenticação
    String instanceKey;     // Chave da instância WhatsApp

    unsigned long lastPoll; // Timestamp do último poll
    int pollInterval;       // Intervalo entre polls em ms

    HTTPClient http;

    // Métodos privados auxiliares
    bool httpRequest(String method, String endpoint, String payload, String& response);
    void parseMessages(String json, WhatsAppMessage* messages, int& count);

public:
    // Construtor
    WhatsAppESP32(String url, String key, String instance);

    // Destrutor
    ~WhatsAppESP32();

    // Configuração
    void setPollInterval(int intervalMs);
    void begin();

    // Status
    bool isConnected();
    String getPhoneNumber();
    int getQueueSize();

    // Receber mensagens
    // NOTA: As mensagens são automaticamente removidas da fila ao serem lidas
    int getMessages(WhatsAppMessage* messages, int maxMessages = MAX_MESSAGES);

    // Enviar mensagens - Texto
    bool sendText(String to, String text);
    bool sendTextToGroup(String groupId, String text);

    // Enviar mensagens - Botões
    bool sendButtons(String to, String text, String* buttons, int buttonCount);

    // Enviar mensagens - Lista
    bool sendList(String to, String text, String title, String buttonText,
                  ListSection* sections, int sectionCount);

    // Enviar mensagens - Localização
    bool sendLocation(String to, float latitude, float longitude);

    // Loop - chamar no loop() do Arduino
    void loop();
};

#endif // WHATSAPP_ESP32_H
