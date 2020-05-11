/*Typical pin layout used:
   -----------------------------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino     ESP32
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro   30-pinout
   Signal      Pin          Pin           Pin       Pin        Pin              Pin         (in this program)
   -----------------------------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST         22
   SPI SS      SDA(SS)      10            53        D10        10               10          21
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16          23
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14          19
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15          18
*/

#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>    // Library for Mifare RC522 Devices
#include <EEPROM.h>     // Library for Using EEPROM or Flash Memory

#define relay 13        // Set Relay Pin
#define EEPROM_SIZE 4   // Size flash memory that we are using (4 because we only store 1 RFID UID)

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module

int Result; //for the result of compare

// Create MFRC522 instance.
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  //Arduino Pin Configuration
  pinMode(relay, OUTPUT);
  //Be careful how relay circuit behave on while resetting or power-cycling your Arduino
  digitalWrite(relay, HIGH);    // Make sure door is locked
  
  //Protocol Configuration
  Serial.begin(9600);  // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  
  if (!EEPROM.begin(EEPROM_SIZE)) // Initialize EEPROM
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  
  Serial.println(F("PKE System v0.6"));   // For debugging purposes
  //added EEPROM
  //using UID in EEPROM
  //remove showing detail of RC522
  //still no security, sorry :'(
  
  //use here if you want to input NEW UID of the card
  /*storedCard[0]=0xB7;
  storedCard[1]=0xC4;
  storedCard[2]=0x60;
  storedCard[3]=0x62;
  for(int i = 0; i<4;i++){    // saving the stored Card to EEPROM
    EEPROM.write(i, storedCard[i]);
  }*/
  for ( uint8_t i = 0; i < 4; i++ ) {          //move Data from EEPROM to function
    EEPROM.commit();
    storedCard[i] = EEPROM.read(i);
  }
  
  Serial.println();
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting RFID Card to be scanned"));
}

void loop() {
  digitalWrite(relay, HIGH);
  successRead = getID();
  while(successRead){       //this will keep looping if there any card near the RFID Reader
      Result = compare();   //check the card is the ID card the same with the stored card
      if (Result == 1){
        Serial.println("Authorized access");
        do{                 //this will keep looping if the TRUE card still near the RFID Reader
          digitalWrite(relay, LOW);
          delay(1000);
          successRead = getID();
          Result = compare();
        }while(Result && successRead);
      }
      else{
        Serial.println(" Access denied");
      }
      successRead = getID();
  }
  Serial.print(".");
}


///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("\nScanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println();
  return 1;
}

////////////////////////////////////////////compare READ from STORED/////////////////////////////
int compare (){
  for (int i=0;i<4;i++){
    if (storedCard[i]==readCard[i]){}
    else{
      return 0;
    }
  }
  return 1;
}
