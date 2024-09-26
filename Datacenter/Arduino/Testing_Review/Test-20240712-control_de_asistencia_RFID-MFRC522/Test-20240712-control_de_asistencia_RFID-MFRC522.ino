#include <SPI.h>       // libreria comunicacion SPI
#include <MFRC522.h>   // libreria del Lector RFID
#include <Wire.h>    // libreria comunicacion I2C
#include <Time.h>     // libreria para el tiempo 
#include <DS1307RTC.h>   // libreria del reloj
#include <SD.h>       // libreria para la memoria SD
 

#define RST_PIN 9 // Configurable, pin 9 para Reset del MFRC522 
#define SS_PIN 8 // Configurable, pin 8 para el selector de chip del MFRC522

const char *Mes[13] = {" ",        // arreglo de meses 
  "Ene", "Feb", "Mar", "Abr", "May", "Jun",
  "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"
};

File myFile;   

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

String read_rfid;   // variable donde almacena la tarjeta leida 
String tag[4] = {"ab74e5b5", "707870a4", "a49839a5", "d7485d79"};   // tag Almacenados 
boolean P1=false ,P2= false ,P3=false ,P4=false;    // condiciones para los estados de entrada o salida 
String nombre[5] = { "Luis ", "Maria ", "Pedro ", "Sofia ", "Desconocido "};   // Usuarios registrados 

int pinSD = 10;    // pin selector para la SD 
int LedVerde=7;    // pin 7 para el led verde 

void setup() {
 pinMode(LedVerde,OUTPUT);    // pin 7 se declara como salida 
 Serial.begin(9600);        // se inicializa el puerto serial 
 SPI.begin();            // se inicializa la comunicacion SPI 
  mfrc522.PCD_Init();     // se inicia el modulo lector RFID 
  if (!SD.begin(pinSD)) {     // se compruba que este activa la tarjeta SD 
    Serial.println("initializacion fallida!");
    return;
  }
   Serial.println("initializacion lista.");

}


// funcion para leer los bloques de la tarjeta RFID 
void dump_byte_array(byte *buffer, byte bufferSize) {    
read_rfid="";
for (byte i = 0; i < bufferSize; i++) {
read_rfid=read_rfid + String(buffer[i], HEX);
}
}

void loop() {
  if(!mfrc522.PICC_IsNewCardPresent())   // comprueba si no existe alguna targeta presente en el lector 
  return;
  
  if(!mfrc522.PICC_ReadCardSerial())     // comprueba si no se ha leido la tarjeta 
  return;

  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);       // va a la funcion para leer el codigo de la tarjeta 
  registro();  // entra en el registro de evento 
  digitalWrite(LedVerde,LOW);   // apaga el led indicador 
}



// registro de evento
void registro()
{
  myFile = SD.open("registro.txt", FILE_WRITE);  // abre el archivo Registro donde se guardan los datos obtenidos 
  tmElements_t tm;      // reasiga nombre a variable 
  RTC.read(tm);     // lee los datos del reloj
  ingreso();      // va a la funcion ingreso para indicar su estado si es entrada o salida 
  Serial.print(",");  // separadores de datos 
  myFile.print(",");
  Personal();   // va a la funcion personal para identicar el tag leido 
  Serial.print(",");
  myFile.print(",");
  Serial.print("UID:");   
  //myFile.print("UID:");
  Serial.print(read_rfid);    // ID del tag leido 
  myFile.print(read_rfid);    // guarda el tag leido en la memoria SD 
  Serial.print(",");
  myFile.print(",");
//Registra Hora 
  Serial.print(" Hora:");
  //myFile.print(" Hora:");
    print2digits(tm.Hour);     
    Serial.write(':');
    myFile.print(":");
    print2digits(tm.Minute);
    Serial.write(':');
    myFile.print(":");
    print2digits(tm.Second);
    Serial.print(",");
    myFile.print(",");
// Registro de fecha 
    Serial.print(" Fecha(D/M/Y):");
    //myFile.print(" Fecha(D/M/Y):");
    Serial.print(tm.Day);
    myFile.print(tm.Day);
    Serial.write('/');
    myFile.print("/");
    Serial.print(Mes[tm.Month]);
    myFile.print(Mes[tm.Month]);
    Serial.write('/');
    myFile.print("/");
    Serial.print(tmYearToCalendar(tm.Year));
    myFile.print(tmYearToCalendar(tm.Year));
    Serial.println();
    myFile.println();// realiza un salto de linea en la memoria SD 
    myFile.close();     // cierra el archivo de la SD 
    digitalWrite(LedVerde,HIGH);   // enciende el led verde 
    delay(500);   // esperaa 1/2 segundo 
}

// le asiga un cero si el digito es mejor a 10 
void print2digits(int numero) {
  if (numero >= 0 && numero < 10) {  // si el digito es menor de 10 y mayor o igual a cero 
    Serial.write('0');     // coloca un cero 
    myFile.print("0");    
  }
  Serial.print(numero);   
  myFile.print(numero);
}


// identificacion de personal
void Personal(){
  if(tag[0] == read_rfid){      // tag de Persona 1 
    Serial.print(nombre[0]);
    myFile.print(nombre[0]);   
  }
    if(tag[1] == read_rfid){    // tag de persons 2 
    Serial.print(nombre[1]);
    myFile.print(nombre[1]);
  }
    if(tag[2] == read_rfid){     // tag de persona 3 
    Serial.print(nombre[2]);
    myFile.print(nombre[2]);
  }
    if(tag[3] == read_rfid){   //  tag de persons 4
    Serial.print(nombre[3]);
    myFile.print(nombre[3]);
  }
    if( tag[0] != read_rfid && tag[1] != read_rfid && tag[2] != read_rfid && tag[3] != read_rfid){    // si el tag no esta Registrado 
    Serial.print(nombre[4]);
    myFile.print(nombre[4]);
  }
}


// comprobacion de estado del ingreso 
void ingreso(){
  bool x = false;   // condicion inicial 
    if(tag[0] == read_rfid && P1 == false && x==false){  // condicion para entrada de persona 1 
      Serial.print("Entrada ");
      myFile.print("Entrada ");
      P1 = true;
      x=true;
    }
     if(tag[0] == read_rfid && P1 == true  && x==false){   // condicion para salida de persona 1 
      Serial.print("Salida ");
      myFile.print("Salida ");
      P1 = false;
      x=true;
    }
      if(tag[1] == read_rfid && P2 == false && x==false){    // condicion para entrada de persona 2
      Serial.print("Entrada ");
      myFile.print("Entrada ");
      P2 = true;
      x=true; 
    }
      if(tag[1] == read_rfid && P2 == true && x==false){   // condicion para salida de persona 2 
      Serial.print("Salida ");
      myFile.print("Salida ");
      P2 = false;
      x=true; 
    }
      if(tag[2] == read_rfid && P3 == false && x==false){   // condicion para salida de persona 3 
      Serial.print("Entrada ");
      myFile.print("Entrada ");
      P3 = true;
      x=true;
    }
      if(tag[2] == read_rfid && P3 == true && x==false){   // condicion para salida de persona 3 
      Serial.print("Salida ");
      myFile.print("Sallida ");
      P3 = false;
      x=true; 
    }
      if(tag[3] == read_rfid && P4== false && x==false){    // condicion para salida de persona 4 
      Serial.print("Entrada ");
      myFile.print("Entrada ");
      P4 = true;
      x=true;
    }
      if(tag[3] == read_rfid && P4 == true && x==false){     // condicion para salida de persona 4 
      Serial.print("Salida ");
      myFile.print("Salida ");
      P4 = false;
      x=true;
    }
  
}
