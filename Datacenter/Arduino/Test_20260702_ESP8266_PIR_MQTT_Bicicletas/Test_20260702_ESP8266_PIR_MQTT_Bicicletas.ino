
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid="TU_SSID";
const char* password="TU_PASSWORD";
const char* mqtt_server="192.168.1.100";
const int mqtt_port=1883;
const char* mqtt_topic="negocio/oficina/bicicletas";

WiFiClient espClient;
PubSubClient client(espClient);

const int PIR_PIN=D1;
const int LED_ROJO=D5;
const int LED_VERDE=D6;

const unsigned long TIEMPO_ACTIVADO=10000;
const unsigned long INTERVALO_PARPADEO=500;

bool movimiento=false;
unsigned long inicioMovimiento=0;
unsigned long ultimoParpadeo=0;
bool estadoRojo=false;

void conectarWiFi(){
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid,password);
 while(WiFi.status()!=WL_CONNECTED){delay(500);}
}
void reconnectMQTT(){
 while(!client.connected()){
  String id="ESP8266_Bicicletas_"+String(ESP.getChipId(),HEX);
  if(!client.connect(id.c_str())) delay(5000);
 }
}
void setup(){
 pinMode(PIR_PIN,INPUT);
 pinMode(LED_ROJO,OUTPUT);
 pinMode(LED_VERDE,OUTPUT);
 Serial.begin(115200);
 conectarWiFi();
 client.setServer(mqtt_server,mqtt_port);
 digitalWrite(LED_VERDE,HIGH);
 digitalWrite(LED_ROJO,LOW);
}
void loop(){
 if(!client.connected()) reconnectMQTT();
 client.loop();
 unsigned long ahora=millis();
 if(digitalRead(PIR_PIN)==HIGH){
   if(!movimiento){
      movimiento=true;
      inicioMovimiento=ahora;
      client.publish(mqtt_topic,"MOVIMIENTO");
   } else inicioMovimiento=ahora;
 }
 if(movimiento){
    digitalWrite(LED_VERDE,LOW);
    if(ahora-ultimoParpadeo>=INTERVALO_PARPADEO){
      ultimoParpadeo=ahora;
      estadoRojo=!estadoRojo;
      digitalWrite(LED_ROJO,estadoRojo);
    }
    if(ahora-inicioMovimiento>=TIEMPO_ACTIVADO){
      movimiento=false;
      digitalWrite(LED_ROJO,LOW);
      digitalWrite(LED_VERDE,HIGH);
    }
 } else {
    digitalWrite(LED_VERDE,HIGH);
    digitalWrite(LED_ROJO,LOW);
 }
}
