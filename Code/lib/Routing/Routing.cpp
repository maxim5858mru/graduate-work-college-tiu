#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "Routing.h"

//Маршрутизация

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(SPIFFS, "/NotFound.html", "text/html");
}