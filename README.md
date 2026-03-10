# WhatsApp ESP32 Library

Biblioteca Arduino para integração de ESP32 com WhatsApp API, permitindo receber e enviar mensagens através de sua própria API WhatsApp.

## Características

- Receber mensagens de usuários
- Enviar texto para usuários individuais
- Enviar texto para grupos
- Enviar mensagens com botões
- Enviar listas de opções
- Enviar localização
- Otimizado para memória limitada do ESP32
- Polling configurável

## Requisitos

### Hardware
- ESP32 (qualquer modelo)
- Conexão WiFi

### Software
- Arduino IDE 1.8.x ou superior
- ESP32 Board Package
- Biblioteca ArduinoJson (v6.x)

### API
- Conta ativa no WPIoT
- API Key válida
- Instância WhatsApp conectada

## Instalação

### Método 1: Manual

1. Baixe os arquivos da biblioteca
2. Copie a pasta `WhatsAppESP32` para:
   - **Windows**: `Documentos/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
   - **Mac**: `~/Documents/Arduino/libraries/`
3. Reinicie a Arduino IDE

### Método 2: Arduino IDE

1. Na Arduino IDE, vá em **Sketch > Include Library > Add .ZIP Library**
2. Selecione o arquivo ZIP da biblioteca
3. Reinicie a Arduino IDE

### Instalar Dependências

No Arduino IDE, instale a biblioteca **ArduinoJson**:
1. Vá em **Tools > Manage Libraries**
2. Procure por "ArduinoJson"
3. Instale a versão 6.x

## Uso Básico

```cpp
#include <WiFi.h>
#include "WhatsAppESP32.h"

// Configurações
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "minha-instancia";

// Criar objeto WhatsApp
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

void setup() {
  Serial.begin(115200);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Inicializar WhatsApp
  whatsapp.begin();
  whatsapp.setPollInterval(5000);  // 5 segundos
}

void loop() {
  // Receber mensagens
  WhatsAppMessage messages[10];
  int count = whatsapp.getMessages(messages, 10);

  for (int i = 0; i < count; i++) {
    Serial.println(messages[i].text);

    // Responder
    whatsapp.sendText(messages[i].from, "Recebi sua mensagem!");
  }

  delay(5000);
}
```

## Exemplos

A biblioteca vem com vários exemplos prontos:

### BasicExample
Exemplo básico de recebimento e envio de mensagens.

### MenuExample
Sistema de menu interativo com comandos.

### ButtonsAndListExample
Demonstra envio de botões, listas e localização.

## Referência da API

### Construtor

```cpp
WhatsAppESP32(String url, String apiKey, String instanceKey)
```

### Métodos de Configuração

#### `void begin()`
Inicializa a biblioteca.

#### `void setPollInterval(int intervalMs)`
Define o intervalo de polling em milissegundos.

### Métodos de Status

#### `bool isConnected()`
Verifica se a instância está conectada.

**Retorno:** `true` se conectado, `false` caso contrário.

#### `String getPhoneNumber()`
Obtém o número de telefone da instância.

**Retorno:** String com o número (ex: "5511999999999").

#### `int getQueueSize()`
Obtém o número de mensagens pendentes na fila.

**Retorno:** Quantidade de mensagens.

### Receber Mensagens

#### `int getMessages(WhatsAppMessage* messages, int maxMessages)`
Obtém mensagens não processadas. As mensagens são automaticamente removidas da fila ao serem lidas.

**Parâmetros:**
- `messages`: Array para armazenar as mensagens
- `maxMessages`: Número máximo de mensagens a buscar

**Retorno:** Número de mensagens recebidas.

**Exemplo:**
```cpp
WhatsAppMessage msgs[10];
int count = whatsapp.getMessages(msgs, 10);

for (int i = 0; i < count; i++) {
  Serial.println(msgs[i].text);
}
```

### Enviar Mensagens

#### `bool sendText(String to, String text)`
Envia mensagem de texto para um número.

**Parâmetros:**
- `to`: Número do destinatário (ex: "5511999999999")
- `text`: Texto da mensagem

**Retorno:** `true` se enviado com sucesso.

**Exemplo:**
```cpp
whatsapp.sendText("5511999999999", "Olá!");
```

#### `bool sendTextToGroup(String groupId, String text)`
Envia mensagem de texto para um grupo.

**Parâmetros:**
- `groupId`: ID do grupo (pode ser com ou sem @g.us)
- `text`: Texto da mensagem

**Retorno:** `true` se enviado com sucesso.

**Exemplo:**
```cpp
whatsapp.sendTextToGroup("120363123456@g.us", "Olá grupo!");
```

#### `bool sendButtons(String to, String text, String* buttons, int buttonCount)`
Envia mensagem com botões.

**Parâmetros:**
- `to`: Número do destinatário
- `text`: Texto da mensagem
- `buttons`: Array de strings com os textos dos botões
- `buttonCount`: Número de botões

**Retorno:** `true` se enviado com sucesso.

**Exemplo:**
```cpp
String buttons[] = {"Sim", "Não", "Talvez"};
whatsapp.sendButtons("5511999999999", "Confirma?", buttons, 3);
```

#### `bool sendList(String to, String text, String title, String buttonText, ListSection* sections, int sectionCount)`
Envia mensagem com lista de opções.

**Parâmetros:**
- `to`: Número do destinatário
- `text`: Texto da mensagem
- `title`: Título da lista
- `buttonText`: Texto do botão
- `sections`: Array de seções
- `sectionCount`: Número de seções

**Retorno:** `true` se enviado com sucesso.

**Exemplo:**
```cpp
String opcoes[] = {"Opção 1", "Opção 2", "Opção 3"};
ListSection section;
section.title = "Escolha";
section.rows = opcoes;
section.rowCount = 3;

whatsapp.sendList("5511999999999", "Selecione", "Menu", "Ver", &section, 1);
```

#### `bool sendLocation(String to, float latitude, float longitude)`
Envia localização.

**Parâmetros:**
- `to`: Número do destinatário
- `latitude`: Latitude
- `longitude`: Longitude

**Retorno:** `true` se enviado com sucesso.

**Exemplo:**
```cpp
whatsapp.sendLocation("5511999999999", -23.550520, -46.633308);
```

## Autenticação

A biblioteca usa autenticação via API Key. Certifique-se de:

1. Ter uma conta válida no WPIoT
2. Ter uma API Key ativa
3. Ter uma instância WhatsApp conectada

## Configurações Avançadas

### Intervalo de Polling

```cpp
// Verificar mensagens a cada 3 segundos
whatsapp.setPollInterval(3000);

// Verificar mensagens a cada 10 segundos (economizar bateria)
whatsapp.setPollInterval(10000);
```

### Timeout HTTP

O timeout padrão é de 10 segundos. Para alterar, edite `WhatsAppESP32.h`:

```cpp
#define HTTP_TIMEOUT 15000  // 15 segundos
```

### Máximo de Mensagens

Por padrão, a biblioteca busca até 10 mensagens por vez. Para alterar:

```cpp
#define MAX_MESSAGES 20  // Buscar até 20 mensagens
```

**Atenção:** Valores muito altos podem causar problemas de memória no ESP32.

## Estrutura WhatsAppMessage

```cpp
struct WhatsAppMessage {
    String id;              // ID único da mensagem
    String from;            // Número do remetente
    String text;            // Texto da mensagem
    unsigned long timestamp; // Timestamp Unix
};
```

## Troubleshooting

Consulte o arquivo [TROUBLESHOOTING.md](TROUBLESHOOTING.md) para soluções detalhadas.

## Licença

Este projeto está sob licença MIT.

## Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou pull requests.

## Suporte

Para suporte, abra uma issue no repositório do projeto.
