#include <Arduino.h>
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Wire.begin();
}

void loop()
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 64);
  for (int i = 0; i < 0x3f; i++)
  {
    Serial.print(Wire.read(), HEX);
    Serial.print(" ");
  }
  Serial.println(Wire.endTransmission());  
  delay(1000);
}