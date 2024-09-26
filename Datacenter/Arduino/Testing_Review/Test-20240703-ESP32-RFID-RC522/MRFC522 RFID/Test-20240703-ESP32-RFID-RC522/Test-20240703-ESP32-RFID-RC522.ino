#include <SPI.h>
#include <MFRC522.h>
  
#define SS_PIN 5
#define RST_PIN 17
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
void setup() 
{
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
}
 
void loop() 
{
  // Procura por cartao RFID
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona o cartao RFID
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
 
  if (conteudo.substring(1) == "45 15 30 5B") //UID 1 - Cartao
  {
    Serial.println("Bem vindo!");
    Serial.println();
    digitalWrite(21, HIGH); // ativa rele, abre a trava solenoide
    delay(3000);           // espera 3 segundos
    digitalWrite(21, LOW);  // desativa rele, fecha a trava solenoide
  }  

  else {
    Serial.println("Tag nao cadastrada!");
    Serial.println();
    digitalWrite(22, HIGH); // ativa rele, abre a trava solenoide
    delay(3000);           // espera 3 segundos
    digitalWrite(22, LOW);  // desativa rele, fecha a trava solenoide
}

}