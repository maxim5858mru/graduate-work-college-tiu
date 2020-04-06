#include "Arduino.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "../lib/IIC.h"

//Адрес памяти в шине IIC
#define EEPROM_ADDRESS 0b1010000
//Адреса памяти
#define EEPROM_ADDRESS_WiFi 1
#define EEPROM_ADDRESS_AP   289
#define EEPROM_ADDRESS_MDNS 385
//Стандартный SSID и пароль
#define DEFAULT_SSID      "AC001"
#define DEFAULT_PASSWORD  "10012019-C"

//Индетификаторы настроек
#define NeedSerial settings[0]
#define Buzzer settings[1]
#define WiFiAP settings[2]
#define ShowError settings[3]
//Индетификаторы флагов
#define SPIFFSWorking flags[0]

//Параметры загружаемые с EEPROM
bool settings[8] = {
  true,   //UART интерфейс
  true,   //Динамик
  true,   //Точка доступа
  true    //Вывод ошибок через UART
};
String ssid;
String password;
String host;
//Флаги состояния
bool flags[1] = {false};  //SPIFFS

EEPROM memory(EEPROM_ADDRESS, 10000);
AsyncWebServer server(80);

void setup()
{
  //Получение настроек с памяти
  if (memory.status) for (int i = 0; i < 8; i++) {settings[i] = memory.readbit(i, 0);}

  //Инициализация компонентов
  if (NeedSerial) {Serial.begin(112500);}         //UART
  if (Buzzer) {ledcSetup(0, 600, 8);}             //Настройка ШИМ для пьезодинамика
  if (SPIFFS.begin(true)) {SPIFFSWorking = true;} //Монтирование файловой системы
  else if (ShowError) {Serial.println("Error mounting SPIFFS");}

  //Настройка Wi-Fi
  if (memory.status) {setupWiFi;}
  else {presetupWiFi();}                            //Создание точки доступа с предустановленными значениями
}

void loop()
{

}

//Настройка Wi-Fi
void setupWiFi()
{
  //Сброс Wi-Fi модуля
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  //Получение SSID и пароля из памяти
  if (WiFiAP)                                       //Если в настройках указанно что нужна точка доступа
  {
    ssid = memory.readString(32, EEPROM_ADDRESS_AP);
    if (!ssid.isEmpty())                            //Если пусто, то ждём проверки на соединение (результат которой будет ложный)
    {
      password = memory.readString(64, EEPROM_ADDRESS_AP + 32);
      WiFi.softAP(ssid.c_str(), password.c_str());  //Создание точки доступа
      if (Serial && ShowError) {Serial.println("Attempt to create a network with ssid '" + ssid + "'.");}//Для отладки
    }
  }
  else for (int i = 0; i < 3; i++)                  //Перебор сохранённых значений
  {
    ssid = memory.readString(32, EEPROM_ADDRESS_WiFi + (96) * i);
    if (ssid.isEmpty()) {break;}
    password = memory.readString(64, EEPROM_ADDRESS_WiFi + 32 + (96) * i);
    WiFi.begin(ssid.c_str(), password.c_str());     //Попытка подключения
    if (Serial && ShowError) {Serial.println("Attempt to connect to " + ssid + " network.");}
    if (WiFi.status() == WL_CONNECTED) {return;}
  }

  if (WiFi.status() == WL_CONNECTED) {return;}
  else if (Serial && selectWiFi())                  //Сразу проверка на UART и выбор сети, если пользователь ничего не выберет, включается стандартная точка доступа
  {
    if (WiFiAP) 
    {
      WiFi.softAP(ssid.c_str(), password.c_str());        if (Serial && ShowError) {Serial.println("Attempt to create a network with ssid '" + ssid + "'.");}
    }
    else 
    {
      WiFi.begin(ssid.c_str(), password.c_str());
      if (Serial && ShowError) {Serial.println("Attempt to connect to " + ssid + " network.");}
    }
  }
  else {presetupWiFi();}
}

//Выбор Wi-Fi сети
bool selectWiFi()
{
  int numberOfNetworks = WiFi.scanNetworks();
  if (numberOfNetworks == 0) 
  {
    if (NeedSerial) {Serial.println("No networks available.\nEnable access point? (y/n)");}
    while(!Serial.available()){}
    if (Serial.readString() != "y")
    {
      WiFiAP = true;
      ///
    }
    else {return false;}
  }
  else
  {
    Serial.println("Number of networks found: " + String(numberOfNetworks));  
    String networks[numberOfNetworks][2];    //Массив нужен что бы правильно работал выбор сети по номеру 
    for (int i = 0; i < numberOfNetworks; i++)
    {
      networks[i][0] = WiFi.SSID(i);
      networks[i][1] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" открытая":" закрытая";

      //Вывод списка сетей
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(networks[i][0]);
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println(networks[i][1]);
      delay(10);
    }

    Serial.println("Select a WiFi network or enter 0 to enable an access point: ");
    while(!Serial.available()){}
    String temp = Serial.readString();

    if (temp = "-1")
    {
      ////
    }
    else if (temp = "0")
    {
      WiFiAP = true;
      ////
    }
    // else if () {}
    else
    {
      ////
    }
  }
}

//Создание точки доступа с предустановленными значениями 
void presetupWiFi()
{
  WiFiAP = true;
  ssid = DEFAULT_SSID;
  password = DEFAULT_PASSWORD;
  WiFi.softAP(ssid.c_str(), password.c_str());  //Создание точки доступа
  if (Serial && ShowError) {Serial.println("Attempt to create a network with ssid '" + ssid + "'.");}
}