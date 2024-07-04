#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5   // Pin de selección de esclavo (SDA)
#define RST_PIN 27 // Pin de reset

MFRC522 mfrc522(SS_PIN, RST_PIN); // Crear una instancia del objeto MFRC522

void setup() {
  Serial.begin(115200); // Inicializar la comunicación serie
  SPI.begin();          // Inicializar la interfaz SPI
  mfrc522.PCD_Init();   // Inicializar el lector MFRC522
  Serial.println("Lector RFID listo. Por favor, acerque una tarjeta.");
}

void loop() {
  // Revisar si hay una nueva tarjeta presente
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Seleccionar una tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Mostrar el UID de la tarjeta
  Serial.print("UID de la tarjeta: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Poner la tarjeta en modo espera
  mfrc522.PICC_HaltA();
}

