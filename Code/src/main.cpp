#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
// #include <ArduinoOTA.h>
#include "../lib/IIC/IIC.h"
#include "../lib/Timer/Timer.h"
#include "../lib/Network/Network.h"
#include "../lib/Routing/Routing.h"

//Адрес памяти в шине IIC
#define EEPROM_ADDRESS 0b1010000

//Адреса памяти
#define EEPROM_ADDRESS_MDNS 385
#define EEPROM_SIZE_MDNS 10

//Индетификаторы настроек
#define NeedSerial settings[0]
#define Buzzer settings[1]
#define WiFiAP settings[2]
#define ShowError settings[3]

//Индетификаторы флагов
#define SPIFFSWorking flags[0]
#define MDSNWorking flags[1]

//Параметры загружаемые с EEPROM
bool settings[8] = {
  true,   //UART интерфейс
  true,   //Динамик
  true,   //Точка доступа
  true    //Вывод ошибок через UART
};

//Флаги состояния
bool flags[2] = {
  false,  //SPIFFS
  false   //MDNS
};

String host;

EEPROM memory(EEPROM_ADDRESS, 10000);
Timer timer;
AsyncWebServer server(80);

void setup()
{
  memory.begin(EEPROM_ADDRESS);

  //Получение настроек с памяти
  if (memory.status) for (int i = 0; i < 8; i++) {settings[i] = memory.readbit(i, 0);}

  //Инициализация компонентов
  if (NeedSerial) {Serial.begin(115200);}         //UART
  if (Buzzer) {ledcSetup(0, 600, 8);}             //Настройка ШИМ для пьезодинамика
  if (SPIFFS.begin(true)) {SPIFFSWorking = true;} //Монтирование файловой системы
  else if (ShowError) {Serial.println("Error mounting SPIFFS");}

  //Настройка Wi-Fi
  if (memory.status) {Network::setupWiFi();}
  else {Network::presetupWiFi();}                            //Создание точки доступа с предустановленными значениями

  //Настройка Web сервера
  if (SPIFFSWorking && WiFi.status() == WL_CONNECTED) 
  {
    if (memory.status)                //Включение MDNS
    {
      host = memory.readString(EEPROM_SIZE_MDNS, EEPROM_ADDRESS_MDNS);
      if (!host.isEmpty() && MDNS.begin(host.c_str())) 
      {
        if (NeedSerial) {Serial.println("MDNS включён, локальный адресс: http://" + String(host) + ".local/");}
        MDSNWorking = true;
      }
    }

    //Настройка маршрутизации сервера

    //HTML
    server.onNotFound(handleNotFound);
    server.on("/", HTTP_GET, handleNotFound);                 

    //JS & CSS
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/script.js", "text/javascript");
    });

    //Используемые плагины
    server.on("/FontAwsome/font-awsome all.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/FontAwsome/font-awsome all.css", "text/css");
    });
    server.on("/FontAwsome/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/FontAwsome/fa-solid-900.woff2", "font/woff");
    });
    server.on("/FontAwsome/fa-solid-900.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/FontAwsome/fa-solid-900.svg", "image/svg+xml");
    });

    server.begin();
    if (MDSNWorking) {MDNS.addService("http", "tcp", 80);}

    /*Обновление по сети
    ArduinoOTA.onStart([]                                                           
    {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {type = "sketch";}
      else {type = "filesystem";}
      if (SPIFFSWorking) {SPIFFS.end();}
    });
    ArduinoOTA.begin();*/
  }
}

void loop()
{
  // ArduinoOTA.handle();
}