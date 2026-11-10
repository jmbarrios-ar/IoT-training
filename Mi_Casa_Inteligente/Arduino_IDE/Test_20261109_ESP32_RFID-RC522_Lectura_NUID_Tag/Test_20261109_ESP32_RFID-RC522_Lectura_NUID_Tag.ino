#include <SPI.h>
#include <MFRC522.h>

// Pines del lector RC522
#define SS_PIN    5
#define RST_PIN   22
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==================================");
  Serial.println("  ESP32 + RFID RC522");
  Serial.println("  PRUEBA DE LECTURA UID / NUID");
  Serial.println("==================================");

  // Inicializar bus SPI con pines definidos
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);

  // Inicializar lector RFID
  rfid.PCD_Init();
  delay(100);

  // Comprobar comunicacion con RC522
  byte version = rfid.PCD_ReadRegister(
    MFRC522::VersionReg
  );

  Serial.print("Version RC522: 0x");
  if (version < 0x10) Serial.print("0");
  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: No se detecta el RC522.");
    Serial.println("Revisar cables, SPI y 3.3V.");
    return;
  }

  Serial.println("Lector inicializado.");
  Serial.println("Acerque una tarjeta o llavero...");
}

void loop() {

  // Verificar presencia de una nueva tarjeta
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }

  // Leer identificador
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println();
  Serial.println("------------------------------");
  Serial.println("TAG RFID DETECTADO");

  // Mostrar UID en hexadecimal
  Serial.print("UID HEX: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(":");
    }
  }

  Serial.println();

  // Mostrar UID en decimal
  Serial.print("UID DEC: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], DEC);

    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();

  // Longitud del UID
  Serial.print("Longitud UID: ");
  Serial.print(rfid.uid.size);
  Serial.println(" bytes");

  // Identificar tipo de tarjeta
  MFRC522::PICC_Type tipo =
    rfid.PICC_GetType(rfid.uid.sak);

  Serial.print("Tipo RFID: ");
  Serial.println(rfid.PICC_GetTypeName(tipo));

  Serial.println("------------------------------");
  Serial.println("Retire el TAG y acerque otro.");

  // Finalizar comunicacion con la tarjeta
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(700);
}
