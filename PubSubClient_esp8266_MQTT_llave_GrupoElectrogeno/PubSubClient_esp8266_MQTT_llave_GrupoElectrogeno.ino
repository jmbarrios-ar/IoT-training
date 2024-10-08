#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración de WiFi Barrio NORTE
const char* ssid = "247IASbros2.4";
const char* password = "T3reKByo2023$";
// Configuración del servidor MQTT
const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";

// Configuración de WiFi Datacenter
/*const char* ssid = "datacenter";
const char* password = "NOv22$1nicI0";
//const char *mqtt_server = "45.186.124.70";
const char *mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char *mqtt_user = "adminmqtt";
const char *mqtt_pass = "Ia$247";*/

// Pin del pulsador
const int led2rojo = 5;  // GPIO5 - D1
const int relePin = 4;  // GPIO4 - D2
//bool lastReleState = LOW; // CPNEXIÓN A PULL DOWN
int lastReleState = LOW; // de inicio el relé esta abierto
int val = 0;    //
// Cliente WiFi y cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//************* DECLARAR FUNCIONES ***************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(9600);
  pinMode(relePin, INPUT); 
  pinMode(led2rojo, OUTPUT);
  digitalWrite(led2rojo, LOW);  // Apagar el LED inicialmente
 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Lectura del estado del Relé del G.E.
  //bool currentReleState = digitalRead(relePin);
  val = digitalRead(relePin);
  if (val == HIGH)  {  //si está activado
    digitalWrite(led2rojo, HIGH);  //LED ON
    if (lastReleState == LOW)  {  //si previamente estaba apagado
      Serial.println("Grupo Electrógeno encendido: Publicando ON");
      client.publish("casa/pulsador/estado", "ON"); // Barrio NORTE
      //client.publish("datacenter/grupo/estado", "ON"); // Datacenter
      lastReleState = HIGH;
    }
  }
  else  {  //si esta desactivado
    digitalWrite(led2rojo, LOW); // LED OFF
    if (lastReleState == HIGH)   {  //si previamente estaba encendido
      Serial.println("Grupo Electrógeno apagado. Publicando OFF");
      client.publish("casa/pulsador/estado", "OFF"); // Barrio NORTE
      //client.publish("datacenter/grupo/estado", "OFF"); // Datacenter
      lastReleState = LOW;
    }
  }
  /*
  if (currentReleState != lastReleState) {  // Verifica si el estado del relé ha cambiado
    lastReleState = currentReleState;
    if (currentReleState == HIGH) { // El botón se presionó
      client.publish("casa/pulsador/estado", "ON"); // Barrio NORTE
      //client.publish("datacenter/grupo/estado", "ON"); // Datacenter
      Serial.println("Botón presionado: Publicando ON");
      digitalWrite(led2rojo, HIGH);
    } else { // El botón se soltó
      client.publish("casa/pulsador/estado", "OFF"); // Barrio NORTE
      //client.publish("datacenter/grupo/estado", "OFF"); // Datacenter
      Serial.println("Botón soltado: Publicando OFF");
      digitalWrite(led2rojo, LOW);
    }
  } */
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

// ********* RECEPCIÓN DE MENSAJES EN LOS T´PICOS SUSCRIPTOS **********
void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);
  Serial.println();

//***** Si el mensaje recibido es "ON" o "OFF", puedes hacer algo aquí ******
  /*if (String(topic) == "casa/pulsador/estado") {
    if (messageTemp == "ON") {
      // Acción cuando el mensaje es ON
      Serial.println("El pulsador está en estado ON");
      digitalWrite(led2rojo, HIGH);
    } else if (messageTemp == "OFF") {
      // Acción cuando el mensaje es OFF
      Serial.println("El pulsador está en estado OFF");
      digitalWrite(led2rojo, LOW);
    }
  }*/
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
      
      // Suscribirse al tópico
      client.subscribe("casa/pulsador/estado");    // Barrio NORTE
      //client.subscribe("datacenter/grupo/estado");  // Datacenter
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}
