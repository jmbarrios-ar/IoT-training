//***** ALARMA MONITOREO ESTADO DE GRUPO ELECTRÓGENO *****//
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

void(* Resetea) (void) = 0;//Función Reset por soft para el arduino (como si apretaramos el botón reset)

// ETHERNET config.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//Dirección MAC de nuestro módulo ethernet
//char server[] = "123.123.123.204"; //Cambiar por la url del servidor a usar   
//IPAddress ip(192, 168, 24, 151); // IP en Barrio NORTE
IPAddress ip(170, 10, 10, 38); // IP en Datacenter
//IPAddress gateway(192, 168, 24, 1); //Pasarela en Barrio NORTE
IPAddress gateway(170, 10, 10, 1); //Pasarela en Datacenter
//IPAddress subnet(255, 255, 255, 0);  //Mascara en Barrio NORTE
IPAddress subnet(255, 255, 255, 0);  //Mascara en Datacenter
//IPAddress dnServer(192, 168, 100, 1);  //DNS en Barrio NORTE
IPAddress dnServer(8, 8, 8, 8);  //DNS en Datacenter

// ********Configuración del servidor MQTT en Barrio NORTE*************
/*const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";*/
// ******** Configuración del servidor MQTT en Datacenter ***************
//const char *mqtt_server = "45.186.124.70";
const char *mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char *mqtt_user = "adminmqtt";
const char *mqtt_pass = "Ia$247";

// Config relé detector de corte de energía
int ledROJO = 4;  // Led indicador con corte energía Grupo Electrógeno ENCENDIDO
int ledVERDE = 5;  // Led indicador sin corte energía Grupo Electrógeno APAGADO
int relePin = 3;  
int lastReleState = LOW; // de inicio el relé esta abierto
int val = 0;    //

EthernetClient cliente;
PubSubClient client(cliente);

//************* DECLARAR FUNCIONES ***************************
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(9600);
  pinMode(relePin, INPUT); 
  pinMode(ledROJO, OUTPUT); // Led indicador CON corte energía
  pinMode(ledVERDE, OUTPUT); // Led indicador SIN corte energía
  digitalWrite(ledROJO, LOW);  // Apagar el LED inicialmente
  digitalWrite(ledVERDE, HIGH);  // Encender el LED inicialmente

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Falló para configurar Ethernet usando DHCP");
    // Intento congifurar usando la dirección IP en lugar de DHCP:
    Ethernet.begin(mac, ip);
  }
  // Dejo el Ethernet Shield un segundo para inicializar:
  delay(1000);
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("connecting...");
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Lectura del estado del Relé del G.E.
  val = digitalRead(relePin);
  if (val == HIGH)  {  //si está activado
    digitalWrite(ledROJO, HIGH);  //LED ROJO ENCENDIDO
    digitalWrite(ledVERDE, LOW);  //LED VERDE APAGADO
    if (lastReleState == LOW)  {  //si previamente estaba apagado
      Serial.println("Grupo Electrógeno encendido: Publicando ON");
      //client.publish("casa/pulsador/estado", "ON"); // Barrio NORTE
      client.publish("datacenter/grupo/estado", "ON"); // Datacenter
      lastReleState = HIGH;
    }
  }
  else  {  //si esta desactivado
    digitalWrite(ledROJO, LOW); // LED OFF
    digitalWrite(ledVERDE, HIGH); // LED ON
    if (lastReleState == HIGH)   {  //si previamente estaba encendido
      Serial.println("Grupo Electrógeno apagado. Publicando OFF");
      //client.publish("casa/pulsador/estado", "OFF"); // Barrio NORTE
      client.publish("datacenter/grupo/estado", "OFF"); // Datacenter
      lastReleState = LOW;
    }
  }

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
      //client.subscribe("casa/pulsador/estado");    // Barrio NORTE
      client.subscribe("datacenter/grupo/estado");  // Datacenter
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}