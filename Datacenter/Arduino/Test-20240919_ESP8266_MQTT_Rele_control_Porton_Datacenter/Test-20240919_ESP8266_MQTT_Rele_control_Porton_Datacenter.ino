// ***********  CONTROL PORTÓN EN DATACENTER **********************
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ******** DATACENTER ********
const char* ssid = "datacenter";
const char* password = "NOv22$1nicI0";
//const char *mqtt_server = "45.186.124.70";
const char* mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char* mqtt_user = "adminmqtt";
const char* mqtt_pass = "Ia$247";

// Definición del pin relé que activa porton
const int porton = 5;  // GPIO5 - D1   Conexión a la entrada del modulo relé
//const int pinled = 4;  // GPIO4 - D2   Led piloto del PULSO

WiFiClient espClient;
PubSubClient client(espClient);

//************* DECLARAR FUNCIONES ***************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(9600);
  // Inicializar el pin modulo relé del portón y el led piloto
  pinMode(porton, OUTPUT);
  //pinMode(pinled, OUTPUT);
  // Apagar inicialmente el relé del portón y el led
  digitalWrite(porton, LOW);
  //digitalWrite(pinled, LOW);

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
  if (String(topic) == "datacenter/porton/control" && messageTemp == "PULSE") {  //Datacenter
    digitalWrite(porton, HIGH);                                                  // Activar el relé
    //digitalWrite(pinled, HIGH);
    delay(1000);                // Mantener el estado HIGH durante 1 segundo
    digitalWrite(porton, LOW);  // Desactivar el relay
    //digitalWrite(pinled, LOW);
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
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!");

      // Suscribirse al tópico del relé del porton
      client.subscribe("datacenter/porton/control");  // Datacenter
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}