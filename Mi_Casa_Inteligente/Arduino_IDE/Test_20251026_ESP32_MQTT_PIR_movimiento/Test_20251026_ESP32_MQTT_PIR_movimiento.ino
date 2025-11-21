#include <WiFi.h>
#include <PubSubClient.h>

// --- Configuración WiFi ---
const char* ssid = "247IASbrosHA";
const char* password = "GBxT0K6be99GmznHfUb247$!";

// --- Configuración MQTT ---
const char* mqtt_server = "192.168.55.150";
const int mqtt_port = 1883;
const char* mqtt_user = "usermqtt";
const char* mqtt_pass = "Ia$247";
const char* topic = "datacenter/porton/movimiento";

// --- Configuración sensor PIR ---
const int pirPin = 27; // GPIO 27 es seguro y recomendado para entrada digital con sensor PIR en ESP32[web:23][web:14]

// --- Variables ---
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32PIRClient", mqtt_user, mqtt_pass)) {
      Serial.println("Conectado a MQTT broker.");
    } else {
      Serial.print("Fallo MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  pinMode(pirPin, INPUT);
  client.setServer(mqtt_server, mqtt_port);
  reconnect_mqtt();
  Serial.println("Esperando calentamiento del PIR...");
  delay(20000); // Tiempo recomendado para calentamiento del HC-SR501[web:29]
  Serial.println("Sensor PIR listo.");
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  int estadoPIR = digitalRead(pirPin);

  if (estadoPIR == HIGH) {
    client.publish(topic, "ATENCION");
    Serial.println("¡Movimiento detectado! Notificación enviada.");
    delay(2000); // Anti-rebote y evitar múltiples envíos por el mismo movimiento
  }
}
