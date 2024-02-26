#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define SS_PIN 5
#define RST_PIN 0
#define WIFI_SSID "비밀번호 12345678"
#define WIFI_PASSWORD "dongwook"
#define API_KEY "AIzaSyDtHBVBmcG3gcYxhhBXD0DceS8QVymgm8M"
#define DATABASE_URL "https://umbrella-system-6eeb4-default-rtdb.firebaseio.com"

MFRC522 RFID(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key KEY; 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
byte nuidPICC[4];

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  SPI.begin();
  RFID.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    KEY.keyByte[i] = 0xFF;
  }
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(KEY.keyByte, MFRC522::MF_KEY_SIZE);
  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
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
    Firebase.RTDB.setInt(&fbdo, "/ESP32", RFID.uid.uidByte[0]);
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
