#include <WiFi.h>
#include <PubSubClient.h>
//#include <DHT11.h>
#include <DHT.h>

#define DHTPIN 21 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT (DHT11 o DHT22)

// ********** Configuración de la red WiFi ******************
const char* ssid = "247IASbros1";       // Nombre de la red WiFi
const char* password = "GBxT0K6be99GmznHfUb7"; // Contraseña de la red WiFi

// ********* Configuración del servidor MQTT ********
const char* mqttServer = "192.168.55.150"; // Cambia por tu servidor MQTT
const int mqttPort = 1883;                   // Puerto del servidor MQTT
const char* mqttUser = "usermqtt";                   // Usuario (si aplica)
const char* mqttPassword = "Ia$247";               // Contraseña (si aplica)

// ************ Config pines para relés que activan porton y ventilador ************
const int porton = 5;  // Conexión a la entrada del modulo relé señal porton
const int fan1 = 26;  // Conexión para relé control de Fan1
const int fan2 = 27;  // Conexión para relé control de Fan2
//int lastReleState = HIGH; // de inicio el relé esta abierto
//const int val = 1; 

// *********** Variables para el control del tiempo ***********
unsigned long previousMillis = 0;
//const long interval = 120 * 60 * 1000; // 120 minutos en milisegundos
const long interval = 1 * 60 * 1000;
bool fan1State = false;
bool fan2State = true; // Iniciar con el ventilador 2 encendido (alternado)

// *********Config sensor de temperatura ***************
//int pin=35;
//DHT11 dht11(pin);  // Asignacion del pin del DHT11, el RTC tiene SDA en el A4 (SDA del arduino) y el SCL en el A5 (SCL del arduino)
//const int umbral = 24;  //Temperatura que activa alarma
DHT dht11(DHTPIN, DHTTYPE);

// ****************** Tópicos MQTT Barrio NORTE **************************
const char* topicTemp = "casa/garage/dht11/temperatura";     // Tópico para la temperatura
const char* topicHum = "casa/garage/dht11/humedad";          // Tópico para la humedad

// Variables de WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// *********** Configuración inicial ***********

//************* DECLARAR FUNCIONES ***************************
//void setup_wifi();
//void callback(char* topic, byte* payload, unsigned int length);
//void reconnect();

// Función para conectar al WiFi
void setup_wifi() {
  delay(1 * 30 * 1000); // Espera 30 seg hasta que este disponible el WiFi 247IASbros1
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
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Creamos un cliente ID
    String clientId = "esp32_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqttUser,mqttPassword)) {
      Serial.println("Conectado!");
      
      // Suscribirse al tópico del relé del porton
      client.subscribe("casa/porton/control");  //Barrios NORTE
    } else {
      Serial.print("Falló la conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 2 segundos");
      delay(2000); // Esperar 2 seg antes de intentar nuevamente
    }
  }
}

// Función callback para manejar mensajes MQTT entrantes
void callback(char* topic, byte* payload, unsigned int length) {
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
  if (String(topic) == "casa/porton/control" && messageTemp == "PULSO") {  //Barrio NORTE
    Serial.println("Pulso de 1 segundo enviado al rele");
    digitalWrite(porton, HIGH);  // Activar el relé
    delay(10000);  // Mantener el estado HIGH durante 1 segundo
    digitalWrite(porton, LOW);   // Desactivar el relay
    Serial.println("El rele se desactivó");
    
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
  Serial.print("Temperatura en rack IoT: ");
  Serial.println(tempStr);

  // Publicar humedad
  client.publish(topicHum, humStr.c_str());
  Serial.print("Humedad en rack IoT: ");
  Serial.println(humStr);

  // Esperar 1 segundo antes de la próxima lectura
  delay(1000);
}

//************ CONTROL DE VENTILADORES FAN1 Y FAN2 **************
void controlfanes() {
  unsigned long currentMillis = millis();
  
  // Control del modo alternado cada 120 minutos
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Alternar los estados de los ventiladores
    fan1State = !fan1State;
    fan2State = !fan2State;
    
    // Aplicar los nuevos estados a los relés
    digitalWrite(fan1, fan1State ? HIGH : LOW);
    digitalWrite(fan2, fan2State ? HIGH : LOW);
    
    //printFanStates();
  }
  
  delay(1000); // Pequeña pausa para evitar sobrecarga
}

void setup() {
  Serial.begin(115200); // Inicializa comunicación serial
  dht11.begin(); // initialize the DHT11 sensor
  pinMode(porton, OUTPUT); // Inicializar el pin modulo relé del portón y el led piloto
  digitalWrite(porton, LOW);  // Apagar inicialmente el relé del portón y el led
  pinMode(fan1, OUTPUT); // Inicializar el pin modulo relé del ventilador1 y el led piloto
  pinMode(fan2, OUTPUT); // Inicializar el pin modulo relé del ventilador2 y el led piloto
  digitalWrite(fan1, LOW);  // Apagar inicialmente el relé del ventilador y el led
  digitalWrite(fan2, LOW);  // Apagar inicialmente el relé del ventilador y el led
  
  setup_wifi();      // Conexión inicial al WiFi
  client.setServer(mqttServer, mqttPort); // Configura el servidor MQTT
  client.setCallback(callback);       // Asigna la función callback
}

// Bucle principal
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Intentando reconectar...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop(); // Mantiene la conexión MQTT viva
  tempyhumd();
  controlfanes();

  // Opcional: Publicar un mensaje periódicamente
  /*static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 5000) {
    lastPublish = millis();
    client.publish("test/topic", "Hello from ESP32!");
    Serial.println("Mensaje publicado en el topic test/topic.");
  }*/
}



/*void printFanStates() {
  Serial.println();
  Serial.print("Estado actual - ");
  Serial.print("FAN1: ");
  Serial.print(fan1State ? "ENCENDIDO" : "APAGADO");
  Serial.print(" | FAN2: ");
  Serial.println(fan2State ? "ENCENDIDO" : "APAGADO");
  Serial.print("Próximo cambio en: ");
  Serial.print(interval / 60000);
  Serial.println(" minutos");
}*/

//************ ACTIVACIÓN DE VENTILADOR FAN **************
/* void releventilador(){
  // Lectura del estado del Relé detector de energía eléctrica
  val = digitalRead(relePin);
  if (val == LOW)  {  //si está activado
    digitalWrite(ledROJO, HIGH);  //LED ROJO ENCENDIDO
    digitalWrite(ledVERDE, LOW);  //LED VERDE APAGADO
    if (lastReleState == HIGH)  {  //si previamente estaba apagado
      Serial.println("Corte de Energía Eléctrica detectado: Publicando OFF");
      mqttclient.publish("casa/rele/estado", "OFF"); // Barrio NORTE
      lastReleState = LOW;
    }
  }
  else  {  //si esta desactivado
    digitalWrite(ledROJO, LOW); // LED OFF
    digitalWrite(ledVERDE, HIGH); // LED ON
    if (lastReleState == LOW)   {  //si previamente estaba encendido
      Serial.println("La Energía Eléctrica se ha restablecido: Publicando ON");
      mqttclient.publish("casa/rele/estado", "ON"); // Barrio NORTE
      lastReleState = HIGH;
    }
  }
} */