#include <WiFi.h>
#include <PubSubClient.h>

// Configuración de la red WiFi
const char* ssid = "247IASbrosHA";       // Nombre de la red WiFi
const char* password = "GBxT0K6be99GmznHfUb247$!"; // Contraseña de la red WiFi

// ======= CONFIGURACIÓN DE MQTT =======
const char* mqtt_server = "192.168.55.150";
const int mqtt_port = 1883;
const char* topic = "casa/ventilador/dormitorio28";
const char* mqttUser = "usermqtt";                   // Usuario (si aplica)
const char* mqttPassword = "Ia$247";               // Contraseña (si aplica)

// ======= CONFIGURACIÓN DE RELE Y TIEMPOS =======
const int relayPin = 25;  // Pin seguro recomendado para relé en ESP32
//const unsigned long tiempoEncendido = 3600000UL; // 1 hora = 1 * 60 * 60 * 1000 ms
//const unsigned long tiempoApagado = 1800000UL;   // 30 minutos = 30 * 60 * 1000 ms
const unsigned long tiempoEncendido = 60000UL; // 1 hora = 1 * 60 * 60 * 1000 ms
const unsigned long tiempoApagado = 30000UL;   // 30 minutos = 30 * 60 * 1000 ms

// ======= OBJETOS DE CONEXIÓN =======
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long previousMillis = 0;
bool estadoVentilador = false;

// ======= FUNCIONES =======
void setup_wifi() {
  delay(10);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Conectando al servidor MQTT...");
    if (client.connect("ESP32_Ventilador", mqttUser, mqttPassword)) {
      Serial.println("Conectado a MQTT");
    } else {
      Serial.print("Fallo MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void publicarEstado(const char* estado) {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.publish(topic, estado);
  Serial.println(String("Mensaje enviado: ") + estado);
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  reconnect_mqtt();

  // Inicio del ciclo
  publicarEstado("Activado");
  digitalWrite(relayPin, HIGH);
  previousMillis = millis();
  estadoVentilador = true;
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if (estadoVentilador && (currentMillis - previousMillis >= tiempoEncendido)) {
    // Cambiar a apagado
    estadoVentilador = false;
    digitalWrite(relayPin, LOW);
    publicarEstado("Desactivado");
    previousMillis = currentMillis;
  } 
  else if (!estadoVentilador && (currentMillis - previousMillis >= tiempoApagado)) {
    // Cambiar a encendido
    estadoVentilador = true;
    digitalWrite(relayPin, HIGH);
    publicarEstado("Activado");
    previousMillis = currentMillis;
  }
}