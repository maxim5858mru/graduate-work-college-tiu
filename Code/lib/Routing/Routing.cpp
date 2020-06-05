
#include "Routing.h"

// Маршрутизация
void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(SD, "/NotFound.html", "text/html");
}