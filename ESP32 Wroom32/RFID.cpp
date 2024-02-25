#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 0
 
MFRC522 RFID(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key KEY; 

byte nuidPICC[4];

void setup() {
  Serial.begin(115200);
  SPI.begin();
  RFID.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    KEY.keyByte[i] = 0xFF;
  }
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(KEY.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {
  if ( ! RFID.PICC_IsNewCardPresent() &&! RFID.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = RFID.PICC_GetType(RFID.uid.sak);
  Serial.println(RFID.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  
  if (RFID.uid.uidByte[0] != nuidPICC[0] || RFID.uid.uidByte[1] != nuidPICC[1] || RFID.uid.uidByte[2] != nuidPICC[2] || RFID.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = RFID.uid.uidByte[i];
    }
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(RFID.uid.uidByte, RFID.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(RFID.uid.uidByte, RFID.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  RFID.PICC_HaltA();
  RFID.PCD_StopCrypto1();
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}
