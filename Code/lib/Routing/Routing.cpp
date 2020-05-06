#include <Arduino.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>
#include "Routing.h"

//Маршрутизация

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(SD, "/NotFound.html", "text/html");
}