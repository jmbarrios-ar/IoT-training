//***** ALARMA CORTE DE ENERGÍA ELECTRICA y MONITOREO DE TEMPERATURA - CÓMPUTOS CASA GOBIERNO *****//
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT11.h>    // Agregar libreria DHT11 by Dhruba Saha

#define DHTPIN 2 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT (DHT11 o DHT22)

void(* Resetea) (void) = 0;//Funcíon Reset por soft para el arduino (como si apretaramos el botón reset)

// ********** ETHERNET config. CASA GOBIERNO *********************************
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//Dirección MAC de nuestro módulo ethernet
IPAddress ip(170, 10, 10, 38); // IP en Cómputos
IPAddress gateway(170, 10, 10, 1); //Pasarela en Cómputos
IPAddress subnet(255, 255, 255, 0);  //Mascara en Cómputos
IPAddress dnServer(8, 8, 8, 8);  //DNS en Cómputos

// ********Configuración del servidor MQTT para CASA GOBIERNO *************
const char *mqtt_server = "172.16.16.27";
const int mqtt_port = 1883;
const char *mqtt_user = "adminmqtt";
const char *mqtt_pass = "Ia$247";

// ************** Config relé detector de corte de energía **********************
int ledROJO = 4;  // Led indicador corte energía en Cómputos
int ledVERDE = 5;  // Led indicador sin corte energía en Cómputos
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
const char* topicTemp = "casagob/dht11/temperatura";     // Tópico para la temperatura
const char* topicHum = "casagob/dht11/humedad";          // Tópico para la humedad

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
      client.subscribe("casagob/rele/estado");    // Cómputos
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
  // Lectura del estado del Relé detector Corte de Energía en Cómputos
  val = digitalRead(relePin);
  if (val == LOW)  {  //si está activado
    digitalWrite(ledROJO, HIGH);  //LED ROJO ENCENDIDO
    digitalWrite(ledVERDE, LOW);  //LED VERDE APAGADO
    if (lastReleState == HIGH)  {  //si previamente estaba apagado
      Serial.println("Corte de Energía Eléctrica detectado: Publicando OFF");
      client.publish("casagob/rele/estado", "OFF"); // Cómputos
      lastReleState = LOW;
    }
  }
  else  {  //si esta desactivado el relé
    digitalWrite(ledROJO, LOW); // LED OFF
    digitalWrite(ledVERDE, HIGH); // LED ON
    if (lastReleState == LOW)   {  //si previamente estaba encendido
      Serial.println("La Energía Eléctrica se ha restablecido: Publicando ON");
      client.publish("casagob/rele/estado", "ON"); // Cómputos
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

  // Esperar 6 segundos antes de la próxima lectura
  delay(6000);
}