//***** ALARMA MONITOREO ESTADO DE GRUPO ELECTRÓGENO y MONITOREO DE TEMPERATURA  *****//
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT11.h>

#define DHTPIN 2 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT (DHT11 o DHT22)

void(* Resetea) (void) = 0;//Funcíon Reset por soft para el arduino (como si apretaramos el botón reset)

// ********** ETHERNET config. Barrio NORTE *********************************
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//Dirección MAC de nuestro módulo ethernet
IPAddress ip(192, 168, 24, 151); // IP en Barrio NORTE
IPAddress gateway(192, 168, 24, 1); //Pasarela en Barrio NORTE
IPAddress subnet(255, 255, 255, 0);  //Mascara en Barrio NORTE
IPAddress dnServer(192, 168, 100, 1);  //DNS en Barrio NORTE

// ********Configuración del servidor MQTT en Barrio NORTE*************
const char *mqtt_server = "192.168.24.150";
const int mqtt_port = 1883;
const char *mqtt_user = "usermqtt";
const char *mqtt_pass = "Ia$247";

// ************** Config relé detector de corte de energía **********************
int ledROJO = 4;  // Led indicador con corte energía
int ledVERDE = 5;  // Led indicador sin corte energía
int relePin = 3;  
int lastReleState = HIGH; // de inicio el relé esta abierto
int val = 1;    //

// Config leds Temperatura
//int ledAZUL = 5;  // Led indicador de temperatura normal
//int ledNARANJA = 6;  // Led indicador de umbral temperatura 

// *********Config sensor de temperatura ***************
int pin=2;
DHT11 dht11(pin);  // Asignacion del pin del DHT11, el RTC tiene SDA en el A4 (SDA del arduino) y el SCL en el A5 (SCL del arduino)
//int umbral = 28;  //Temperatura que activa alarma

// Tópicos MQTT Barrio NORTE
const char* topicTemp = "casa/dht11/temperatura";     // Tópico para la temperatura
const char* topicHum = "casa/dht11/humedad";          // Tópico para la humedad

// Crear cliente Ethernet y MQTT
EthernetClient cliente;
PubSubClient client(cliente);

//************* DECLARAR FUNCIONES ***************************
//void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void relesinluz();
void tempyhumd();

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
  Serial.println("Conectando...");
  
  client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  relesinluz();
  tempyhumd();
}

// ********* RECEPCIÓN DE MENSAJES EN LOS T´PICOS SUSCRIPTOS **********
/*void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);
  Serial.println();
} */

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
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
}

//************ DETECTAR CORTE DE ENERGÍA **************
void relesinluz(){
  // Lectura del estado del Relé del G.E.
  val = digitalRead(relePin);
  if (val == LOW)  {  //si está activado
    digitalWrite(ledROJO, HIGH);  //LED ROJO ENCENDIDO
    digitalWrite(ledVERDE, LOW);  //LED VERDE APAGADO
    if (lastReleState == HIGH)  {  //si previamente estaba apagado
      Serial.println("Corte de Energía Eléctrica detectado: Publicando ON");
      client.publish("casa/pulsador/estado", "ON"); // Barrio NORTE
      lastReleState = LOW;
    }
  }
  else  {  //si esta desactivado
    digitalWrite(ledROJO, LOW); // LED OFF
    digitalWrite(ledVERDE, HIGH); // LED ON
    if (lastReleState == LOW)   {  //si previamente estaba encendido
      Serial.println("La Energía Eléctrica se ha restablecido: Publicando OFF");
      client.publish("casa/pulsador/estado", "OFF"); // Barrio NORTE
      lastReleState = HIGH;
    }
  }
}

//************ LEER TEMPERATURA Y HUMEDAD DESDE SENSOR DHT11 **************
void tempyhumd() {
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
  Serial.print("Temperatura en Casa: ");
  Serial.println(tempStr);

  // Publicar humedad
  client.publish(topicHum, humStr.c_str());
  Serial.print("Humedad en Casa: ");
  Serial.println(humStr);

  // Esperar 10 segundos antes de la próxima lectura
  delay(10000);
}