#include <WiFi.h>
#include <PubSubClient.h>

// Configuración de la red WiFi
const char* ssid = "247IASbros1";       // Nombre de la red WiFi
const char* password = "GBxT0K6be99GmznHfUb7"; // Contraseña de la red WiFi

// Configuración del servidor MQTT
const char* mqttServer = "192.168.55.150"; // Cambia por tu servidor MQTT
const int mqttPort = 1883;                   // Puerto del servidor MQTT
const char* mqttUser = "usermqtt";                   // Usuario (si aplica)
const char* mqttPassword = "Ia$247";               // Contraseña (si aplica)

// Definición del pin relé que activa porton
const int porton = 26;  // Conexión a la entrada del modulo relé

// Variables de WiFi y MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Configuración inicial
void setup() {
  Serial.begin(115200); // Inicializa comunicación serial
  pinMode(porton, OUTPUT); // Inicializar el pin modulo relé del portón y el led piloto
  digitalWrite(porton, LOW);  // Apagar inicialmente el relé del portón y el led
  connectToWiFi();      // Conexión inicial al WiFi
  mqttClient.setServer(mqttServer, mqttPort); // Configura el servidor MQTT
  mqttClient.setCallback(mqttCallback);       // Asigna la función callback
}

// Bucle principal
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Intentando reconectar...");
    connectToWiFi();
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  mqttClient.loop(); // Mantiene la conexión MQTT viva

  // Opcional: Publicar un mensaje periódicamente
  /*static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 5000) {
    lastPublish = millis();
    mqttClient.publish("test/topic", "Hello from ESP32!");
    Serial.println("Mensaje publicado en el topic test/topic.");
  }*/
}

// Función para conectar al WiFi
void connectToWiFi() {
  delay(1 * 30 * 1000); // Espera 30 seg hasta que este disponible el WiFi 247IASbros1
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado con Dirección IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("No se pudo conectar al WiFi ):");
  }
}

// Función para reconectar al servidor MQTT
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Conectando al servidor MQTT...");
    if (mqttClient.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Conexión MQTT exitosa!!!");
      mqttClient.subscribe("casa/porton/pulsar"); // Suscribirse al topico de Barrio Norte
      Serial.println("Suscrito a topico para control del portón!");
    } else {
      Serial.print("Falló la conexión MQTT. Código de error: ");
      Serial.println(mqttClient.state());
      delay(2000); // Esperar 2 seg antes de intentar nuevamente
    }
  }
}

// Función callback para manejar mensajes MQTT entrantes
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  Serial.print("Mensaje recibido en topic: ");
  Serial.println(topic);
  Serial.print("Mensaje: ");
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Se compara al tópico suscripto y al payload para activar el relé SI es correcto //
  if (String(topic) == "casa/porton/pulsar" && messageTemp == "PULSE") {  //Barrio NORTE
    digitalWrite(porton, HIGH);  // Activar el relé
    delay(1000);  // Mantener el estado HIGH durante 1 segundo
    digitalWrite(porton, LOW);   // Desactivar el relay
    Serial.println("Pulso de 1 segundo enviado al relay");
  }
}
