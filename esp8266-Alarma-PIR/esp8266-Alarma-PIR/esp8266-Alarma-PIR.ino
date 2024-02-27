const int LEDPin= D6;
//int inputPin = 7; // for Arduino microcontroller
const int PIRPin = D7; // for ESP8266 microcontroller
//int inputPin = 4; // for ESP32 microcontroller
 
void setup() 
{
  pinMode(PIRPin, INPUT);
  pinMode(LEDPin, OUTPUT);
  Serial.begin(9600);

  delay(60*1000);
}
 
void loop(){
  int value = digitalRead(PIRPin);
  if (val == HIGH) {
    digitalWrite(LEDPin, HIGH);
    delay(500);
    digitalWrite(LEDPin, LOW);
    delay(500);
    Serial.println("Motion detected!");
    }
  else {
    digitalWrite(LEDPin, LOW);
    Serial.println("No Motion detected!");
    }
    
delay(10);
}
				