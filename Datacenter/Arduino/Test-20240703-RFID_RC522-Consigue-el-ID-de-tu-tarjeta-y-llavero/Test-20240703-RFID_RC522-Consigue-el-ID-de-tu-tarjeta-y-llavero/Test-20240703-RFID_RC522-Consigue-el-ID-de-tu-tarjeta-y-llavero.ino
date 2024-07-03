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
  // Comprueba la presencia de una tarjeta
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Lee el ID de la tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Imprime el UID
  Serial.print("UID tag: ");

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  //Se enciende el led y se apaga
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);

  Serial.println();
  delay(1000);
  

}
