#include <Arduino.h>    
#include <WiFi.h>   // Librería para conectar al Wifi
#include <PubSubClient.h>   // Librería para MQTT
#include "secretos.h";   // Aquí he dejado los datos de conexión de mi Wifi y mi usuario/contraseña para MQTT

const char* mqtt_server = "192.168.0.192";  // La dirección de mi servidor de MQTT

WiFiClient espClient;
PubSubClient cliente(espClient);

const int ledPin = 33;
const int pulsadorPin = 25;

void setup_wifi() {
  delay(10);
  // Conectando al Wifi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Mientras esperamos la conexión vamos mostrando un punto en el terminal
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Conexión realizada
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}


// Función para escuchar los mensajes recibidos por MQTT
void escuchar(char* topic, byte* mensaje, unsigned int largo) {
  Serial.print("Mensaje recibido en el topic: ");
  Serial.print(topic);
  Serial.print(". Mensaje: ");
  String mensajeTemp;
  
  // Copiamos la cadena de bytes recibidos en una variable tipo String, y la mostramos en el terminal
  for (int i = 0; i < largo; i++) {
    Serial.print((char)mensaje[i]);
    mensajeTemp += (char)mensaje[i];
  }
  Serial.println();

  // Si el mensaje recibido tiene un payload = on encendemos el led
  Serial.print("Cambiando el led a ");
  if(mensajeTemp == "on"){
    Serial.println("on");
    digitalWrite(ledPin, LOW);
  } // Si el payload = off apagamos el led
  else if(mensajeTemp == "off"){
    Serial.println("off");
    digitalWrite(ledPin, HIGH);
  }
}

// Conexión a MQTT
void conectarMQTT() {
  // Si no está conectado, entramos en bucle hasta que conecte
  while (!cliente.connected()) {
    Serial.print("Conectando a MQTT...");

    // Ahora intentamos la conexión
    if (cliente.connect("ESP8266Client",usuario,contrase)) {
      Serial.println("conectado");
      
      // Suscribimos nuestro ESP32 al topic esp32/led para escuchar los mensajes recibidos
      cliente.subscribe("esp32/led"); 
    } else {
      // si no conecta esperamos 5 segundos para intentarlo de nuevo más tarde
      Serial.print("fallo, rc=");
      Serial.print(cliente.state());
      Serial.println(" reintento en 5 segundos");
      
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200); // Configuración del puerto serie
  setup_wifi(); // Conexión al Wifi
  
  // Establece la configuración del servidor MQTT
  cliente.setServer(mqtt_server, 1883);
  // Se define qué función va a ser llamada cuando llegue un mensaje por MQTT
  cliente.setCallback(escuchar);

  // Inicialización de pines del led y el pulsador
  pinMode(ledPin, OUTPUT);
  pinMode(pulsadorPin, INPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  // Si no se ha establecido la conexión (o se ha perdido) intentamos conectar
  if (!cliente.connected()) {
    conectarMQTT();
  }
  // El cliente de MQTT necesita un llamar a loop para procesar mensajes entrantes
  // y publicar los mensajes salientes
  cliente.loop();

  if (!digitalRead(pulsadorPin)) // Si se ha pulsado el pulsador 
    {
      // Enviamos el payload "Pulsado"
      Serial.println("Pulsado");
      cliente.publish("esp32/pulsador", "Pulsado");

      // Mientras el pulsador siga pulsado, esperamos en este bucle
      while (!digitalRead(pulsadorPin));

      // Cuando quitamos el dedo del pulsador, enviamos el payload "Soltado"
      Serial.println("Soltado");
      cliente.publish("esp32/pulsador", "Soltado");
    }
}