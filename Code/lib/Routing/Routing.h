#ifndef ROUTING_H
#define ROUTING_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

//Маршрутизация

void handleNotFound(AsyncWebServerRequest *request);

#endif