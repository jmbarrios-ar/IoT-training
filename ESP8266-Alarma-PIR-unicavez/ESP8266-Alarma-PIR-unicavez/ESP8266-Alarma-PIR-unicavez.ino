// Si quisiéramos ejecutar una acción una única vez al detectar movimiento, 
//en lugar de todo el tiempo que la señal este activa, usaríamos el siguiente código.

const int LEDPin = D6;        // pin para el LED
const int PIRPin = D7;         // pin de entrada (for PIR sensor)

int pirState = LOW;           // de inicio no hay movimiento
int val = 0;                  // estado del pin

void setup() 
{
  pinMode(LEDPin, OUTPUT); 
  pinMode(PIRPin, INPUT);
  Serial.begin(115200);
}

void loop()
{
  val = digitalRead(PIRPin);
  if (val == HIGH)   //si está activado
  { 
    digitalWrite(LEDPin, HIGH);  //LED ON
    if (pirState == LOW)  //si previamente estaba apagado
    {
      Serial.println("Sensor activado");
      pirState = HIGH;
    }
  } 
  else   //si esta desactivado
  {
    digitalWrite(LEDPin, LOW); // LED OFF
    if (pirState == HIGH)  //si previamente estaba encendido
    {
      Serial.println("Sensor parado");
      pirState = LOW;
    }
  }
}