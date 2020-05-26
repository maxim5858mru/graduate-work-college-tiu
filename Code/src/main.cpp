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

// Адреса модулей в шине IIC
#define IIC_EEPROM 0x50
#define IIC_CLOCK  0x68
#define IIC_KEYPAD 0x20
#define IIC_LCD    0x27

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

// Выводы МК, к которым подключены реле, кнопки и пьезодинамик
#define RELE0_PIN   26
#define RELE1_PIN   25
#define BUTTON0_PIN 13
#define BUTTON1_PIN 12
#define TONE_PIN    27

// Параметры загружаемые с EEPROM
bool settings[8] = {
  true,                                           // UART интерфейс
  true,                                           // Динамик
  true,                                           // Точка доступа
  true                                            // Вывод ошибок через UART
};

// Флаги состояния
bool flags[2] = {
  false,                                          // SD
  false                                           // MDNS
};

String host;

EEPROM memory(IIC_EEPROM, 10000);                 // Память
Keypad keypad(IIC_KEYPAD, KB4x4);                 // Клавиатура
Clock RTC(IIC_CLOCK, false);                      // RTC часы
LiquidCrystal_I2C lcd(IIC_LCD, 16, 2);            // Дисплей
MFRC522 rfid(14, 2);                              // Считыватель RFID карт
AsyncWebServer server(80);                        // Веб сервер
Timer timer;                                      // Таймер
WiFiClient http;                                  // Клиент для различных обращений к другим серверам

void open()                                       // Прерывание на открытие двери при нажатии кнопки
{
  ESP.restart();                                  //!!!!! Чтобы написать нормальное прерывание необходимо отключить Watchdog для Arduino Core в ESP-IDF
}

void setup()
{
  // Инициализция обязательных компонентов
  memory.begin(IIC_EEPROM);
  SPI.begin();  // Для сканера меток
  rfid.PCD_Init(); 
  lcd.init();
  keypad.begin();

  // Получение настроек с памяти
  if (memory.status) for (int i = 0; i < 8; i++) {settings[i] = memory.readbit(i, 0);}

  // Инициализация компонентов
  if (NeedSerial) {Serial.begin(115200);}         // UART
  ledcSetup(0, 1000, 8);                          // Настройка ШИМ для пьезодинамика
  if (Buzzer) {ledcWrite(0, 200);}
  else {ledcWrite(0, 0);}            
  if (SD.begin() && SD.cardType() != CARD_NONE && SD.cardType() != CARD_UNKNOWN) {SDWorking = true;}// Монтирование файловой системы
  else if (ShowError) {Serial.println("Error mounting SD");}

  /*!!!!! Временно !!!!!*/
  /* Псевдо-прерывание */
  pinMode(BUTTON0_PIN, INPUT);                                             // Настройка выводов МК для прерывания
  // pinMode(BUTTON1_PIN, INPUT);
  pinMode(RELE0_PIN, OUTPUT);
  pinMode(RELE1_PIN, OUTPUT);
  digitalWrite(RELE0_PIN, HIGH);
  digitalWrite(RELE1_PIN, HIGH);
  if (digitalRead(BUTTON0_PIN) == LOW) {Interface::open(0);}               // Псевдо-проверка
  // else if (digitalRead(BUTTON1_PIN) == LOW) {Interface::open(1);}
  attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), open, FALLING);
  // attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), open, FALLING);

  // Настройка Wi-Fi
  if (memory.status) {Network::setupWiFi();}
  else {Network::presetupWiFi();}                                          // Создание точки доступа с предустановленными значениями

  // Сихронизация часов
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

  lcd.noBacklight();
  Interface::goHome();
}

void loop()
{
  ArduinoOTA.handle();
  
  // Проверка растояния 
  if (Interface::getDistance(15, 4) < 60 || timer.timerIsWorking())        // Для энергии на подсветке
  {
    // Таймер постоянно сбразывается, если человек стоит поблизости
    if (Interface::getDistance(15, 4) < 60) 
    {
      timer.timerCheckAndStop();
      timer.timerStart(10);
    }
    
    lcd.backlight();

    // Получение значения с клавиатуры
    keypad.read();                                
    if (keypad.state == ON_PRESS) switch (keypad.Numb)
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
        Interface::checkPassword(Interface::readPassword());
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

    // RFID
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {Interface::checkAndGetRFID();} // Если карта поднесена только что и считывание удалось, то выполняем проверку
  }
  else {lcd.noBacklight();}
}