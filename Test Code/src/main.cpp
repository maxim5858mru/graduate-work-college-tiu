#include <Arduino.h>
#include "../lib/Fingerprint/Fingerprint.h"

/* 
Arduino 
    A4 - SDA
    A5 - SCL
ESP32
    D22 - SCL
    D21 - SDA
    D23 - MOSI
    D19 - MISO
    D18 - SCK
    D5  - CS (для SD) 
ESP8266
    D1 - SCL
    D2 - SDA
STM32F1
    B6 - SCL1
    B7 - SDA1
*/

FingerPrint fingerprint(&Serial1);

void setup() 
{
    Serial.begin(9600);
    while (!Serial) {}
    delay(100);

    if (fingerprint.init()) {
        Serial.println("Init OK");
    }
    else {
        Serial.println("Init Fall");
    }
}

void loop() 
{
    Serial.print("ID: 0x");
    Serial.println(fingerprint.read(), HEX);

}