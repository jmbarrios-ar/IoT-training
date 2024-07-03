//Importamos las librerias
#include <SPI.h>
#include <MFRC522.h>

// Definimos los pines del lector RFID
#define SS_PIN 10
#define RST_PIN 9

//Creamos instancia del objeto RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

int led = 2;

void setup() {
  // Definimos led como salida
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  //Habilitamos SPI para la comunicacion MOSI, MISO, SCK Y SS(SDA)
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  // put your main code here, to run repeatedly:

}
