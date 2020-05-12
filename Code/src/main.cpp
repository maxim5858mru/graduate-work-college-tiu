#include <Arduino.h>
#include <SD.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "../lib/IIC/IIC.h"
#include "../lib/Timer/Timer.h"
#include "../lib/Network/Network.h"
#include "../lib/Routing/Routing.h"
#include "../lib/Interface/Interface.h"

// Адреса памяти
#define EEPROM_ADDRESS_MDNS 385
#define EEPROM_SIZE_MDNS 10

// Индетификаторы настроек
#define NeedSerial settings[0]
#define Buzzer settings[1]
#define WiFiAP settings[2]
#define ShowError settings[3]

// Индетификаторы флагов
#define SDWorking flags[0]
#define MDSNWorking flags[1]

// Параметры загружаемые с EEPROM
bool settings[8] = {
  true,   // UART интерфейс
  true,   // Динамик
  true,   // Точка доступа
  true    // Вывод ошибок через UART
};

// Флаги состояния
bool flags[2] = {
  false,  // SD
  false   // MDNS
};

String host;

EEPROM memory(0x50, 10000);
Keypad keypad(0x20, KB4x4);
Clock RTC(0x68, false);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(14, 2);
Timer timer;
AsyncWebServer server(80);
WiFiClient http;

void setup()
{
  // Инициализция обязательных компонентов
  memory.begin(0x50);
  SPI.begin();  // Для сканера меток
  rfid.PCD_Init(); 
  lcd.init();
  keypad.begin();

  // Получение настроек с памяти
  if (memory.status) for (int i = 0; i < 8; i++) {settings[i] = memory.readbit(i, 0);}

  // Инициализация компонентов
  if (NeedSerial) {Serial.begin(115200);}         // UART
  if (Buzzer) {ledcSetup(0, 600, 8);}             // Настройка ШИМ для пьезодинамика
  if (SD.begin() && SD.cardType() != CARD_NONE && SD.cardType() != CARD_UNKNOWN) {SDWorking = true;}// Монтирование файловой системы
  else if (ShowError) {Serial.println("Error mounting SD");}

  // Настройка Wi-Fi
  if (memory.status) {Network::setupWiFi();}
  else {Network::presetupWiFi();}                                          // Создание точки доступа с предустановленными значениями

  // // Сихронизация часов
  RTC.begin();
  RTC.sync(http);

  // Настройка Web сервера
  if (SDWorking && WiFi.status() == WL_CONNECTED) 
  {
    if (memory.status)                            // Включение MDNS
    {
      host = memory.readString(EEPROM_SIZE_MDNS, EEPROM_ADDRESS_MDNS);
      if (!host.isEmpty() && MDNS.begin(host.c_str())) 
      {
        if (NeedSerial) {Serial.println("MDNS включён, локальный адресс: http://" + String(host) + ".local/");}
        MDSNWorking = true;
      }
    }

    // Настройка маршрутизации сервера

    // HTML
    server.onNotFound(handleNotFound);
    server.on("/", HTTP_GET, handleNotFound);                 

    // JS & CSS
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SD, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SD, "/script.js", "text/javascript");
    });

    // Используемые плагины
    server.on("/FontAwsome/font-awsome all.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SD, "/FontAwsome/font-awsome all.css", "text/css");
    });
    server.on("/FontAwsome/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SD, "/FontAwsome/fa-solid-900.woff2", "font/woff");
    });
    server.on("/FontAwsome/fa-solid-900.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SD, "/FontAwsome/fa-solid-900.svg", "image/svg+xml");
    });

    server.begin();
    if (MDSNWorking) {MDNS.addService("http", "tcp", 80);}

    // Обновление по сети
    ArduinoOTA.onStart([]                                                           
    {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {type = "sketch";}
      else {type = "filesystem";}
    });
    ArduinoOTA.begin();
  }

  lcd.backlight();
  Interface::goHome();
}

void loop()
{
  ArduinoOTA.handle();
  
  // Проверка растояния 
  if (Interface::getDistance(15, 4) < 60 || timer.timerIsWorking())        // Для экономии подсветки
  {
    // Таймер работает, когда никого нету по близости
    if (Interface::getDistance(15, 4) < 60) 
    {
      timer.timerCheckAndStop();
      timer.timerStart(10);
    }
    
    lcd.backlight();
    keypad.read();                                // Получение значения с клавиатуры
    if (keypad.state == ON_PRESS)
    {
      String password;                            // Переменные лучше не объявлять в switch, иначе break может не сработать после 1 условия
      bool stopReadPass = false;

      switch (keypad.Numb)
      {
        //Режим ввода ПИН-кода
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
          lcd.clear();
          lcd.print("PIN");

          // Ввод кода
          password = keypad.Char;
          Interface::showPassword(password, true);

          while (password.length() > 0 && password.length() < 16 && !stopReadPass)                  // Режим ввода ПИН-кода будет, пока пользователь не введёт или сбросит пароль
          {
            keypad.read();                                                 // Получение нового значения, прошлое уже записано ранее
            if (keypad.state == ON_PRESS) 
            {
              switch (keypad.Numb)
              {
                // ПИН-код может состоять только из чисел
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                  password += keypad.Char;
                  Interface::showPassword(password, true);
                  break;
                // Кнопка * - удаляет символ
                case 14:
                  password.remove(password.length() - 1);  
                  Interface::showPassword(password, false);  
                  break;
                // Кнопка # - окончание ввода пароля
                case 15:
                  stopReadPass = true;
                  break;    
                // A B C D - сбрассывают пароль   
                default:
                  password = "";
                  break;
              }
            }
          }
          
          Interface::checkPassword(password);
          Interface::goHome();

          break;
        //Вход в меню
        case 10:
        case 11:
        case 12:
        case 13:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Menu");
        default:
          break;
      }
    }

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {Interface::checkAndGetRFID();}   
  }
  else {lcd.noBacklight();}

  /////Нужно добавить прерывние на открытие двери по нажатии кнопки выхода
}