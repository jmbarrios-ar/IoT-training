//***** ALARMA MONITOREO ESTADO DE GRUPO ELECTRÓGENO, TEMPERATURA/HUMEDAD y DETECCIÓN DE AGUA EN DATACENTER *****//
// Revisión 2026-09-30:
//  - Loop NO bloqueante (millis() en lugar de delay()) -> el MQTT y el relé se atienden siempre.
//  - Agua: publica AGUAS / NOAGUA UNA sola vez por cambio de estado, con umbral + histéresis + confirmación.
//  - Si la publicación falla (MQTT caído), se reintenta hasta lograrla: no se pierde el cambio de estado.
//  - Corrección de Ethernet.begin() con IP fija (orden de parámetros).
//  - Lectura DHT11 con la API correcta de la librería de Dhruba Saha (devuelve int + código de error).
//  - Strings en F() para ahorrar RAM del UNO (2 KB).
 
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT11.h>      // Librería "DHT11" by Dhruba Saha (v2.x)
 
void(* Resetea) (void) = 0; // Reset por soft (salta a la dirección 0)
 
// ********** PINES *********************************
const uint8_t PIN_DHT   = 2;   // DHT11 (data)
const uint8_t PIN_RELE  = 3;   // COM del relé (con pull-down de 10k a GND)
const uint8_t LED_ROJO  = 4;   // GE ENCENDIDO  (ver nota: pin 4 = CS de la SD del Ethernet Shield)
const uint8_t LED_VERDE = 5;   // GE APAGADO
const uint8_t PIN_AGUA  = A0;  // Señal S del sensor de agua
 
// ********** ETHERNET *********************************
// IMPORTANTE: cada Arduino de la red debe tener una MAC distinta (Test y Producción).
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(172, 16, 16, 41);     // Datacenter - Arduino UNO producción
//IPAddress ip(172, 16, 16, 141);    // Datacenter - Arduino UNO Test
IPAddress ip(192, 168, 55, 124);     // Barrio NORTE - Arduino UNO Test
//IPAddress gateway(172, 16, 16, 16);// Datacenter
IPAddress gateway(192, 168, 55, 1);  // Barrio NORTE
IPAddress subnet(255, 255, 255, 0);
IPAddress dnServer(8, 8, 8, 8);
bool usandoDHCP = true;
 
// ********** MQTT *********************************
//const char *mqtt_server = "172.16.16.27";   // Datacenter
//const char *mqtt_server = "172.16.16.98";   // Datacenter Test
const char *mqtt_server = "192.168.55.150";   // Barrio NORTE
const int   mqtt_port   = 1883;
//const char *mqtt_user   = "adminmqtt";   // Datacenter
const char *mqtt_user   = "usermqtt";   // Barrio NORTE
const char *mqtt_pass   = "Ia$247";
//const char *mqtt_clientId = "arduino_uno_datacenter"; // ID fijo y único por equipo
const char *mqtt_clientId = "arduino_uno_test_norte"; // ID fijo y único por equipo
 
// Tópicos MQTT
//const char* topicTemp = "datacenter/dht11/temperatura";
const char* topicTemp = "casa/climatizacion/temperatura";
//const char* topicHum  = "datacenter/dht11/humedad";
const char* topicHum  = "casa/climatizacion/humedad";
//const char* topicAgua = "datacenter/entrepiso/agua";
const char* topicAgua = "casa/entrepiso/agua";
//const char* topicGE   = "datacenter/grupo/estado";
const char* topicGE   = "casa/rele/estado";
 
// ********** TIEMPOS (ms) *********************************
const unsigned long INTERVALO_DHT        = 30000; // Lectura/publicación temp. y humedad
const unsigned long INTERVALO_AGUA       = 500;   // Muestreo del sensor de agua
const unsigned long DEBOUNCE_GE          = 1000;  // Estabilidad requerida del relé
const unsigned long INTERVALO_RECONEXION = 5000;  // Reintento MQTT
const uint8_t       MAX_FALLOS_MQTT      = 24;    // ~2 min sin broker -> reset del Arduino
 
// ********** SENSOR DE AGUA (calibrar con el monitor serie) *********
const int     UMBRAL_AGUA_ON  = 200;  // >= este valor: hay agua
const int     UMBRAL_AGUA_OFF = 100;  // <= este valor: no hay agua (histéresis)
const uint8_t LECTURAS_CONFIRMACION = 4; // lecturas consecutivas (4 x 500 ms = 2 s)
 
// ********** OBJETOS *********************************
DHT11 dht11(PIN_DHT);
EthernetClient ethClient;
PubSubClient client(ethClient);
 
// ********** ESTADOS *********************************
// Grupo electrógeno
bool geLecturaAnterior = false;
bool geEstado          = false;  // estado estable (con debounce)
bool gePublicado       = false;  // último estado publicado con éxito (se asume OFF al inicio)
unsigned long tCambioGE = 0;
 
// Agua
bool    aguaEstado    = false;   // estado confirmado del sensor
bool    aguaPublicada = false;   // último estado publicado con éxito (se asume NOAGUA al inicio)
uint8_t contAgua      = 0;
unsigned long tAgua   = 0;
 
// DHT / MQTT
unsigned long tDHT = 0;
unsigned long tReconexion = 0;
uint8_t fallosMQTT = 0;
 
// ********** DECLARACIÓN DE FUNCIONES ****************
void reconnect();
void relegrupoelectrogeno();
void tempyhumd();
void aguadeteccion();
 
void setup() {
  Serial.begin(9600);
  pinMode(PIN_RELE, INPUT);      // pull-down externo de 10k
  pinMode(LED_ROJO, OUTPUT);    // Led indicador CON corte energía
  pinMode(LED_VERDE, OUTPUT);   // Led indicador SIN corte energía
  digitalWrite(LED_ROJO, LOW);  // Apagar el LED inicialmente
  digitalWrite(LED_VERDE, HIGH);  // Encender el LED inicialmente
 
  // ***** Red: DHCP y, si falla, IP fija *****
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Fallo DHCP, usando IP fija"));
    // Orden correcto: mac, ip, dns, gateway, subnet
    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
    usandoDHCP = false;
  }
  // Solo IP fija:
  //Ethernet.begin(mac, ip, dnServer, gateway, subnet); usandoDHCP = false;
 
  delay(1000); // inicialización del shield
  Serial.print(F("IP Address: "));
  Serial.println(Ethernet.localIP());
 
  client.setServer(mqtt_server, mqtt_port);
 
  tDHT = millis() - INTERVALO_DHT;                  // primera lectura DHT inmediata
  tReconexion = millis() - INTERVALO_RECONEXION;    // primer intento MQTT inmediato
}
 
void loop() {
  if (usandoDHCP) Ethernet.maintain();   // renueva la concesión DHCP
 
  if (!client.connected()) {
    reconnect();                         // no bloqueante
  } else {
    client.loop();
  }
 
  // Los sensores y LEDs funcionan aunque MQTT esté caído
  relegrupoelectrogeno();
  aguadeteccion();
  tempyhumd();
}
 
//******** RECONEXIÓN MQTT (NO BLOQUEANTE) *******
void reconnect() {
  if (millis() - tReconexion < INTERVALO_RECONEXION) return;
  tReconexion = millis();
 
  Serial.print(F("Intentando conexion MQTT..."));
  if (client.connect(mqtt_clientId, mqtt_user, mqtt_pass)) {
    Serial.println(F(" Conectado!"));
    fallosMQTT = 0;
  } else {
    Serial.print(F(" Fallo, rc="));
    Serial.print(client.state());
    Serial.println(F(". Reintento en 5 s"));
    if (++fallosMQTT >= MAX_FALLOS_MQTT) {
      Serial.println(F("Demasiados fallos MQTT, reiniciando..."));
      delay(100);
      Resetea();
    }
  }
}
 
//************ DETECTAR ENCENDIDO GRUPO ELECTRÓGENO **************
void relegrupoelectrogeno() {
  bool lectura = (digitalRead(PIN_RELE) == HIGH);
 
  // Debounce: el estado debe mantenerse DEBOUNCE_GE ms
  if (lectura != geLecturaAnterior) {
    geLecturaAnterior = lectura;
    tCambioGE = millis();
  }
  if (millis() - tCambioGE >= DEBOUNCE_GE) {
    geEstado = lectura;
  }
 
  digitalWrite(LED_ROJO,  geEstado ? HIGH : LOW);
  digitalWrite(LED_VERDE, geEstado ? LOW  : HIGH);
 
  // Publica una sola vez por cambio; si falla, reintenta en el próximo loop
  if (geEstado != gePublicado && client.connected()) {
    if (client.publish(topicGE, geEstado ? "ON" : "OFF")) {
      gePublicado = geEstado;
      Serial.println(geEstado ? F("Grupo Electrogeno ENCENDIDO: publicado ON")
                              : F("Grupo Electrogeno APAGADO: publicado OFF"));
    }
  }
}
 
//************ LEER TEMPERATURA Y HUMEDAD DESDE SENSOR DHT11 **************
void tempyhumd() {
  if (millis() - tDHT < INTERVALO_DHT) return;
  tDHT = millis();
 
  int t = 0, h = 0;
  int resultado = dht11.readTemperatureHumidity(t, h);  // una sola lectura para ambos valores
  if (resultado != 0) {
    Serial.print(F("Error DHT11: "));
    Serial.println(DHT11::getErrorString(resultado));
    return;
  }
 
  Serial.print(F("Temperatura: ")); Serial.print(t);
  Serial.print(F(" C - Humedad: ")); Serial.print(h); Serial.println(F(" %"));
 
  if (!client.connected()) return;
  char buf[8];
  itoa(t, buf, 10);
  client.publish(topicTemp, buf);
  itoa(h, buf, 10);
  client.publish(topicHum, buf);
}
 
//************ DETECCIÓN ANALÓGICA DE AGUA **************
void aguadeteccion() {
  if (millis() - tAgua < INTERVALO_AGUA) return;
  tAgua = millis();
 
  int lectura = analogRead(PIN_AGUA);
  //Serial.print(F("Sensor agua (raw): ")); Serial.println(lectura);  // descomentar para calibrar
 
  // Histéresis: activa con UMBRAL_ON, desactiva con UMBRAL_OFF
  bool candidato = aguaEstado;
  if (!aguaEstado && lectura >= UMBRAL_AGUA_ON)  candidato = true;
  if ( aguaEstado && lectura <= UMBRAL_AGUA_OFF) candidato = false;
 
  // Confirmación: el cambio debe repetirse N lecturas consecutivas
  if (candidato != aguaEstado) {
    if (++contAgua >= LECTURAS_CONFIRMACION) {
      aguaEstado = candidato;
      contAgua = 0;
      Serial.println(aguaEstado ? F("Cambio de estado: AGUA DETECTADA en entrepiso")
                                : F("Cambio de estado: entrepiso SIN agua"));
    }
  } else {
    contAgua = 0;
  }
 
  // Publica UNA sola vez por cambio de estado. Solo se marca como publicado
  // si el broker lo aceptó; si MQTT estaba caído, se envía al reconectar.
  if (aguaEstado != aguaPublicada && client.connected()) {
    if (client.publish(topicAgua, aguaEstado ? "AGUAS" : "NOAGUA")) {
      aguaPublicada = aguaEstado;
      Serial.println(aguaEstado ? F("Publicado AGUAS") : F("Publicado NOAGUA"));
    }
  }
}
 