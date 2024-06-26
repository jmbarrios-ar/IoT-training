const int LEDPin= D6;
//int inputPin = 7; // for Arduino microcontroller
const int PIRPin = D7; // for ESP8266 microcontroller
//int inputPin = 4; // for ESP32 microcontroller
int val = LOW;
 
void setup() 
{
  //pinMode(PIRPin, INPUT_PULLUP);
  pinMode(PIRPin, INPUT);
  pinMode(LEDPin, OUTPUT);
  Serial.begin(115200);

  delay(10*1000);
}
 
void loop(){
  int val = digitalRead(PIRPin);
  if (val == HIGH) {
    Serial.println("Movimiento detectado!");
    digitalWrite(LEDPin, HIGH);
    delay(1000);
    digitalWrite(LEDPin, LOW);
    int val = LOW;
    delay(2000);
    }
  else {
    digitalWrite(LEDPin, LOW);
    Serial.println("No hay movimientos");
    }
    
delay(1000);
}
				