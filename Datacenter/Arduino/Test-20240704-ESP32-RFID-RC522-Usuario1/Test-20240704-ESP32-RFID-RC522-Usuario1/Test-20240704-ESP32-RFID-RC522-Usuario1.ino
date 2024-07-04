#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 15           // Configurable, see typical pin layout above
#define SS_PIN  21          // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
/**
 * Initialize.
 */
void setup() {
  Serial.begin(115200); // Initialize serial communications with the PC
  pinMode(2, OUTPUT);
  pinMode(22, OUTPUT);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Control de acceso:");
}

byte ActualUID[4];
byte Usuario1[4]= {0x6D, 0X0C, 0XEF, 0X2B};
//byte Usuario2[4]= {0xC1, 0X2F, 0XD6, 0X0E};  //en caso e requerir mas usuarios
void loop(){
  if ( ! mfrc522.PICC_IsNewCardPresent())
        {
          if ( mfrc522.PICC_ReadCardSerial())
          {
              Serial.print(F("Card UID:"));
              for (byte i = 0; i < mfrc522.uid.size; i++) {
                Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(mfrc522.uid.uidByte[i], HEX);
                ActualUID[i]=mfrc522.uid.uidByte[i];
              }
              Serial.print("    ");

              if(compareArray(ActualUID,Usuario1)) {
                digitalWrite(2, HIGH);
                digitalWrite(22, LOW);
                delay(3000);
                digitalWrite(2, LOW);
              }
              /*else if(compareArray(ActualUID,Usuario2))
                Serial.println("Acceso concedido...")*/
              else{
                digitalWrite(22, HIGH);
                digitalWrite(2, LOW);
                delay(3000);
                digitalWrite(22, LOW);
              }
              //Terminamos la lectura de la tarjeta actual
              mfrc522.PICC_HaltA();
          }
        }
}