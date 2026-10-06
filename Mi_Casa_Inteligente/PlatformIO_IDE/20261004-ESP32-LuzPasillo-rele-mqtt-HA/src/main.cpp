#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Hardware: HC-SR501 OUT -> GPIO27; relay IN -> GPIO25; connect grounds.
constexpr uint8_t PIR_PIN = 27;
constexpr uint8_t RELAY_PIN = 25;
constexpr bool RELAY_ACTIVE_LOW = true; // Set false for an active-HIGH relay board.
constexpr uint32_t LIGHT_ON_MS = 120000UL;
constexpr uint32_t STATE_PUBLISH_INTERVAL_MS = 5000UL;

// Replace these values with the credentials and broker address on your LAN.
const char *WIFI_SSID = "TU_WIFI";
const char *WIFI_PASSWORD = "TU_CLAVE_WIFI";
const char *MQTT_HOST = "192.168.1.10"; // Home Assistant / MQTT broker IP
constexpr uint16_t MQTT_PORT = 1883;
const char *MQTT_USER = "TU_USUARIO_MQTT"; // Use "" if broker allows anonymous access.
const char *MQTT_PASSWORD = "TU_CLAVE_MQTT";

const char *COMMAND_TOPIC = "casa/iluminacion/pasillo";
const char *STATE_TOPIC = "casa/iluminacion/pasillo/estado";
const char *DEVICE_ID = "esp32-luz-pasillo";

WiFiClient networkClient;
PubSubClient mqttClient(networkClient);

bool lightOn = false;
bool pirArmed = false;
uint32_t lightStartedAt = 0;
uint32_t lastStatePublishAt = 0;
uint32_t lastWifiAttemptAt = 0;
uint32_t lastMqttAttemptAt = 0;

void setRelay(bool on) {
  const uint8_t onLevel = RELAY_ACTIVE_LOW ? LOW : HIGH;
  const uint8_t offLevel = RELAY_ACTIVE_LOW ? HIGH : LOW;
  digitalWrite(RELAY_PIN, on ? onLevel : offLevel);
}

void publishState(bool force = false) {
  if (!mqttClient.connected()) return;

  const uint32_t now = millis();
  if (!force && static_cast<uint32_t>(now - lastStatePublishAt) < STATE_PUBLISH_INTERVAL_MS) return;

  mqttClient.publish(STATE_TOPIC, lightOn ? "ON" : "OFF", true);
  lastStatePublishAt = now;
}

void turnLightOn(const char *source) {
  const bool wasOn = lightOn;
  lightOn = true;
  lightStartedAt = millis(); // Every accepted activation starts a fresh 120 s interval.
  setRelay(true);
  if (!wasOn) {
    Serial.printf("Luz ON (%s)\n", source);
    publishState(true);
  }
}

void turnLightOff() {
  lightOn = false;
  setRelay(false);
  // Require a LOW level after the light turns off before accepting another PIR event.
  // This discards a PIR HIGH left over from activity while detections were ignored.
  pirArmed = (digitalRead(PIR_PIN) == LOW);
  Serial.println("Luz OFF; PIR esperando nivel LOW para rearmarse");
  publishState(true);
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, COMMAND_TOPIC) != 0) return;
  constexpr char command[] = "LUZPASILLO";
  if (length != sizeof(command) - 1 || memcmp(payload, command, length) != 0) return;

  turnLightOn("MQTT");
}

void maintainConnections() {
  const uint32_t now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (static_cast<uint32_t>(now - lastWifiAttemptAt) >= 10000UL) {
      lastWifiAttemptAt = now;
      Serial.println("Conectando a WiFi...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    return;
  }

  if (!mqttClient.connected() && static_cast<uint32_t>(now - lastMqttAttemptAt) >= 5000UL) {
    lastMqttAttemptAt = now;
    Serial.println("Conectando a MQTT...");
    if (mqttClient.connect(DEVICE_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT conectado");
      mqttClient.subscribe(COMMAND_TOPIC);
      publishState(true);
    } else {
      Serial.printf("Fallo MQTT, estado=%d\n", mqttClient.state());
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Preload OFF before enabling output to reduce the chance of a boot-time relay pulse.
  setRelay(false);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pirArmed = (digitalRead(PIR_PIN) == LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttemptAt = millis();

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(2);

  Serial.println("Control de luz de pasillo iniciado");
  Serial.printf("PIR %s; relay %s\n", pirArmed ? "armado" : "esperando LOW", lightOn ? "ON" : "OFF");
}

void loop() {
  maintainConnections();
  if (mqttClient.connected()) mqttClient.loop();

  const uint32_t now = millis();

  if (lightOn) {
    // Intentionally do not sample or process the PIR while the relay is active.
    if (static_cast<uint32_t>(now - lightStartedAt) >= LIGHT_ON_MS) turnLightOff();
  } else {
    const bool pirHigh = (digitalRead(PIR_PIN) == HIGH);
    if (!pirArmed) {
      if (!pirHigh) pirArmed = true;
    } else if (pirHigh) {
      turnLightOn("PIR");
    }
  }

  publishState(false);
  delay(5);
}
