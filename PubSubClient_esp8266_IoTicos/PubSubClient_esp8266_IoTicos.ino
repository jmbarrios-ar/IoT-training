#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "datacenter";
const char* password = "NOv22$1nicI0";

//const char *mqtt_server = "45.186.124.70";
const char *mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char *mqtt_user = "adminmqtt";
const char *mqtt_pass = "Ia$247";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];

const int lednaranja = D6; //pin del led naranja

int valor_prueba = 0;

//**********************************************
//************* DECLARAR FUNCIONES ***********
//**********************************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  pinMode(lednaranja, OUTPUT);
  Serial.begin(9600);
  randomSeed(micros());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /*long now = millis();
  if (now - lastMsg > 5000){
    lastMsg = now;
    valor_prueba++;

    snprintf (msg, 50, "HOLA MUNDO", valor_prueba);
    Serial.print("Mensaje publicado: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
*/
}


//******* CONEXION WIFI ***********
void setup_wifi() {
  delay(10);
  // Comenzamos conectándonos a una red WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  String incoming = "";
  Serial.print("Mensaje recibido en el tópico: ");
  Serial.print(topic);
  Serial.println("");
  // Aquí puedes agregar código para procesar el mensaje recibido
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje -> " + incoming);

  if ( incoming == "on") {
    digitalWrite(lednaranja, HIGH);
    } if ( incoming == "off") {
      digitalWrite(lednaranja, LOW);
    }  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al servidor MQTT...");
    // Creamos un cliente ID
    String clientId = "esp8266_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      // Nos suscribimos
      client.subscribe("casa/led/estado"); // Suscripción al tópico
      client.subscribe("casa/pulsador/estado"); // Suscripción al tópico
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
