#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
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
Keypad keypad(0x21, KB4x4);
MFRC522 rfid(14, 2);
Timer timer;
AsyncWebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);

long getDistance(uint8_t trig, uint8_t echo);
void showPassword(String password, bool addOrDel);
bool checkPassword(String password);
int getRFID(MFRC522 rfid);
bool checkRFID(int key);

void setup()
{
  memory.begin(EEPROM_ADDRESS);
  SPI.begin();
  rfid.PCD_Init(); 
  lcd.init();
  keypad.begin();

  //Получение настроек с памяти
  if (memory.status) for (int i = 0; i < 8; i++) {settings[i] = memory.readbit(i, 0);}

  //Инициализация компонентов
  if (NeedSerial) {Serial.begin(115200);}         //UART
  if (Buzzer) {ledcSetup(0, 600, 8);}             //Настройка ШИМ для пьезодинамика
  if (SPIFFS.begin(true)) {SPIFFSWorking = true;} //Монтирование файловой системы
  else if (ShowError) {Serial.println("Error mounting SPIFFS");}

  //Настройка Wi-Fi
  if (memory.status) {Network::setupWiFi();}
  else {Network::presetupWiFi();}                                          //Создание точки доступа с предустановленными значениями

  //Настройка Web сервера
  if (SPIFFSWorking && WiFi.status() == WL_CONNECTED) 
  {
    if (memory.status)                            //Включение MDNS
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
  if (getDistance(15, 4) < 60 || timer.timerIsWorking())                   // Для экономии подсветки
  {
    if (getDistance(15, 4) < 60) 
    {
      timer.timerCheckAndStop();
      timer.timerStart(10);
    }
    
    lcd.backlight();

    keypad.read();                                // Получение значения с клавиатуры
    if (keypad.state == ON_PRESS)
    {
      String password = "";                       // Переменные лучше не объявлять в switch, иначе break может не сработать после 1 условия
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
          // lcd.setCursor(0,0);
          lcd.print("PIN");

          //Ввод кода
          password = keypad.Char;
          showPassword(password, true);

          while (password.length() > 0 && password.length() < 16 && !stopReadPass)          //Режим ввода ПИН-кода будет, пока пользователь не введёт или сбросит пароль
          {
            keypad.read();                                                 //Получение нового значения, прошлое уже записано ранее
            if (keypad.state == ON_PRESS) 
            {
              switch (keypad.Numb)
              {
                //ПИН-код может состоять только из чисел
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
                  showPassword(password, true);
                  break;
                //Кнопка * - удаляет символ
                case 14:
                  password.remove(password.length() - 1);  
                  showPassword(password, false);  
                  break;
                //Кнопка # - окончание ввода пароля
                case 15:
                  stopReadPass = true;
                  break;    
                //A B C D - сбрассывают пароль   
                default:
                  password = "";
                  break;
              }
            }
          }
          
          if (checkPassword(password)) 
          {
            /////////////////

          }

          lcd.clear();
          lcd.setCursor(4, 0);
          lcd.print("Good Day");

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

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) checkRFID(getRFID(rfid));    
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


void showPassword(String password, bool addOrDel)
{
  if (addOrDel)
  {
    lcd.setCursor(password.length() - 1, 1);
    lcd.print(password[password.length() - 1]);
    delay(250);

    lcd.setCursor(password.length() - 1, 1);
    lcd.print("*");
  }
  else
  {
    lcd.setCursor(password.length(), 1);
    lcd.print(" ");
  }
}

bool checkPassword(String password)
{
  lcd.clear();

  if (password.length() != 0) 
  {
    lcd.print("Open");
    delay(2000);

    return true;
  }
  else 
  {
    lcd.print("Wrong password");
    delay(2000);

    return false;
  }
}

int getRFID(MFRC522 rfid)
{
  int key = 0;

  for (int i = 0; i < 4; i++)
  {
    key |= rfid.uid.uidByte[i] << ((3-i)*8);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return key;
}

bool checkRFID(int key)
{
  lcd.clear();

  if (key == 0xE98F8499)
  {
    lcd.print("Open");
    delay(2000);

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Good Day");

    return true;
  }
  else
  {
    lcd.print("Access Deny");
    delay(2000);

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Good Day");

    return false;
  }
}