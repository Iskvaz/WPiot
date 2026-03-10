# Exemplos da Biblioteca WhatsApp ESP32

Esta pasta contém 12 exemplos completos demonstrando como usar a biblioteca WhatsAppESP32 em diferentes aplicações.

## 📚 Bibliotecas Necessárias

### Obrigatórias (Todos os Exemplos)
- **WhatsAppESP32** - Esta biblioteca
- **ArduinoJson** (v6.x) - Manipulação de JSON
  - Instalar via Arduino IDE: `Tools > Manage Libraries > "ArduinoJson"`

### Opcionais (Exemplos Específicos)
- **DHT sensor library** - Para SensorMonitor e HomeAutomation
  - Instalar via Arduino IDE: `Tools > Manage Libraries > "DHT sensor library"`
  - Necessário para: `SensorMonitor.ino`, `HomeAutomation.ino`

## 📋 Lista de Exemplos

### 1. ⭐ BasicExample
**Dificuldade:** Básico
**Pinos:** Nenhum hardware externo necessário
**O que faz:** Recebe mensagens e responde automaticamente.

**Ideal para:** Aprender o básico da biblioteca.

---

### 2. ⭐ MenuExample
**Dificuldade:** Básico
**Pinos:** LED onboard (GPIO 2)
**O que faz:** Menu interativo com comandos de sistema e controle de LED.

**Comandos:**
- `menu` - Ver menu
- `status` - Status do ESP32
- `led on/off` - Controlar LED
- `info` - Informações do sistema

**Ideal para:** Aprender comandos interativos.

---

### 3. ⭐⭐ ButtonsAndListExample
**Dificuldade:** Intermediário
**Pinos:** Nenhum hardware externo necessário
**O que faz:** Demonstra envio de botões, listas e localização.

**Comandos:**
- `menu` - Ver menu
- `botoes` - Enviar botões
- `lista` - Enviar lista de opções
- `local` - Enviar localização

**Ideal para:** Aprender interface avançada do WhatsApp.

---

### 4. ⭐⭐ ControlePins
**Dificuldade:** Intermediário
**Pinos:**
- LEDs: GPIO 2, 4
- Relés: GPIO 5, 18
- Botões: GPIO 19, 21 (com pull-up)

**O que faz:** Controle completo de pinos digitais (LEDs, relés, botões).

**Comandos:**
- `status` - Ver status de todos os pinos
- `led1/led2 on/off` - Controlar LEDs
- `rele1/rele2 on/off` - Controlar relés
- `todos on/off` - Ligar/desligar tudo

**Ideal para:** Automação básica, controle de dispositivos.

---

### 5. ⭐⭐⭐ SensorMonitor
**Dificuldade:** Avançado
**Pinos:**
- DHT22: GPIO 4
- LDR: GPIO 34
- Solo: GPIO 35
- Potenciômetro: GPIO 32
- PIR: GPIO 15

**Bibliotecas Extras:** DHT sensor library

**O que faz:** Monitoramento completo de sensores com alertas automáticos.

**Comandos:**
- `leituras` - Ver leituras atuais
- `relatorio` - Relatório completo
- `alertas on/off` - Ativar/desativar alertas

**Ideal para:** Estufas, monitoramento ambiental.

---

### 6. ⭐⭐⭐⭐ HomeAutomation
**Dificuldade:** Expert
**Pinos:**
- Luzes: GPIO 2, 4, 5, 18, 19
- Ventilador: GPIO 21
- Ar Condicionado: GPIO 22
- Bomba: GPIO 23
- DHT22: GPIO 25
- Sensores: GPIO 15, 16, 17, 34

**Bibliotecas Extras:** DHT sensor library

**O que faz:** Sistema completo de casa inteligente com cenas predefinidas.

**Cenas:**
- `sair` - Sair de casa (apaga tudo, ativa alarme)
- `chegar` - Chegar em casa (acende luzes principais)
- `dormir` - Modo noturno
- `acordar` - Modo matinal

**Comandos:**
- `status` - Status completo da casa
- `sala/quarto/cozinha on/off` - Controlar luzes
- `ventilador/ar on/off` - Controlar clima
- `irrigar` - Irrigar jardim

**Ideal para:** Automação residencial completa.

---

### 7. ⭐⭐⭐⭐ AlarmSystem
**Dificuldade:** Expert
**Pinos:**
- Sensores Porta/Janela: GPIO 15, 16, 17
- PIR: GPIO 18, 19
- Sirene: GPIO 2
- LED Status: GPIO 4
- Buzzer: GPIO 5

**O que faz:** Sistema profissional de alarme com 5 zonas de segurança.

**Comandos:**
- `armar` - Armar alarme (30s para sair)
- `desarmar` - Desarmar alarme
- `status` - Ver status das zonas
- `teste` - Testar sensores
- `zona1-5 on/off` - Ativar/desativar zonas
- `historico` - Ver histórico de eventos

**Estados:**
1. Desarmado
2. Ativando (30s)
3. Armado (monitorando)
4. Alerta (15s para desarmar)
5. Disparado (sirene ativa)

**Ideal para:** Segurança residencial.

---

### 8. ⭐⭐ GroupMessaging
**Dificuldade:** Intermediário
**Pinos:** LED de status (GPIO 2)

**O que faz:** Envio e recebimento de mensagens em grupos do WhatsApp.

**Recursos:**
- Enviar mensagens para grupos específicos
- Receber e processar mensagens de grupos
- Identificar grupo vs mensagem privada
- Broadcast para múltiplos grupos
- Relatórios automáticos para grupos
- Comandos específicos para grupos

**Comandos:**
- `/menu` - Menu de comandos
- `/status` - Status do sistema
- `/relatorio` - Relatório completo
- `/temp` - Temperatura atual
- `/broadcast [msg]` - Enviar para todos os grupos
- `/led on/off` - Controlar LED

**Ideal para:** Bots de grupos, notificações para equipes, monitoramento compartilhado.

---

### 9. ⭐⭐⭐ BroadcastExample
**Dificuldade:** Avançado
**Pinos:**
- LED Alerta: GPIO 2
- Sensor Temperatura: GPIO 34 (analógico)
- Sensor Movimento: GPIO 15 (PIR)

**O que faz:** Sistema de broadcast (envio em massa) para múltiplos contatos.

**Recursos:**
- Listas de distribuição personalizadas (Família, Admins, Técnica, VIP)
- Envio simultâneo para múltiplos contatos
- Mensagens personalizadas por contato
- Alertas críticos para todas as listas
- Relatórios automáticos para grupos específicos
- Monitoramento com alertas automáticos

**Comandos:**
- `/teste familia` - Testa lista família
- `/teste admins` - Testa lista admins
- `/teste tecnica` - Testa lista técnica
- `/teste geral` - Testa todas as listas
- `/relatorio` - Envia relatório
- `/alerta teste` - Simula alerta

**Ideal para:** Alertas de segurança, notificações em massa, sistemas de emergência.

---

### 10. ⭐⭐⭐⭐ GroupManagement
**Dificuldade:** Expert
**Pinos:**
- LED Atividade: GPIO 2
- LED Alerta: GPIO 4

**O que faz:** Sistema completo de gerenciamento e monitoramento de grupos WhatsApp.

**Recursos:**
- Gerenciar múltiplos grupos simultaneamente
- Monitoramento de atividade em grupos
- Estatísticas detalhadas por grupo
- Sistema de permissões (admins autorizados)
- Comandos de administração
- Relatórios automáticos periódicos
- Broadcast inteligente para grupos

**Comandos Gerais:**
- `/menu` - Menu de comandos
- `/stats` - Estatísticas do grupo
- `/status` - Status do sistema
- `/uptime` - Tempo online
- `/memoria` - Uso de memória
- `/wifi` - Informações WiFi

**Comandos Admin:**
- `/relatorio` - Relatório completo
- `/broadcast [msg]` - Envia para todos os grupos
- `/listar grupos` - Lista grupos gerenciados

**Ideal para:** Bots de administração, moderação automatizada, sistemas corporativos.

---

### 11. ⭐⭐ NetworkDiagnostic
**Dificuldade:** Intermediário
**Pinos:** LED onboard (GPIO 2)

**O que faz:** Diagnóstico completo de rede e conectividade.

**Comandos:**
- `diagnostico` - Diagnóstico completo
- `ping` - Testa conectividade
- `wifi` - Informações WiFi
- `reconectar` - Reconectar WiFi

**Ideal para:** Debug de problemas de conectividade.

---

### 12. ⭐⭐⭐ AccessControl
**Dificuldade:** Avançado
**Pinos:**
- LED Status: GPIO 2
- LED Acesso Permitido: GPIO 4 (verde)
- LED Acesso Negado: GPIO 5 (vermelho)

**O que faz:** Sistema completo de controle de acesso com whitelist gerenciável.

**Recursos:**
- Administrador fixo definido no código
- Lista de números permitidos armazenada na Flash (NVS)
- Validação automática de TODAS as mensagens
- Gerenciamento remoto da lista via WhatsApp
- Persistência de dados (mantém lista após restart)
- Bloqueio automático de não autorizados
- Estatísticas de acesso (permitidos/negados)
- LEDs indicadores visuais de acesso

**Comandos Admin:**
- `/adicionar [numero]` - Adiciona número à whitelist
- `/remover [numero]` - Remove número da whitelist
- `/listar` - Lista todos os números permitidos
- `/limpar` - Limpa toda a lista
- `/total` - Estatísticas de acesso

**Comandos Usuários Permitidos:**
- `/menu` - Menu de comandos
- `/status` - Status completo do sistema
- `/info` - Informações do bot
- `/ping` - Testa conexão
- `/help` - Ajuda

**Comportamento:**
- Números NÃO autorizados recebem mensagem de acesso negado
- Admin tem acesso total sempre
- Lista persiste na Flash (sobrevive a reinicializações)
- Máximo de 50 números permitidos (configurável)

**Ideal para:** Sistemas de segurança, bots privados, controle de acesso residencial/corporativo.

---

## 🚀 Como Usar

### 1. Instalar Bibliotecas

No Arduino IDE:
```
Tools > Manage Libraries
```

Instalar:
- ArduinoJson (v6.x) - **OBRIGATÓRIO**
- DHT sensor library - Opcional (para SensorMonitor e HomeAutomation)

### 2. Abrir Exemplo

```
File > Examples > WhatsAppESP32 > [Nome do Exemplo]
```

### 3. Configurar Credenciais

Editar no topo do arquivo:
```cpp
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";
const char* apiKey = "SUA_API_KEY";
const char* instanceKey = "sua-instancia";
```

### 4. Conectar Hardware

Seguir o diagrama de pinos no cabeçalho de cada exemplo.

### 5. Upload

- Selecionar placa ESP32
- Selecionar porta COM
- Upload (Ctrl+U)

### 6. Testar

- Abrir Serial Monitor (115200 baud)
- Enviar "menu" via WhatsApp para seu número
- Seguir os comandos

---

## 📝 Notas Importantes

### LED_BUILTIN
Alguns exemplos usam `LED_BUILTIN`. Se sua placa não define isso automaticamente, o GPIO 2 será usado (padrão ESP32).

### DHT22
Para exemplos com DHT22:
1. Instalar biblioteca "DHT sensor library" by Adafruit
2. Conectar:
   - VCC → 3.3V
   - GND → GND
   - DATA → GPIO especificado
   - Resistor pull-up 10kΩ entre VCC e DATA (recomendado)

### Sensores Analógicos
ESP32 tem ADC de 12 bits (0-4095). Sensores analógicos devem ser conectados aos pinos ADC1 (GPIO 32-39).

### Relés
Use relés de 3.3V ou 5V com transistor/optoacoplador para proteção do ESP32.

---

## 🔧 Troubleshooting

### "LED_BUILTIN not declared"
Alguns exemplos definem automaticamente. Se o erro persistir, adicione no topo:
```cpp
#define LED_BUILTIN 2
```

### "DHT.h: No such file"
Instale a biblioteca "DHT sensor library" via Library Manager.

### "Compilation error"
Verifique se:
- Arduino IDE está atualizado
- Placa ESP32 está instalada
- Bibliotecas necessárias estão instaladas

### "Connection refused"
Verifique se:
- API está rodando
- URL está correta (inclua http://)
- ESP32 e servidor estão na mesma rede
- Firewall não está bloqueando

---

## 💡 Dicas

1. **Comece pelo BasicExample** para testar a conexão
2. **Use Serial Monitor** para debug (115200 baud)
3. **Teste comandos** enviando via WhatsApp
4. **Adapte os exemplos** para seu projeto
5. **Consulte o README principal** para API completa

---

## 📦 Estrutura dos Exemplos

Todos os exemplos seguem esta estrutura:
```cpp
// 1. Includes
#include <WiFi.h>
#include "WhatsAppESP32.h"

// 2. Configurações
const char* ssid = "...";
// ...

// 3. Objeto WhatsApp
WhatsAppESP32 whatsapp(apiUrl, apiKey, instanceKey);

// 4. Setup
void setup() {
  // Conectar WiFi
  // Configurar pinos
  // Inicializar WhatsApp
}

// 5. Processamento de Comandos
void processCommand(String from, String cmd) {
  // Lógica dos comandos
}

// 6. Loop Principal
void loop() {
  // Obter mensagens
  // Processar comandos
  // delay()
}
```

---

## 🤝 Contribuindo

Encontrou um bug ou tem uma sugestão de exemplo? Abra uma issue no repositório!

---

**Desenvolvido com ❤️ para a comunidade ESP32**
