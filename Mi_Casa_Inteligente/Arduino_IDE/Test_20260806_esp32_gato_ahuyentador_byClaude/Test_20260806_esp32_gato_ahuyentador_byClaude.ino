/*
  Proyecto: Detector de gato + disuasor automático
  Hardware: NodeMCU ESP32-S + PIR HC-SR501 + Módulo relé + LED indicador
  IDE: Arduino IDE (board package "esp32" by Espressif Systems)

  Lógica:
    1. El PIR detecta movimiento -> GPIO_PIR pasa a HIGH.
    2. El ESP32 activa el relé (GPIO_RELAY) por RELAY_ON_MS milisegundos.
    3. El LED indicador (GPIO_LED) se enciende en simultáneo con el relé.
    4. Se dispara un webhook HTTP POST hacia N8N con los datos del evento.
    5. Se aplica un período de "cooldown" para evitar disparos repetidos
       por el mismo evento de movimiento prolongado.
*/

#include <WiFi.h>
#include <HTTPClient.h>

// ---------- CONFIGURACIÓN WIFI ----------
const char* WIFI_SSID     = "247IASbrosHA";
const char* WIFI_PASSWORD = "GBxT0K6be99GmznHfUb247$!";

// ---------- CONFIGURACIÓN WEBHOOK N8N ----------
// URL del webhook de N8N (nodo "Webhook" en modo Production, método POST)
const char* N8N_WEBHOOK_URL = "https://nochoncito.s3tech.dev/webhook/gato-detectado";

// ---------- PINES ----------
const int GPIO_PIR   = 27;  // Entrada digital desde el PIR
const int GPIO_RELAY = 26;  // Salida digital hacia el módulo relé
const int GPIO_LED   = 25;  // Salida digital hacia el LED indicador

// ---------- PARÁMETROS DE TIEMPO ----------
const unsigned long RELAY_ON_MS     = 3000;   // Duración de activación del relé (ms)
const unsigned long COOLDOWN_MS     = 15000;  // Tiempo mínimo entre disparos (ms)
const unsigned long PIR_WARMUP_MS   = 30000;  // Tiempo de calentamiento del PIR al boot (ms)
const unsigned long WIFI_TIMEOUT_MS = 15000;  // Timeout máximo de conexión WiFi (ms)

// ---------- ESTADO ----------
unsigned long lastTriggerTime = 0;
bool systemReady = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(GPIO_PIR, INPUT);
  pinMode(GPIO_RELAY, OUTPUT);
  pinMode(GPIO_LED, OUTPUT);

  digitalWrite(GPIO_RELAY, LOW);
  digitalWrite(GPIO_LED, LOW);

  Serial.println("Iniciando sistema de deteccion...");
  conectarWiFi();

  Serial.println("Calentando sensor PIR, por favor esperar...");
  delay(PIR_WARMUP_MS);

  systemReady = true;
  Serial.println("Sistema listo. Monitoreando movimiento.");
}

void loop() {
  // Reconexión WiFi automática si se cae la red
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Reintentando...");
    conectarWiFi();
  }

  int pirState = digitalRead(GPIO_PIR);
  unsigned long now = millis();

  if (systemReady && pirState == HIGH) {
    // Filtro de cooldown: ignora disparos repetidos demasiado seguidos
    if (now - lastTriggerTime > COOLDOWN_MS) {
      lastTriggerTime = now;
      Serial.println("Movimiento detectado -> activando disuasor");
      activarDisuasor();
      enviarNotificacionN8N();
    }
  }

  delay(200); // pequeño debounce de lectura, no afecta la respuesta real
}

// ---------- FUNCIONES ----------

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado. IP local: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No se pudo conectar al WiFi (continua en modo local).");
  }
}

void activarDisuasor() {
  digitalWrite(GPIO_RELAY, HIGH);
  digitalWrite(GPIO_LED, HIGH);
  delay(RELAY_ON_MS);
  digitalWrite(GPIO_RELAY, LOW);
  digitalWrite(GPIO_LED, LOW);
}

void enviarNotificacionN8N() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin WiFi: no se pudo enviar notificacion a N8N.");
    return;
  }

  HTTPClient http;
  http.begin(N8N_WEBHOOK_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Payload JSON simple. Ajustar campos segun lo que espere tu workflow de N8N.
  String payload = "{";
  payload += "\"evento\":\"gato_detectado\",";
  payload += "\"timestamp_ms\":" + String(millis()) + ",";
  payload += "\"dispositivo\":\"esp32_patio\"";
  payload += "}";

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.printf("Webhook N8N respondio con codigo: %d\n", httpCode);
  } else {
    Serial.printf("Error al enviar webhook: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
