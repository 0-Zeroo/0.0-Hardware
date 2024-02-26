#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "비밀번호 12345678"
#define WIFI_PASSWORD "dongwook"
#define API_KEY "AIzaSyDtHBVBmcG3gcYxhhBXD0DceS8QVymgm8M"
#define DATABASE_URL "https://umbrella-system-6eeb4-default-rtdb.firebaseio.com"
#define SS_PIN 5
#define RST_PIN 0
#define RFID_Code_Size 4
#define KEY_Size 6

MFRC522 RFID(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key KEY;
FirebaseData Fbdo;
FirebaseAuth Auth;
FirebaseConfig Config;
int RFID_Code[RFID_Code_Size];

void setup() {
    Serial.begin(115200);
    SPI.begin();
    RFID.PCD_Init();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    for (int i = 0; i < KEY_Size; i++) {
        KEY.keyByte[i] = 0xFF;
    }
    Serial.println(F("This code scan the MIFARE Classsic NUID."));
    Serial.print(F("Using the following key:"));
    //printHex(KEY.keyByte, MFRC522::MF_KEY_SIZE);

    Config.api_key = API_KEY;
    Config.database_url = DATABASE_URL;
    if (Firebase.signUp(&Config, &Auth, "", "")) {
        Serial.println("ok");
    }
    else {
        Serial.printf("%s\n", Config.signer.signupError.message.c_str());
    }
    Config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&Config, &Auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    if (!RFID.PICC_IsNewCardPresent() && !RFID.PICC_ReadCardSerial()) {
        return;
    }

    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = RFID.PICC_GetType(RFID.uid.sak);
    Serial.println(RFID.PICC_GetTypeName(piccType));

    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
    }

    if (RFID.uid.uidByte[0] != RFID_Code[0] || RFID.uid.uidByte[1] != RFID_Code[1] || RFID.uid.uidByte[2] != RFID_Code[2] || RFID.uid.uidByte[3] != RFID_Code[3] ) {
        Serial.println(F("A new card has been detected."));
        for (int i = 0; i < RFID_Code_Size; i++) {
            RFID_Code[i] = RFID.uid.uidByte[i];
        }
        Serial.println(F("The NUID tag is:"));
        /*
        Serial.print(F("In hex: "));
        printHex(RFID.uid.uidByte, RFID.uid.size);
        Serial.println();

        Serial.print(F("In dec: "));
        printDec(RFID.uid.uidByte, RFID.uid.size);
        Serial.println();
        */
        Firebase.RTDB.setInt(&Fbdo, "/ESP32", FirebaseString());
    }
    else Serial.println(F("Card read previously."));
    RFID.PICC_HaltA();
    RFID.PCD_StopCrypto1();
}

long long FirebaseString() {
    long long A;
    for (int i=0;i<RFID_Code_Size;i++){
        A*=1000;
        A+=RFID_Code[i];
    }
    return A;
}

/*
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
*/
