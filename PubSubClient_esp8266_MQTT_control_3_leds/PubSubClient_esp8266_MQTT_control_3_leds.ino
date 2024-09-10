#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuración de WiFi
const char* ssid = "247IASbros2.4";
const char* password = "T3reKByo2023$";

// Configuración del servidor MQTT
const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";

// Definición de los pines de los LEDs
const int led1verde = 5;  // GPIO5 - D1
const int led2rojo = 4;  // GPIO4 - D2
const int led3azul = 0;  // GPIO0 - D3

WiFiClient espClient;
PubSubClient client(espClient);

//************* DECLARAR FUNCIONES ***************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(9600);
  // Inicializar los pines de los LEDs
  pinMode(led1verde, OUTPUT);
  pinMode(led2rojo, OUTPUT);
  pinMode(led3azul, OUTPUT);
  // Apagar los LEDs inicialmente
  digitalWrite(led1verde, LOW);
  digitalWrite(led2rojo, LOW);
  digitalWrite(led3azul, LOW);
  
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
  
//***** COMPARAR EL TÓPICO y ENCENDER/APAGAR LOS LEDs ******
  if (String(topic) == "casa/luz/1/comando") {
    if (messageTemp == "ON") {
      digitalWrite(led1verde, HIGH);
    } else if (messageTemp == "OFF") {
      digitalWrite(led1verde, LOW);
    }
  }
  
  if (String(topic) == "casa/luz/2/comando") {
    if (messageTemp == "ON") {
      digitalWrite(led2rojo, HIGH);
    } else if (messageTemp == "OFF") {
      digitalWrite(led2rojo, LOW);
    }
  }
  
  if (String(topic) == "casa/luz/3/comando") {
    if (messageTemp == "ON") {
      digitalWrite(led3azul, HIGH);
    } else if (messageTemp == "OFF") {
      digitalWrite(led3azul, LOW);
    }
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
      
      // Suscribirse a los tópicos de los LEDs
      client.subscribe("casa/luz/1/comando");
      client.subscribe("casa/luz/2/comando");
      client.subscribe("casa/luz/3/comando");
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}
