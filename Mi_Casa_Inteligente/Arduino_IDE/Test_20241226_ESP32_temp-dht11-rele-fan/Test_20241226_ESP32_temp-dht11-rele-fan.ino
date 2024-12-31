#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT11.h>

#define DHTPIN 36 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT (DHT11 o DHT22)

// Configuración de la red WiFi
const char* ssid = "247IASbros1";       // Nombre de la red WiFi
const char* password = "GBxT0K6be99GmznHfUb7"; // Contraseña de la red WiFi

// Configuración del servidor MQTT
const char* mqttServer = "192.168.24.150"; // Cambia por tu servidor MQTT
const int mqttPort = 1883;                   // Puerto del servidor MQTT
const char* mqttUser = "usermqtt";                   // Usuario (si aplica)
const char* mqttPassword = "Ia$247";               // Contraseña (si aplica)

// Definición del pin relé que activa el ventilador
const int ventilador = 32;  // Conexión al pin de salida para el modulo relé
const int sth11=36;  // Conexión al pin de entrada del sensor DHT11
DHT11 dht11(sth11);  // Asignacion del pin del DHT11, el RTC tiene SDA en el A4 (SDA del arduino) y el SCL en el A5 (SCL del arduino)
const int umbral = 24;  //Temperatura que activa el ventilador

// Tópicos MQTT Barrio NORTE
const char* topicTemp = "casa/porton/temperatura";     // Tópico para la temperatura del control porton
const char* topicHum = "casa/porton/humedad";          // Tópico para la humedad

// Variables de WiFi y MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Configuración inicial
void setup() {
  Serial.begin(115200); // Inicializa comunicación serial
  pinMode(ventilador, OUTPUT); // Inicializar el pin modulo relé del ventilador
  digitalWrite(ventilador, LOW);  // Apagar inicialmente el relé del ventilador
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
  tempporton();
}

// Función para conectar al WiFi
void connectToWiFi() {
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
      //mqttClient.subscribe("casa/porton/pulsar"); // Suscribirse al topico de Barrio Norte
      //Serial.println("Suscrito a topico para control del portón!");
    } else {
      Serial.print("Falló la conexión MQTT. Código de error: ");
      Serial.println(mqttClient.state());
      delay(2000); // Esperar 2 seg antes de intentar nuevamente
    }
  }
}

//************ LEER TEMPERATURA Y HUMEDAD DESDE SENSOR DHT11 EN MOTOR PORTON**************
void tempporton() {
  // Leer temperatura del DHT11
  float h = dht11.readHumidity();  // Humedad en porcentaje %
  float t = dht11.readTemperature(); // Temperatura en grados Celsius

  // Verificar si las lecturas son válidas
  if (isnan(h) || isnan(t)) {
      Serial.println("Error al leer del sensor DHT11");
    return;
  }
  // Convertir los valores float a int
  int hi;
  int ti;
  hi = (int) h;
  ti = (int) t;
  // Publicar los valores en los tópicos MQTT
  String tempStr = String(ti);
  String humStr = String(hi);

  // Publicar temperatura
  client.publish(topicTemp, tempStr.c_str());
  Serial.print("Temperatura en Control Portón: ");
  Serial.println(tempStr);
  // Publicar humedad
  /*client.publish(topicHum, humStr.c_str());
  Serial.print("Humedad en Casa: ");
  Serial.println(humStr);*/

  if (t >= umbral) 
  {
    digitalWrite(ventilador,HIGH);
  }   
  else 
  {
    digitalWrite(ventilador,LOW); 
  }

  // Esperar 6 segundos antes de la próxima lectura
  delay(6000);
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
