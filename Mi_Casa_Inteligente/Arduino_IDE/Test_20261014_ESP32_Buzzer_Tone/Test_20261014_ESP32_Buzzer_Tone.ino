// Define el pin GPIO del ESP32-S3 conectado a la base del transistor (o al pin de señal del módulo)
const int buzzerPin = 4; 

void setup() {
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("Iniciando prueba de buzzer electromagnético...");
}

void loop() {
  // OPCIÓN A: Si tu buzzer es ACTIVO (pitido intermitente de encendido/apagado)
  //Serial.println("Buzzer ON");
  //digitalWrite(buzzerPin, HIGH);
  //delay(1000); // Suena durante 1 segundo
  
  //Serial.println("Buzzer OFF");
  //digitalWrite(buzzerPin, LOW);
  //delay(1000); // Silencio durante 1 segundo


   
  //  OPCIÓN B: Si tu buzzer es PASIVO (requiere tonos musicales o frecuencias)
  //  Descomenta las siguientes líneas y comenta la Opción A para probar frecuencias:
    
    // En ESP32 con Arduino IDE moderno se suele usar ledcWriteTone o tone()
    tone(buzzerPin, 2000); // Emite un tono de 2000 Hz
    delay(1000);
    noTone(buzzerPin);
    delay(1000);
  
}