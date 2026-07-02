/***********************************************************************
 * Proyecto : Detector de Movimiento para Bicicletero
 * Hardware : ESP8266 + PIR HC-SR501 + LED Verde + LED Rojo
 * MQTT     : Publica "MOVIMIENTO" en negocio/oficina/bicicletas
 ***********************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/***********************************************************************
 * CONFIGURACION
 ***********************************************************************/
const char* WIFI_SSID     = "TU_SSID";
const char* WIFI_PASSWORD = "TU_PASSWORD";

const char* MQTT_SERVER = "192.168.1.100";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "negocio/oficina/bicicletas";

// Pines
const byte PIN_PIR       = D1;
const byte PIN_LED_ROJO  = D5;
const byte PIN_LED_VERDE = D6;

// Tiempos
const unsigned long TIEMPO_DETECCION = 10000;     // 10 segundos
const unsigned long PARPADEO_LED     = 500;       // ms
const unsigned long FILTRO_PIR       = 300;       // ms

// Reconexión
const unsigned long WIFI_RETRY = 10000;
const unsigned long MQTT_RETRY = 5000;

/***********************************************************************
 * VARIABLES
 ***********************************************************************/
WiFiClient espClient;
PubSubClient client(espClient);

bool movimientoActivo = false;
bool estadoLedRojo = false;

unsigned long tMovimiento = 0;
unsigned long tParpadeo = 0;
unsigned long tFiltro = 0;
unsigned long tWifi = 0;
unsigned long tMQTT = 0;

/***********************************************************************
 * WIFI
 ***********************************************************************/
void conectarWiFi()
{
    Serial.println("Conectando WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void verificarWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    if (millis() - tWifi >= WIFI_RETRY)
    {
        tWifi = millis();

        Serial.println("Reconectando WiFi...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
}

/***********************************************************************
 * MQTT
 ***********************************************************************/
void verificarMQTT()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (client.connected())
        return;

    if (millis() - tMQTT >= MQTT_RETRY)
    {
        tMQTT = millis();

        String id = "ESP8266_" + String(ESP.getChipId(), HEX);

        Serial.println("Reconectando MQTT...");

        if (client.connect(id.c_str()))
        {
            Serial.println("MQTT conectado");
        }
    }
}

/***********************************************************************
 * SETUP
 ***********************************************************************/
void setup()
{
    Serial.begin(115200);

    pinMode(PIN_PIR, INPUT);

    pinMode(PIN_LED_ROJO, OUTPUT);
    pinMode(PIN_LED_VERDE, OUTPUT);

    digitalWrite(PIN_LED_ROJO, LOW);
    digitalWrite(PIN_LED_VERDE, HIGH);

    conectarWiFi();

    client.setServer(MQTT_SERVER, MQTT_PORT);
}

/***********************************************************************
 * LOOP
 ***********************************************************************/
void loop()
{
    verificarWiFi();
    verificarMQTT();

    client.loop();

    unsigned long ahora = millis();

    /***********************
     * LECTURA PIR
     ***********************/
    bool pir = digitalRead(PIN_PIR);

    if (pir == HIGH)
    {
        // Filtro de rebotes
        if (ahora - tFiltro >= FILTRO_PIR)
        {
            tFiltro = ahora;

            if (!movimientoActivo)
            {
                movimientoActivo = true;

                tMovimiento = ahora;

                client.publish(MQTT_TOPIC, "MOVIMIENTO");

                Serial.println("Movimiento detectado");
            }
            else
            {
                // Si continúa detectando movimiento,
                // renueva el temporizador de los 10 s.
                tMovimiento = ahora;
            }
        }
    }

    /***********************
     * ESTADO ACTIVO
     ***********************/
    if (movimientoActivo)
    {
        digitalWrite(PIN_LED_VERDE, LOW);

        if (ahora - tParpadeo >= PARPADEO_LED)
        {
            tParpadeo = ahora;

            estadoLedRojo = !estadoLedRojo;

            digitalWrite(PIN_LED_ROJO, estadoLedRojo);
        }

        if (ahora - tMovimiento >= TIEMPO_DETECCION)
        {
            movimientoActivo = false;

            digitalWrite(PIN_LED_ROJO, LOW);
            digitalWrite(PIN_LED_VERDE, HIGH);

            Serial.println("Fin deteccion");
        }
    }
    else
    {
        digitalWrite(PIN_LED_VERDE, HIGH);
        digitalWrite(PIN_LED_ROJO, LOW);
    }
}
