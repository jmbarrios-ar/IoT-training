//***** ALARMA MONITOREO DE TEMPERATURA *****//
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT11.h>

#define DHTPIN 2 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT (DHT11 o DHT22)

void(* Resetea) (void) = 0;//Funcíon Reset por soft para el arduino (como si apretaramos el botón reset)

// ETHERNET config.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//Dirección MAC de nuestro módulo ethernet
IPAddress ip(192, 168, 24, 151); // IP en Barrio NORTE
//IPAddress ip(170, 10, 10, 38); // IP en Datacenter
IPAddress gateway(192, 168, 24, 1); //Pasarela en Barrio NORTE
//IPAddress gateway(170, 10, 10, 1); //Pasarela en Datacenter
IPAddress subnet(255, 255, 255, 0);  //Mascara en Barrio NORTE
//IPAddress subnet(255, 255, 255, 0);  //Mascara en Datacenter
IPAddress dnServer(192, 168, 100, 1);  //DNS en Barrio NORTE
//IPAddress dnServer(8, 8, 8, 8);  //DNS en Datacenter

// ********Configuración del servidor MQTT en Barrio NORTE*************
const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";
// ******** Configuración del servidor MQTT en Datacenter ***************
//const char *mqtt_server = "45.186.124.70";
/*const char *mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char *mqtt_user = "adminmqtt";
const char *mqtt_pass = "Ia$247";*/

// Config leds
//int ledAZUL = 5;  // Led indicador de temperatura normal
//int ledNARANJA = 6;  // Led indicador de umbral temperatura 

// *********Config sensor de temperatura ***************
int pin=2;
DHT11 dht11(pin);  // Asignacion del pin del DHT11, el RTC tiene SDA en el A4 (SDA del arduino) y el SCL en el A5 (SCL del arduino)
//int umbral = 28;  //Temperatura que activa alarma

// Tópicos MQTT Barrio NORTE
const char* topicTemp = "casa/dht11/temperatura";     // Tópico para la temperatura
const char* topicHum = "casa/dht11/humedad";          // Tópico para la humedad
// Tópicos MQTT Datacenter
//const char* topicTemp = "datacenter/dht11/temperatura";     // Tópico para la temperatura
//const char* topicHum = "datacenter/dht11/humedad";          // Tópico para la humedad

// Crear cliente Ethernet y MQTT
EthernetClient cliente;
PubSubClient client(cliente);

//************* DECLARAR FUNCIONES ***************************
//void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(9600);
  //dht.begin();
  //pinMode(ledAZUL, OUTPUT); // Led indicador de temperatura normal
  //pinMode(ledNARANJA, OUTPUT); // Led indicador de umbral temperatura
  // **** Conexión a Ethernet ****  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Falló para configurar Ethernet usando DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Conectando...");
  
  client.setServer(mqtt_server, mqtt_port);  // Configurar el servidor MQTT
  //client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {   // Verificar si el cliente MQTT está conectado, si no, reconectar
    reconnect();
  }
  client.loop();   // Mantener la conexión activa
  
  // Leer temperatura y humedad del DHT11
  float h = dht11.readHumidity();  // Humedad en porcentaje %
  float t = dht11.readTemperature(); // Temperatura en grados Celsius

  // Verificar si las lecturas son válidas
  if (isnan(h) || isnan(t)) {
    Serial.println("Error al leer del sensor DHT11");
    return;
  }

  // Publicar los valores en los tópicos MQTT
  String tempStr = String(t);
  String humStr = String(h);

  // Publicar temperatura
  client.publish(topicTemp, tempStr.c_str());
  Serial.print("Temperatura Área Servidores: ");
  Serial.println(tempStr);

  // Publicar humedad
  client.publish(topicHum, humStr.c_str());
  Serial.print("Humedad Área Servidores: ");
  Serial.println(humStr);

  // Esperar 10 segundos antes de la próxima lectura
  delay(10000);

}

//******** RECONECTARSE AL SERVER MQTT SI SE PIERDE CONEXIÓN *******
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Creamos un cliente ID
    String clientId = "esp8266_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}