#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- CONFIGURACIÓN DE RED WIFI ---
const char* ssid = "datacenter";
const char* password = "NOv22$1nicI0";

// --- CONFIGURACIÓN MQTT ---
const char* mqtt_server = "172.16.16.98"; // Podés usar un broker público para probar
const int mqtt_port = 1883;
const char* mqtt_user = "adminmqtt";      // Dejar en blanco si no aplica
const char* mqtt_pass = "Ia$247";      // Dejar en blanco si no aplica
const char* root_topic_subscribe = "datacenter/refrigeracion/problemas";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n--- INICIANDO DIAGNÓSTICO ---");
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.print("1. Conectando a la red WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Esperar a que conecte (Time out implícito en el bucle)
  int contador = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    contador++;
    if(contador > 30) { // Si pasa más de 15 segundos sin conectar
      Serial.println("\n[ERROR] No se pudo conectar al WiFi. Verificá credenciales.");
      return;
    }
  }

  Serial.println("");
  Serial.println("[OK] WiFi Conectado con éxito!");
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());
}

// Función que recibe los mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido bajo el tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Bucle hasta conseguir la reconexión
  while (!client.connected()) {
    Serial.print("2. Intentando conexión MQTT a ");
    Serial.print(mqtt_server);
    Serial.print("... ");
    
    // Generar un Client ID único basado en el chip ID del ESP
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId(), HEX);

    // Intentar conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("\n[OK] Conexión al Broker MQTT establecida!");
      // Suscribirse al tópico de prueba
      client.subscribe(root_topic_subscribe);
      Serial.print("Suscripto al tópico: ");
      Serial.println(root_topic_subscribe);
    } else {
      Serial.print("[FALLÓ]. Código de error: ");
      Serial.print(client.state());
      Serial.println(" -> Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void loop() {
  // Verificar que el WiFi siga activo
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // Verificar que MQTT siga activo
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  // Publicar un "ping" cada 10 segundos para verificar subida de datos
  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    if (client.connected()) {
      Serial.println("Publicando mensaje de prueba en 'nodemcu/prueba/status'...");
      client.publish("nodemcu/prueba/status", "NodeMCU Vivo");
    }
  }
}

