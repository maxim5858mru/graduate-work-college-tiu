#ifndef ROUTING_H
#define ROUTING_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

// Настройка таблицы маршрутизации
void setRouting();

// Маршрутизация
void handleNotFound(AsyncWebServerRequest *request);

#endif