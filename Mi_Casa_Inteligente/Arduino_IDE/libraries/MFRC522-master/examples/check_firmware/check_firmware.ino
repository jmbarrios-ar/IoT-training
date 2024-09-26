#include <MFRC522.h>
#include <SPI.h>

#define SAD 10
#define RST 5

MFRC522 nfc(SAD, RST);

void setup() {
  SPI.begin();
  Serial.begin(115200);

  Serial.println("Looking for MFRC522.");
  nfc.begin();

  uint8_t version = nfc.getFirmwareVersion();
  if (! version) {
    Serial.print("Didn't find MFRC522 board.");
    while(1); //halt
  }

  Serial.print("Found chip MFRC522 ");
  Serial.print("Firmware ver. 0x");
  Serial.print(version, HEX);
  Serial.println(".");
  
  if (nfc.digitalSelfTestPass()) {
      Serial.print("Digital self test by MFRC522 passed.");
  } else {
      Serial.print("Digital self test by MFRC522 failed.");
  }
}

void loop() {

}
