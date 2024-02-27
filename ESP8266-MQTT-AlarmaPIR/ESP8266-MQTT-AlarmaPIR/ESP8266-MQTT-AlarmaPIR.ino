#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi
//const char *ssid = "247IASbros2.4"; // Enter your WiFi name
//const char *password = "T3reKByo2023$";  // Enter WiFi password
const char *ssid = "datacenter"; // Enter your WiFi name
const char *password = "NOv22$1nicI0";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "45.186.124.70";
const char *topic = "datacenter/noc/alarma";
const char *mqtt_username = "admin";
const char *mqtt_password = "Ia$247forever";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//Alarm
int inputPin = D7; // for ESP8266 microcontroller
//int inputPin = 4; // for ESP32 microcontroller

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public Datacenter mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "hola Juancito");
  client.subscribe(topic);
  //Entrada para los eventos de alarma
  pinMode(inputPin, INPUT);
  Serial.begin(9600);
  delay(60*1000);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void loop() 
{
  client.loop();
  int val = digitalRead(inputPin);
  if (val == HIGH) {
    Serial.println("Motion detected!");
    }
  else {
    Serial.println("No Motion detected!");
    }
    
  delay(10);
}
