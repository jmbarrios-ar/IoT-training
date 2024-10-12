#include <WiFi.h>  //ESP32
#include <PubSubClient.h>

// ***** Barrio NORTE *******
// Configuración de WiFi
const char* ssid = "247IASbros1";
const char* password = "GBxT0K6be99GmznHfUb7";
// Configuración del servidor MQTT
const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";

// Definición del pin relé que activa porton
#define RELE 26
//#define pinled 27
#define RELE_ON HIGH
#define RELE_OFF LOW
//#define LED_ON HIGH
//#define LED_OFF LOW

WiFiClient espClient;
PubSubClient client(espClient);

//************* DECLARAR FUNCIONES ***************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(115200);
  // Inicializar el pin modulo relé del portón y el led piloto
  pinMode(RELE, OUTPUT);
  //pinMode(pinled, OUTPUT);
  // Apagar inicialmente el relé del portón y el led
  digitalWrite(RELE, RELE_OFF);
  //digitalWrite(pinled, LED_OFF);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

//******* CONEXION WIFI ***********
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// ********* RECEPCIÓN DE MENSAJES EN LOS TÓPICOS SUSCRIPTOS **********
void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);
  
//***** COMPARAR EL TÓPICO y ACTIVAR EL RELÉ SI EL PAYLOAD ES CORRECTO ******
  if (String(topic) == "casa/porton/pulsar" && messageTemp == "PULSE") {  //Barrio NORTE
    digitalWrite(RELE, RELE_ON);  // Activar el relé
    //digitalWrite(pinled, LED_ON);
    delay(1000);  // Mantener el estado HIGH durante 1 segundo
    digitalWrite(RELE, RELE_OFF);   // Desactivar el relay
    //digitalWrite(pinled, LED_OFF);
    Serial.println("Pulso de 1 segundo enviado al relay");
  }
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
      
      // Suscribirse al tópico del relé del porton
      client.subscribe("casa/porton/pulsar");  //Barrios NORTE
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}
