// Definir los pines utilizados
const int pinPulsador = 13; // Pin donde está conectado el pulsador
const int pinLed = 12;      // Pin donde está conectado el LED

// Variables para almacenar el estado del pulsador y el estado actual del LED
int estadoPulsador = 0;
int estadoLed = LOW;

// Variable para detectar cambios en el estado del pulsador
int ultimoEstadoPulsador = LOW;

void setup() {
  // Configuración del pin del pulsador como entrada
  pinMode(pinPulsador, INPUT);
  
  // Configuración del pin del LED como salida
  pinMode(pinLed, OUTPUT);
  
  // Inicializar el LED apagado
  digitalWrite(pinLed, estadoLed);
}

void loop() {
  // Leer el estado actual del pulsador
  estadoPulsador = digitalRead(pinPulsador);
  
  // Verificar si el estado del pulsador ha cambiado (de LOW a HIGH)
  if (estadoPulsador == HIGH && ultimoEstadoPulsador == LOW) {
    // Cambiar el estado del LED
    estadoLed = !estadoLed;
    
    // Escribir el nuevo estado del LED
    digitalWrite(pinLed, estadoLed);
    
    // Pequeño retardo para evitar el rebote del botón
    delay(100);
  }

  // Guardar el estado actual del pulsador para la próxima iteración
  ultimoEstadoPulsador = estadoPulsador;
}