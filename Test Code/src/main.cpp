#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "../lib/IIC/IIC.h"

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

extern int __bss_end;
extern void *__brkval;

Clock clockRTC(0b1101000, false);
WiFiClient temp;
 
// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree()
{
   int freeValue;
   if((int)__brkval == 0)
      freeValue = ((int)&freeValue) - ((int)&__bss_end);
   else
      freeValue = ((int)&freeValue) - ((int)__brkval);
   return freeValue;
}

int serialReadInt(String say)
{
    Serial.println(say);
    while(!Serial.available()) {}
    return Serial.readString().toInt();
}

void setup() 
{
  Serial.begin(115200);
  while (!Serial) ;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin("Eltex", "123456789_M");
  delay(2000);

  clockRTC.begin();
  clockRTC.read(true, true);
  String str = clockRTC.readRAM(true, true);
  for (int i = 0; i < str.length(); i++) {Serial.print(str[i], HEX); Serial.print(" ");}
  Serial.println("\n");
  clockRTC.sync(temp);
}

void loop() 
{
  Serial.println("");
  Serial.println("Seconds: " + (String)second());
  Serial.println("Minutes: " + (String)minute());
  Serial.println("Hours: " + (String)hour());
  Serial.println("Day: " + (String)day());
  Serial.println("Month: " + (String)month());
  Serial.println("Year: " + (String)year());
  delay(1000);
//   if (clockRTC.compare(serialReadInt("Start Hours: "), serialReadInt("Start Min: "), serialReadInt("Stop TiHour:"), serialReadInt("Stop Min: "))) Serial.println("True \n");
}