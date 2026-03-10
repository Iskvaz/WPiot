# 🔧 Guia de Troubleshooting - WhatsApp ESP32

Este guia ajuda a resolver os problemas mais comuns ao usar a biblioteca WhatsAppESP32.

## 🚨 Erro: "Connection Refused"

```
[WhatsApp] Connection failed: connection refused
```

### Causas Comuns:

#### ❌ 1. API não está rodando
**Solução:**
```bash
# No servidor, verifique se a API está rodando
npm start

# Ou
pm2 status
```

#### ❌ 2. URL incorreta
**Problema:** URL sem `https://`

**Errado:**
```cpp
const char* apiUrl = "sua-instancia.wpiot.com.br";  // ❌
```

**Correto:**
```cpp
const char* apiUrl = "https://<sua-instancia>.wpiot.com.br";  // ✅
```

#### ❌ 3. Porta incorreta
**Solução:** Verifique a porta que a API está usando

```bash
# Verificar porta no servidor
netstat -tulpn | grep 80
```

Porta padrão: **80**

#### ❌ 4. ESP32 e servidor em redes diferentes
**Solução:** Ambos devem estar na mesma rede local

```cpp
// No ESP32, verifique o IP
WiFi.localIP();  // Ex: 192.168.1.50

// No servidor
ifconfig  // Linux/Mac
ipconfig  // Windows
// Ex: 192.168.1.100

// Devem estar na mesma rede: 192.168.1.x
```

#### ❌ 5. Firewall bloqueando
**Solução:** Liberar porta no firewall

**Windows:**
```powershell
netsh advfirewall firewall add rule name="WhatsApp API" dir=in action=allow protocol=TCP localport=80
```

**Linux:**
```bash
sudo ufw allow 80/tcp
```

---

## 🔑 Erro: "API Key inválida" (HTTP 401)

```
❌ API Key inválida!
```

### Soluções:

1. **Verifique a API Key:**
   ```cpp
   const char* apiKey = "SUA_API_KEY";  // Copiar exatamente da API
   ```

2. **Gerar nova API Key:**
   - Acesse o painel da API
   - Vá em Settings ou Profile
   - Copie a API Key completa

3. **Verifique espaços em branco:**
   ```cpp
   // ❌ Errado
   const char* apiKey = " abc123 ";

   // ✅ Correto
   const char* apiKey = "abc123";
   ```

---

## 📱 Erro: "Instância não encontrada" (HTTP 404)

```
❌ Instância não encontrada!
```

### Soluções:

1. **Verificar instanceKey:**
   ```cpp
   const char* instanceKey = "minha-instancia";  // Deve existir na API
   ```

2. **Criar instância na API:** Acesse o painel WPIoT e crie uma nova instância.

3. **Listar instâncias existentes:** Verifique no painel WPIoT suas instâncias ativas.

---

## 📶 Erro: "WiFi não conecta"

```
❌ ERRO: Não foi possível conectar ao WiFi!
```

### Soluções:

1. **Verificar SSID:**
   ```cpp
   const char* ssid = "Nome_Exato_Do_WiFi";  // Case-sensitive!
   ```

2. **Verificar senha:**
   ```cpp
   const char* password = "senha123";  // Senha correta
   ```

3. **WiFi 2.4GHz:**
   - ESP32 **não suporta WiFi 5GHz**
   - Use apenas redes 2.4GHz

4. **Aumentar tentativas:**
   ```cpp
   int attempts = 0;
   while (WiFi.status() != WL_CONNECTED && attempts < 30) {
     delay(500);
     Serial.print(".");
     attempts++;
   }
   ```

---

## 📚 Erro: "Biblioteca não encontrada"

```
fatal error: WhatsAppESP32.h: No such file or directory
```

### Soluções:

1. **Instalar biblioteca:**
   - Copiar pasta `esp32-library` para `Arduino/libraries/`
   - Renomear para `WhatsAppESP32`

2. **Caminho correto:**
   ```
   Windows: Documentos/Arduino/libraries/WhatsAppESP32/
   Linux: ~/Arduino/libraries/WhatsAppESP32/
   Mac: ~/Documents/Arduino/libraries/WhatsAppESP32/
   ```

3. **Reiniciar Arduino IDE**

---

## 🔨 Erro: "ArduinoJson não encontrado"

```
fatal error: ArduinoJson.h: No such file or directory
```

### Solução:

Instalar ArduinoJson via Library Manager:

1. Arduino IDE → `Tools` → `Manage Libraries`
2. Procurar: `ArduinoJson`
3. Instalar versão **6.x** (não 7.x)

---

## 🌡️ Erro: "DHT.h não encontrado"

```
fatal error: DHT.h: No such file or directory
```

### Solução:

Instalar biblioteca DHT:

1. Arduino IDE → `Tools` → `Manage Libraries`
2. Procurar: `DHT sensor library`
3. Instalar por **Adafruit**

**Nota:** Necessário apenas para:
- `SensorMonitor.ino`
- `HomeAutomation.ino`

---

## 💾 Erro: "Memória insuficiente"

```
region `dram0_0_seg' overflowed by X bytes
```

### Soluções:

1. **Reduzir tamanho do buffer:**
   ```cpp
   #define MAX_MESSAGES 5  // Ao invés de 10
   ```

2. **Usar Partition Scheme maior:**
   - Arduino IDE → `Tools` → `Partition Scheme`
   - Selecionar: `Huge APP (3MB No OTA)`

3. **Otimizar código:**
   ```cpp
   // Evitar Strings muito longas
   String msg = "Curto";  // ✅

   // Ao invés de
   String msg = "Muito texto muito texto...";  // ❌
   ```

---

## 🔄 Erro: "Watchdog reset"

```
Guru Meditation Error: Core 0 panic'ed (Interrupt wdt timeout on CPU0)
```

### Soluções:

1. **Adicionar delays:**
   ```cpp
   void loop() {
     // ...código...
     delay(100);  // ✅ Importante!
   }
   ```

2. **Usar yield():**
   ```cpp
   while (condition) {
     yield();  // Libera CPU
   }
   ```

3. **Desabilitar Watchdog (não recomendado):**
   ```cpp
   #include "esp_task_wdt.h"

   void setup() {
     esp_task_wdt_init(30, false);  // 30 segundos
   }
   ```

---

## 🧪 Como Diagnosticar Problemas

### 1. Use o Exemplo de Diagnóstico:

```cpp
File → Examples → WhatsAppESP32 → NetworkDiagnostic
```

Este exemplo testa:
- ✅ Conexão WiFi
- ✅ Resolução de IP
- ✅ Ping para servidor
- ✅ Conexão HTTP
- ✅ API Key
- ✅ Instância

### 2. Ative Debug no Serial Monitor:

```cpp
void setup() {
  Serial.begin(115200);  // ✅ 115200 baud
  delay(2000);

  Serial.println("Iniciando...");
}
```

### 3. Verifique Serial Monitor:

- Baud rate: **115200**
- Line ending: `Both NL & CR` ou `Newline`

---

## 📝 Checklist Rápido

Antes de reportar um problema, verifique:

- [ ] WiFi está conectado? (`WiFi.localIP()`)
- [ ] URL tem `https://`?
- [ ] API está rodando no servidor?
- [ ] URL da API está correta?
- [ ] ESP32 e servidor na mesma rede?
- [ ] API Key está correta?
- [ ] Instância existe e está online?
- [ ] ArduinoJson está instalado?
- [ ] Placa ESP32 selecionada?
- [ ] Serial Monitor em 115200 baud?

---

## 🆘 Ainda com Problemas?

### 1. Execute NetworkDiagnostic:

```
File → Examples → WhatsAppESP32 → NetworkDiagnostic
```

Copie a saída completa do Serial Monitor.

### 2. Verifique os Logs:

**ESP32:**
```
Serial Monitor → 115200 baud
```

**Servidor:**
```bash
# Verificar logs da API
pm2 logs

# Ou se rodando direto
# Veja a saída no terminal
```

### 3. Teste Manual:

Use o exemplo **NetworkDiagnostic** para diagnosticar a conexão passo a passo.
O exemplo testa WiFi, DNS, HTTP e a conexão com a API automaticamente.

---

## 💡 Dicas Gerais

### ✅ Boas Práticas:

1. **Sempre começar pelo BasicExample**
2. **Usar Serial.println() para debug**
3. **Verificar WiFi antes de tudo**
4. **Testar um exemplo por vez**
5. **Verificar logs do servidor**

### ⚠️ Evitar:

1. ❌ Não usar delays muito curtos (< 1000ms)
2. ❌ Não fazer polling muito rápido
3. ❌ Não esquecer do `http://` na URL
4. ❌ Não usar WiFi 5GHz
5. ❌ Não misturar versões do ArduinoJson

---

## 📞 Suporte

Se ainda tiver problemas:

1. Verifique os exemplos no `examples/README.md`
2. Consulte a documentação principal `README.md`
3. Rode `NetworkDiagnostic.ino` e compartilhe o resultado
4. Abra uma issue no repositório com:
   - Código usado
   - Saída do Serial Monitor
   - Logs do servidor
   - Resultado do NetworkDiagnostic

---

**Desenvolvido com ❤️ para ajudar você!**
