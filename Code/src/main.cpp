#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LiquidCrystal_I2C.h>
#include <AmperkaKB.h>
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
LiquidCrystal_I2C lcd(0x27, 16, 2);
AmperkaKB keypad(13, 12, 27, 26, 33, 32, 15, 2);
// AmperkaKB keypad(23, 25, 26, 27, 32, 33, 34, 35);

long getDistance(uint8_t trig, uint8_t echo);
void showPassword(String password);

void setup()
{
  memory.begin(EEPROM_ADDRESS);
  lcd.init();
  keypad.begin(KB4x4);

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

  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("Good Day");
}

void loop()
{
  // ArduinoOTA.handle();
  
  //Проверка растояния 
  if (getDistance(15, 4) < 60 || timer.timerIsWorking())   //Для экономии подсветки
  {
    if (getDistance(15, 4) < 60) 
    {
      timer.timerCheckAndStop();
      timer.timerStart(10);
    }
    
    lcd.backlight();

    // keypad.read();            //Получение значения с клавиатуры
    // if (keypad.justPressed())
    // {
    //   String password = "";        //Переменные лучше не объявлять в switch, иначе break может не сработать
    //   switch (keypad.getNum)
    //   {
    //     //Режим ввода ПИН-кода
    //     case 0:
    //     case 1:
    //     case 2:
    //     case 3:
    //     case 4:
    //     case 5:
    //     case 6:
    //     case 7:
    //     case 8:
    //     case 9:
    //       lcd.clear();
    //       lcd.setCursor(0,0);
    //       lcd.print("PIN");

    //       //Ввод кода
    //       lcd.setCursor(0,1);
    //       password = keypad.getChar;
    //       showPassword(password);

    //       while (password.length() > 0 || password.length() < 16)         //Режим ввода ПИН-кода будет, пока пользователь не введёт или сбросит пароль
    //       {
    //         keypad.read();                                                  //Получение нового значения, прошлое уже записано ранее
    //         if (keypad.justPressed()) 
    //         {
    //           showPassword(password);

    //           switch (keypad.getNum)
    //           {
    //             //ПИН-код может состоять только из чисел
    //             case 0:
    //             case 1:
    //             case 2:
    //             case 3:
    //             case 4:
    //             case 5:
    //             case 6:
    //             case 7:
    //             case 8:
    //             case 9:
    //               password += keypad.getChar;
    //               break;
    //             //Кнопка * - удаляет символ
    //             case 14:
    //               password.remove(password.length() - 1);    
    //               break;
    //             //Кнопка # - окончание ввода пароля
    //             case 15:
    //               ////////////////////////////
    //               password = "";   
    //               break;    
    //             //A B C D - сбрассывают пароль   
    //             default:
    //               password = "";

    //               break;
    //           }
    //         }
    //       }
          
    //       lcd.clear();
    //       lcd.setCursor(4, 0);
    //       lcd.print("Good Day");

    //       break;
    //     //Вход в меню
    //     case 10:
    //     case 11:
    //     case 12:
    //     case 13:
    //       lcd.clear();
    //       lcd.setCursor(0, 0);
    //       lcd.print("Menu");
    //     default:
    //       break;
    //   }
    // }
  }
  else {lcd.noBacklight();}
  /////Нужно добавить прерывние на открытие двери по нажатии кнопки выхода
}

long getDistance(uint8_t trig, uint8_t echo)
{
  long duration, cm;

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);

  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  //  Время задержки акустического сигнала на эхолокаторе.
  duration = pulseIn(echo, HIGH);

  // Теперь осталось преобразовать время в расстояние
  cm = (duration / 2) / 29.1;

  // Задержка между измерениями для корректной работы скеча
  delay(250);
  return cm;
}


void showPassword(String password)
{
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print(password);
}