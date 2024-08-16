//ESP32 send DHT temperature and humidity sensor data to Home Assistant via MQTT Broker
//Tutorial: TBC
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti WiFiMulti;
#include "DHT.h"
#include <PubSubClient.h>

const char* ssid = ""; //Your wifi network's SSID
const char* password = ""; //Your wifi network's password
const char* mqtt_server = "";  //Your network's MQTT server (usually same IP address as Home Assistant server)
#define DHTPIN 32 //Must be an ADC1 pin if wifi is connected

//Uncomment which type of temperature/humidity sensor you're using
//#define DHTTYPE DHT11
#define DHTTYPE DHT22
//#define DHTTYPE DHT21

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {

  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop(){

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(1000);
    return;
  } else {

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));

  triggerAlert(t, h);

}

  delay(30000); //Rest for 30 seconds (change this value if you want temperature/humidity readings at specific time intervals)

}

void triggerAlert(float temperature, float humidity) {
  Serial.println("Sending alert via MQTT...");
  if (!client.connected()) {
    reconnect();
  }

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    //msg variable contains JSON string to send to MQTT server
    snprintf (msg, MSG_BUFFER_SIZE, "\{\"temperature\": %4.1f, \"humidity\": %4.1f\}", temperature, humidity);
    //Due to a quirk with escaping special characters, if you're using an ESP8266 you will need to use this instead:
    //snprintf (msg, MSG_BUFFER_SIZE, "{\"temperature\": %4.1f, \"humidity\": %4.1f}", temperature, humidity);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("DHT22/01", msg);
  }

}
