#include <SPI.h>
#include <MFRC522.h>
//#define RST_PIN         9          // Configurable, see typical pin layout above
//#define SS_PIN          10         // Configurable, see typical pin layout above
#define RST_PIN D0          // Configurable, see typical pin layout above
#define SS_PIN D8         // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;
byte LecturaUID[4];
byte Usuario1[4]= {0xE2, 0x1F, 0XF2, 0XC5};  //Codigo UID Tarjeta
byte Usuario2[4]= {0xF4, 0x95, 0X9A, 0XEB};  //Codigo UID Llavero
//#define LED_PIN D1
void setup() {
	Serial.begin(115200);		// Initialize serial communications with the PC
  //pinMode(LED_PIN, OUTPUT);
	SPI.begin();			// Init SPI bus
	rfid.PCD_Init();
  Serial.println("Listo");
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent()) 
    return;
    // Seleccionar una tarjeta
  if ( ! rfid.PICC_ReadCardSerial())
    return;
    Serial.print("UID:");
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) {
        Serial.print(" 0");
        }
        else{
          Serial.print(" ");
          }
          Serial.print(rfid.uid.uidByte[i], HEX);
          LecturaUID[i]=rfid.uid.uidByte[i];
          }
          Serial.print("\t");

          if(comparaUID(LecturaUID, Usuario1))
            Serial.println("Bienvenido Usuario1");
            else if(comparaUID(LecturaUID, Usuario2))
            Serial.println("Bienvenido Usuario2");
            else
            Serial.println("No te conozco");

                rfid.PICC_HaltA();
}

boolean comparaUID(byte lectura[],byte usuario[])
{
  for (byte i=0; i < rfid.uid.size; i++){
    if(lectura[i] != usuario[i])
      return(false);
  }
  return(true);
}