// ALARMA PIR movimiento con modulo relé y MQTT para Home Assistant //
// ================= CONFIGURACIÓN =================
// #define PIR_PIN   D5    // GPIO14 ESP8266
// #define RELAY_PIN D6    // GPIO12 ESP8266
#define PIR_PIN   27   //ESP32
#define RELAY_PIN 26   //ESP32
// ============ PARÁMETROS ============
const unsigned long TIEMPO_ENCENDIDO = 10000; // 10 segundos durante el cual se enciende luz en el relé

bool releActivo = false;
unsigned long tiempoInicio = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Apagar relé al inicio (muchos relés son activos en LOW)
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Sistema iniciado...");
  Serial.println("Esperando estabilización del sensor PIR...");

  delay(30000); // 30 segundos para estabilización del PIR

  Serial.println("Sistema listo. Esperando movimiento.");
}

// ================= LOOP =================
void loop() {

  int estadoPIR = digitalRead(PIR_PIN);

  // Si detecta movimiento y el relé no está activo
  if (estadoPIR == HIGH && !releActivo) {
    Serial.println("Movimiento detectado!");

    activarRele();
  }

  // Si el relé está activo, verificar tiempo
  if (releActivo) {
    if (millis() - tiempoInicio >= TIEMPO_ENCENDIDO) {
      desactivarRele();
    }
  }
}

// ================= FUNCIONES =================

void activarRele() {
  // Muchos relés son activos en LOW
  digitalWrite(RELAY_PIN, LOW);
  releActivo = true;
  tiempoInicio = millis();
  Serial.println("Relé ACTIVADO - Luz encendida");
}

void desactivarRele() {
  digitalWrite(RELAY_PIN, HIGH);
  releActivo = false;
  Serial.println("Relé DESACTIVADO - Luz apagada");
  Serial.println("Sistema nuevamente en espera...");
}
