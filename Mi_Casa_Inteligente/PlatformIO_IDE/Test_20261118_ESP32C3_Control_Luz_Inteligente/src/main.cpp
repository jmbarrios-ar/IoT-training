// Para que el dispositivo hable con Home Assistant sin usar ESPHome, el estándar es MQTT. 
// Este código detecta los cambios de estado del interruptor físico 
// (para que no importe si quedó hacia arriba o hacia abajo) 
// y escucha los comandos desde el dashboard.


#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Credenciales WiFi y MQTT
const char* ssid = "TU_WIFI";
const char* password = "TU_PASSWORD";
const char* mqtt_server = "IP_DE_TU_MINI_PC_BMAX"; // IP del broker MQTT en Home Assistant
const char* mqtt_user = "usuario_mqtt";
const char* mqtt_pass = "clave_mqtt";

// Tópicos MQTT
const char* state_topic = "dormitorio/luz/estado";
const char* command_topic = "dormitorio/luz/comando";

// Configuración de Pines para ESP32-C3 Super Mini
const int RELAY_PIN = 8;  // Pin conectado al control del Relé
const int SWITCH_PIN = 9; // Pin conectado al interruptor de pared (S2)

WiFiClient espClient;
PubSubClient client(espClient);

// Variables de estado
bool lightState = false;
int lastSwitchState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup_wifi() {
  delay(10);
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Comando recibido desde Home Assistant
  if (String(topic) == command_topic) {
    if (message == "ON") {
      lightState = true;
    } else if (message == "OFF") {
      lightState = false;
    }
    updateRelay();
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32C3_Dormitorio", mqtt_user, mqtt_pass)) {
      Serial.println("Conectado al broker MQTT.");
      client.subscribe(command_topic);
      client.publish(state_topic, lightState ? "ON" : "OFF");
    } else {
      Serial.print("Fallo MQTT, estado: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void updateRelay() {
  digitalWrite(RELAY_PIN, lightState ? HIGH : LOW);
  client.publish(state_topic, lightState ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Resistencia interna activada
  
  digitalWrite(RELAY_PIN, LOW);
  lastSwitchState = digitalRead(SWITCH_PIN);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Lectura del interruptor físico con Debounce
  int reading = digitalRead(SWITCH_PIN);
  if (reading != lastSwitchState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Si el estado del interruptor físico cambia, invertimos el estado de la luz
    if (reading != lastSwitchState) {
      lightState = !lightState;
      updateRelay();
      lastSwitchState = reading;
    }
  }
  lastSwitchState = reading;
}
