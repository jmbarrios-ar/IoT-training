#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración de la red WiFi
//const char* ssid = "datacenter";
//const char* password = "NOv22$1nicI0";
const char *ssid = "247IASbros2.4";
const char *password = "T3reKByo2023$";

// Configuración del servidor MQTT
//const char* mqtt_server = "172.16.16.27";
//const int mqtt_port = 1883;
//const char* user = "adminmqtt"; // Reemplaza con tu usuario
//const char* pass = "Ia$247"; // Reemplaza con tu contraseña
const char *mqtt_server = "45.186.124.70";
const int mqtt_port = 1883;
const char *user = "adminmqtt"; // Reemplaza con tu usuario
const char *pass = "Ia$247"; // Reemplaza con tu contraseña
const char *topic = "casa/led/estado"; // Reemplaza con el nombre del tópico

// Cliente WiFi y cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Pin del LED
const int ledPin = D6; 

// Función para reconectar al servidor MQTT si la conexión se pierde
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al servidor MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
      client.subscribe("casa/led/estado"); // Suscripción al tópico
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");
      delay(5000);
    }
  }
}

// Función para manejar los mensajes MQTT recibidos
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();

  // Si el mensaje recibido es "ON" o "OFF", enciende o apaga el LED
  if (String((char*)message) == "ON") {
    digitalWrite(ledPin, LOW); // Enciende el LED (en ESP8266, LOW enciende el LED integrado)
    Serial.println("LED encendido");
  } else if (String((char*)message) == "OFF") {
    digitalWrite(ledPin, HIGH); // Apaga el LED
    Serial.println("LED apagado");
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Asegura que el LED comience apagado

  Serial.begin(9600);

  // Conexión a la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a la red WiFi");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Puedes agregar aquí código adicional si deseas publicar mensajes
}
