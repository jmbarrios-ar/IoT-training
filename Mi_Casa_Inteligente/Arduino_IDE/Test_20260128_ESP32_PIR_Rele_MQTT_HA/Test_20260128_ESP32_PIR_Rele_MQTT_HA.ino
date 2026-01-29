// ALARMA PIR movimiento con modulo relé y MQTT para Home Assistant //
#include <WiFi.h>
#include <PubSubClient.h>

// ============ CONFIG WIFI ============
const char* ssid = "247IASbrosHA";
const char* password = "GBxT0K6be99GmznHfUb247$!";

// ============ CONFIG MQTT ============
const char* mqtt_server = "192.168.55.150";
const int   mqtt_port = 1883;
const char* mqtt_user = "usermqtt";      // o ""
const char* mqtt_pass = "Ia$247";     // o ""
const char* mqtt_client_id = "patio_casa";

// Tópico
const char* topic_alarma = "casa/patio/alarma";

// ================= PINES =================
// #define PIR_PIN   D5    // GPIO14 ESP8266
// #define RELAY_PIN D6    // GPIO12 ESP8266
#define PIR_PIN   27   // GPIO27 ESP32
#define RELAY_PIN 26   // GPIO26 ESP32

// ============ PARÁMETROS ============
const unsigned long TIEMPO_ENCENDIDO = 10000; // 10 segundos durante el cual se enciende luz en el relé

bool releActivo = false;
unsigned long tiempoInicio = 0;

// ============ OBJETOS ============
WiFiClient espClient;
PubSubClient client(espClient);

// ============ WIFI ============
void setup_wifi() {
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ============ MQTT RECONNECT ============
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5s...");
      delay(5000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Apagar relé al inicio (muchos relés son activos en LOW)
  digitalWrite(RELAY_PIN, HIGH);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("Sistema iniciado...");
  Serial.println("Esperando estabilización del sensor PIR...");
  delay(40000); // 40 segundos para estabilización del PIR

  Serial.println("Sistema listo. Esperando movimiento.");
}

// ================= LOOP =================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int estadoPIR = digitalRead(PIR_PIN);

  // Si detecta movimiento y el relé no está activo
  if (estadoPIR == HIGH && !releActivo) {
    Serial.println("Movimiento detectado!");
    activarRele();
    publicarMovimiento();
  }

  // Si el relé está activo, verificar tiempo
  if (releActivo) {
    if (millis() - tiempoInicio >= TIEMPO_ENCENDIDO) {
      desactivarRele();
    }
  }
}

// ================= FUNCIONES =================

void activarRele() {
  // Muchos relés son activos en LOW
  digitalWrite(RELAY_PIN, LOW);
  releActivo = true;
  tiempoInicio = millis();
  Serial.println("Relé ACTIVADO - Luz encendida");
}

void desactivarRele() {
  digitalWrite(RELAY_PIN, HIGH);
  releActivo = false;
  Serial.println("Relé DESACTIVADO - Luz apagada");
  Serial.println("Sistema nuevamente en espera...");
}

void publicarMovimiento() {
  if (client.connected()) {
    client.publish(topic_alarma, "MOVIMIENTO");
    Serial.println("MQTT enviado: MOVIMIENTO");
  }
}
