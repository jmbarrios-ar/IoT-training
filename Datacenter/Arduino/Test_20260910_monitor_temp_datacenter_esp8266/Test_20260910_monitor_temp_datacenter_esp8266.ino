/*
  ============================================================================
  Monitor de Temperatura/Humedad - Centro de Datos (version ESP8266)
  Placa: NodeMCU ESP8266 (ESP-12E)
  Sensor: DHT11
  Salidas: LED verde (normal), LED rojo (alarma), Buzzer 12VDC (via relé/transistor)
  Comunicacion: MQTT (broker Home Assistant)

  USO: Este mismo codigo sirve para las DOS ubicaciones del proyecto.
  Solo cambia la seccion "CONFIGURACION POR UBICACION" mas abajo,
  descomentando la ubicacion correcta antes de subir el sketch a cada placa.
  ============================================================================
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>   // Instalar libreria "PubSubClient" (Nick O'Leary) desde el Library Manager
#include <DHT.h>        // Instalar libreria "DHT sensor library" (Adafruit) + "Adafruit Unified Sensor"

// ---------------------------------------------------------------------------
// CONFIGURACION POR UBICACION -> elegir UNA sola linea (comentar la otra)
// ---------------------------------------------------------------------------
#define UBICACION_REFRIGERACION
// #define UBICACION_RACKS

#ifdef UBICACION_REFRIGERACION
  const char* TOPIC_TEMP   = "datacenter/refrigeracion/temperatura";
  const char* TOPIC_HUM    = "datacenter/refrigeracion/humedad";
  const char* MQTT_PAYLOAD = "UMBRAL";
  const char* DEVICE_ID    = "ESP8266-Refrigeracion";
#endif

//#ifdef UBICACION_RACKS
//  const char* TOPIC_TEMP   = "datacenter/servidores/temperatura";
//  const char* TOPIC_HUM    = "datacenter/servidores/humedad";
//  const char* MQTT_PAYLOAD  = "RACKS";
//  const char* DEVICE_ID    = "ESP8266-Racks";
//#endif

// ---------------------------------------------------------------------------
// CONFIGURACION DE RED Y MQTT
// ---------------------------------------------------------------------------
const char* WIFI_SSID     = "datacenter";
const char* WIFI_PASSWORD = "NOv22$1nicI0";
//const char* WIFI_SSID     = "247IASbrosHA";
//const char* WIFI_PASSWORD = "GBxT0K6be99GmznHfUb247$!";

const char* MQTT_SERVER   = "172.16.16.27";  // IP del broker MQTT / Home Assistant
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "adminmqtt";           // dejar "" si el broker no requiere autenticacion
const char* MQTT_PASS     = "Ia$247";
//const char* MQTT_SERVER   = "192.168.55.150";  // IP del broker MQTT / Home Assistant
//const int   MQTT_PORT     = 1883;
//const char* MQTT_USER     = "usermqtt";           // dejar "" si el broker no requiere autenticacion
//const char* MQTT_PASS     = "Ia$247";

// ---------------------------------------------------------------------------
// CONFIGURACION DE PINES (GPIO) - NodeMCU ESP8266
// ---------------------------------------------------------------------------
// Justificacion de la seleccion (ver detalle completo debajo del codigo):
// Se evitan GPIO0 (D3), GPIO2 (D4) y GPIO15 (D8) por ser pines de "boot
// strapping": su nivel logico en el arranque decide el modo de boot del
// ESP8266, y un componente conectado ahi puede impedir que la placa inicie
// o generar falsos disparos (LED/buzzer) durante el arranque.

#define PIN_DHT       4    // D2 (GPIO4)  - Sensor DHT11 (data)
#define PIN_LED_VERDE 14   // D5 (GPIO14) - LED estado normal
#define PIN_LED_ROJO  12   // D6 (GPIO12) - LED estado alarma
#define PIN_BUZZER    13   // D7 (GPIO13) - Salida a rele/transistor que activa buzzer 12VDC

#define DHT_TYPE DHT11

const float UMBRAL_TEMP = 28.0;   // Umbral de temperatura en grados Celsius

// ---------------------------------------------------------------------------
// OBJETOS GLOBALES
// ---------------------------------------------------------------------------
DHT dht(PIN_DHT, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Control de tiempos sin usar delay() (no bloqueante)
unsigned long ultimaLecturaDHT   = 0;
const unsigned long INTERVALO_DHT = 3000;   // DHT11 necesita minimo ~2s entre lecturas

unsigned long ultimoIntentoMQTT  = 0;
const unsigned long INTERVALO_RECONEXION_MQTT = 5000;

// Estado previo del umbral, para publicar solo en el instante del cruce (flanco)
bool estadoAlarmaPrevio = false;

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_VERDE, HIGH);  // Arranca en estado normal
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  dht.begin();

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

// ---------------------------------------------------------------------------
// LOOP PRINCIPAL
// ---------------------------------------------------------------------------
void loop() {
  gestionarWiFi();     // Reconexion WiFi no bloqueante
  gestionarMQTT();      // Reconexion MQTT no bloqueante
  mqttClient.loop();    // Mantiene viva la conexion MQTT (heartbeat/keepalive)

  // Lectura periodica del sensor sin bloquear el programa
  unsigned long ahora = millis();
  if (ahora - ultimaLecturaDHT >= INTERVALO_DHT) {
    ultimaLecturaDHT = ahora;
    leerYEvaluarSensor();
  }
}

// ---------------------------------------------------------------------------
// WIFI: conexion inicial
// ---------------------------------------------------------------------------
void conectarWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// WIFI: revisa el estado y reconecta sin usar delay() ni bloquear el loop
void gestionarWiFi() {
  static unsigned long ultimoIntentoWiFi = 0;
  const unsigned long INTERVALO_RECONEXION_WIFI = 5000;

  if (WiFi.status() != WL_CONNECTED) {
    unsigned long ahora = millis();
    if (ahora - ultimoIntentoWiFi >= INTERVALO_RECONEXION_WIFI) {
      ultimoIntentoWiFi = ahora;
      Serial.println("WiFi desconectado, reintentando...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
}

// ---------------------------------------------------------------------------
// MQTT: reconexion no bloqueante
// ---------------------------------------------------------------------------
void gestionarMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;  // sin WiFi no tiene sentido intentar MQTT

  if (!mqttClient.connected()) {
    unsigned long ahora = millis();
    if (ahora - ultimoIntentoMQTT >= INTERVALO_RECONEXION_MQTT) {
      ultimoIntentoMQTT = ahora;
      Serial.print("Conectando a MQTT... ");
      bool ok;
      if (strlen(MQTT_USER) > 0) {
        ok = mqttClient.connect(DEVICE_ID, MQTT_USER, MQTT_PASS);
      } else {
        ok = mqttClient.connect(DEVICE_ID);
      }
      Serial.println(ok ? "conectado" : "fallo, reintenta en 5s");
    }
  }
}

// ---------------------------------------------------------------------------
// LECTURA DE SENSOR Y LOGICA DE ALARMA
// ---------------------------------------------------------------------------
void leerYEvaluarSensor() {
  float temperatura = dht.readTemperature();
  float humedad      = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error leyendo el sensor DHT11");
    return;
  }
// Convertir los valores float a int
  int ti;
  int hi;
  ti = (int) temperatura;
  hi = (int) humedad;
  // Publicar los valores en los tópicos MQTT
  String tempStr = String(ti);
  String humStr = String(hi);
 
  // Publicar temperatura
  mqttClient.publish(TOPIC_TEMP, tempStr.c_str());
  Serial.print("Temperatura Área Refrigeración: ");
  Serial.println(tempStr);

  // Publicar humedad
  mqttClient.publish(TOPIC_HUM, humStr.c_str());
  Serial.print("Humedad Área Refrigeración: ");
  Serial.println(humStr);

  Serial.printf("Temp: %.1f C  Hum: %.1f %%\n", temperatura, humedad);

  bool estadoAlarmaActual = (temperatura >= UMBRAL_TEMP);

  // LEDs y buzzer reflejan el estado actual en todo momento
  digitalWrite(PIN_LED_ROJO, estadoAlarmaActual ? HIGH : LOW);
  digitalWrite(PIN_LED_VERDE, estadoAlarmaActual ? LOW : HIGH);
  digitalWrite(PIN_BUZZER, estadoAlarmaActual ? HIGH : LOW);

  // Publicar por MQTT SOLO en el instante del disparo (flanco de subida del umbral)
  if (estadoAlarmaActual && !estadoAlarmaPrevio) {
    publicarAlarma();
  }

  estadoAlarmaPrevio = estadoAlarmaActual;
}

// ---------------------------------------------------------------------------
// PUBLICACION MQTT
// ---------------------------------------------------------------------------
void publicarAlarma() {
  if (mqttClient.connected()) {
    mqttClient.publish(TOPIC_TEMP, MQTT_PAYLOAD);
     Serial.print("Publicado -> topico: ");
    Serial.print(TOPIC_TEMP
);
    Serial.print(" | payload: "); 
    Serial.println(MQTT_PAYLOAD);
  } else {
    Serial.println("No se pudo publicar: MQTT desconectado");
  }
}
