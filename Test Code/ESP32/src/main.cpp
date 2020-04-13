/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "Arduino.h"
#include "../lib/WPS/WPS.h"

void setup()
{
    Serial.begin(115200);
    WPS::start();
    Serial.println("Starting WPS");
}

void loop()
{

}