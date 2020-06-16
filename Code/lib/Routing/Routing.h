#ifndef ROUTING_H
#define ROUTING_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;
extern AsyncWebServer server;
extern WiFiClient http;
extern String hostURL;
extern String databaseURL;

// Настройка таблицы маршрутизации
void setRouting();

// Маршрутизация
void handleNotFound(AsyncWebServerRequest *request);

#endif