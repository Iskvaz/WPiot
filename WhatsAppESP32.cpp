/*
 * WhatsAppESP32 - Implementação da biblioteca
 */

#include "WhatsAppESP32.h"

// ==================== CONSTRUTOR E DESTRUTOR ====================

WhatsAppESP32::WhatsAppESP32(String url, String key, String instance) {
    this->apiUrl = url;
    this->apiKey = key;
    this->instanceKey = instance;
    this->pollInterval = DEFAULT_POLL_INTERVAL;
    this->lastPoll = 0;
}

WhatsAppESP32::~WhatsAppESP32() {
    http.end();
}

// ==================== CONFIGURAÇÃO ====================
void WhatsAppESP32::setPollInterval(int intervalMs) {
    this->pollInterval = intervalMs;
}

void WhatsAppESP32::begin() {
    Serial.println("[WhatsApp] Biblioteca inicializada");
    Serial.print("[WhatsApp] API URL: ");
    Serial.println(apiUrl);
    Serial.print("[WhatsApp] Instance: ");
    Serial.println(instanceKey);
}

// ==================== MÉTODOS PRIVADOS ====================
bool WhatsAppESP32::httpRequest(String method, String endpoint, String payload, String& response) {
    String url = apiUrl + endpoint;

    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);

    int httpCode;

    if (method == "GET") {
        httpCode = http.GET();
    } else if (method == "POST") {
        httpCode = http.POST(payload);
    } else {
        http.end();
        return false;
    }

    if (httpCode > 0) {
        response = http.getString();
        http.end();

        if (httpCode == 200) {
            return true;
        } else {
            Serial.print("[WhatsApp] HTTP Error: ");
            Serial.println(httpCode);
            Serial.println(response);
            return false;
        }
    } else {
        Serial.print("[WhatsApp] Connection failed: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return false;
    }
}

void WhatsAppESP32::parseMessages(String json, WhatsAppMessage* messages, int& count) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[WhatsApp] JSON parse error: ");
        Serial.println(error.c_str());
        count = 0;
        return;
    }

    JsonArray msgArray = doc["messages"];
    count = min((int)msgArray.size(), MAX_MESSAGES);

    for (int i = 0; i < count; i++) {
        messages[i].id = msgArray[i]["id"].as<String>();
        messages[i].from = msgArray[i]["from"].as<String>();
        messages[i].text = msgArray[i]["text"].as<String>();
        messages[i].timestamp = msgArray[i]["time"].as<unsigned long>();
    }
}

// ==================== STATUS ====================

bool WhatsAppESP32::isConnected() {
    String response;
    String endpoint = "/esp32/status?key=" + instanceKey;

    if (httpRequest("GET", endpoint, "", response)) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, response);
        return doc["connected"].as<bool>();
    }

    return false;
}

String WhatsAppESP32::getPhoneNumber() {
    String response;
    String endpoint = "/esp32/status?key=" + instanceKey;

    if (httpRequest("GET", endpoint, "", response)) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, response);
        return doc["phone"].as<String>();
    }

    return "";
}

int WhatsAppESP32::getQueueSize() {
    String response;
    String endpoint = "/esp32/status?key=" + instanceKey;

    if (httpRequest("GET", endpoint, "", response)) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, response);
        return doc["queueSize"].as<int>();
    }

    return 0;
}

// ==================== RECEBER MENSAGENS ====================

int WhatsAppESP32::getMessages(WhatsAppMessage* messages, int maxMessages) {
    String response;
    String endpoint = "/esp32/messages?key=" + instanceKey + "&limit=" + String(maxMessages);

    if (httpRequest("GET", endpoint, "", response)) {
        int count = 0;
        parseMessages(response, messages, count);

        // Mensagens são automaticamente removidas da fila no servidor
        // Não é necessário fazer acknowledge

        return count;
    }

    return 0;
}

// ==================== ENVIAR TEXTO ====================

bool WhatsAppESP32::sendText(String to, String text) {
    String response;
    String endpoint = "/esp32/send?key=" + instanceKey;

    DynamicJsonDocument doc(1024);
    doc["to"] = to;
    doc["text"] = text;

    String payload;
    serializeJson(doc, payload);

    return httpRequest("POST", endpoint, payload, response);
}

bool WhatsAppESP32::sendTextToGroup(String groupId, String text) {
    // Se o groupId não tiver @g.us, adicionar
    if (groupId.indexOf("@g.us") == -1) {
        groupId += "@g.us";
    }

    return sendText(groupId, text);
}

// ==================== ENVIAR BOTÕES ====================

bool WhatsAppESP32::sendButtons(String to, String text, String* buttons, int buttonCount) {
    String response;
    String endpoint = "/esp32/send?key=" + instanceKey;

    DynamicJsonDocument doc(2048);
    doc["to"] = to;
    doc["text"] = text;

    JsonArray btnArray = doc.createNestedArray("buttons");
    for (int i = 0; i < buttonCount; i++) {
        btnArray.add(buttons[i]);
    }

    String payload;
    serializeJson(doc, payload);

    return httpRequest("POST", endpoint, payload, response);
}

// ==================== ENVIAR LISTA ====================

bool WhatsAppESP32::sendList(String to, String text, String title, String buttonText,
                             ListSection* sections, int sectionCount) {
    String response;
    String endpoint = "/esp32/send?key=" + instanceKey;

    DynamicJsonDocument doc(4096);
    doc["to"] = to;
    doc["text"] = text;

    JsonObject list = doc.createNestedObject("list");
    list["title"] = title;
    list["buttonText"] = buttonText;

    JsonArray sectionsArray = list.createNestedArray("sections");

    for (int i = 0; i < sectionCount; i++) {
        JsonObject section = sectionsArray.createNestedObject();
        section["title"] = sections[i].title;

        JsonArray rowsArray = section.createNestedArray("rows");
        for (int j = 0; j < sections[i].rowCount; j++) {
            JsonObject row = rowsArray.createNestedObject();
            row["title"] = sections[i].rows[j];
            row["rowId"] = "row_" + String(i) + "_" + String(j);
        }
    }

    String payload;
    serializeJson(doc, payload);

    return httpRequest("POST", endpoint, payload, response);
}

// ==================== ENVIAR LOCALIZAÇÃO ====================

bool WhatsAppESP32::sendLocation(String to, float latitude, float longitude) {
    String response;
    String endpoint = "/esp32/send?key=" + instanceKey;

    DynamicJsonDocument doc(512);
    doc["to"] = to;

    JsonObject location = doc.createNestedObject("location");
    location["lat"] = latitude;
    location["lng"] = longitude;

    String payload;
    serializeJson(doc, payload);

    return httpRequest("POST", endpoint, payload, response);
}

// ==================== LOOP ====================

void WhatsAppESP32::loop() {
    // Este método pode ser usado para polling automático
    // Por enquanto está vazio, deixando o controle para o usuário
}
