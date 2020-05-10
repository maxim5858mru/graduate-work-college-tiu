#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include "../IIC/IIC.h"
#include "../Timer/Timer.h"
#include "Interface.h"

extern LiquidCrystal_I2C lcd;
extern MFRC522 rfid;
// extern tmElements_t nowTime;

/** Измерение растояния до объекта, с помощью ультразвукового датчика
 * @warning желательно чтобы объект меньше поглощал звуков
 * @param trig - пин для отправки сигнала активации
 * @param echo - пин для приёма ШИМ сигнала от датчика
 * @return примерное растояние до объекта
 */
long Interface::getDistance(uint8_t trig, uint8_t echo)
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

// Сброс экрана
void Interface::goHome()
{          
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Good Day");
}

/** Открытие двери
 * @param door - открываемая дверь
 */
void Interface::open(uint8_t door)
{
  lcd.clear();
  lcd.print("Open");
  delay(2000);

  goHome();
}

/** Отказ в доступе
 * @param - причиной отказа является время?
 */
void Interface::accessDeny(bool byTime)
{
  lcd.clear();
  if (byTime) {lcd.print("Wrong Time");}
  else {lcd.print("Access Deny");}
  delay(2000);

  goHome();
}

/** Вывод пароля (точнее его изменения) на дисплей
 * @param password - вводимый пароль
 * @param becomeMore - пароль увеличился или уменьшился?
 */
void Interface::showPassword(String password, bool becomeMore)
{
  if (becomeMore)
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

/** Проверка пароля
 * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
 * @remarks Открытие двери происходит внутри функции
 * @param password - проверяемый пароль
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkPassword(String password)
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

/** Получение и проверка NUID карты RFID
 * @remarks !!!!!Функция ещё не законченна, нужно реализовать щагрузку базы с резервной копии
 * @remarks ↓↓↓ Алгоритм ↓↓↓
 * открой папку
 * это не папка или она вообще не существует -> скачивай базу данных
 * открой следующий в папке (первый) файл
 * цикл перебора файлов
 *   буффер и переменные JSON
 *   в записи нет RFID авторизации -> новый файл
 *     не прошёл проверку массива NUID -> открой новый файл и continue
 *     не попал по времени -> вывод ошибки по времени и return
 *     есть еще тербования (методы) -> проверяй. Не получилось? -> вывод ошибки и return
 *     открой дверь и return  
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkAndGetRFID()
{
  byte RFID[10];
  for (int i = 0; i < 10; i++) {RFID[i] = rfid.uid.uidByte[i];}

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(100);

  // Открытие папки базы данных, точнее папки с её записями
  File folder = SD.open("/Database");
  if(!folder || !folder.isDirectory()){           // На случай если папку удалят
    // !!!!! Скачиваем базу данных
  }

  // Перебор записей. К сожалению, всю базу не запихнуть в буффер, по этому перебор записей пользователей
  File file = folder.openNextFile();
  while(file)
  {
    DynamicJsonDocument json(600);                // Буффер для файла. Благодаря ему я использую этот тупой костыль-метод
    deserializeJson(json, file);

    // Объявление переменных для файла
    JsonArray JMethod = json["Method"]; 
    if(JMethod[2])                                // Массив Method указывает, какими методами авторизации должен воспользоваться пользователь
    {
      bool result = true;
      JsonArray JRFID = json["RFID"];             // NUID RFID карты состоит из 10 байтов

      for(int i = 0; i < 10; i++)
      {
        if(JRFID[i] != RFID[i])
        {
          file = folder.openNextFile();           // Каждая запись пользователя в отдельном файле, спасибо буффер...
          result = false;
          break;
        }
      }

      if (!result) {continue;}                    // Костыль для реализации break label цикла перебора

      //  Проверка на время, но тут в случае ошибки accessDeny
      // if (!checkTime(json["Start Time"], json["End Time"])) 
      // {
      //   accessDeny(true);
      //   return false;
      // }

      // !!!!! Проверка на доп. требования
      // !!!!! .. .. ..

      open(json["Door"]);
      return true;
    }  
    else {file = folder.openNextFile(); }         
  }

  accessDeny(false);
  return false;
}

/** Проверка на попадание в диапазон временни
 * @param !!!!! Нужно добавить сихронизацию времени
 * @warning если часы сбиты, то пропускаем проверку
 * @param sTime - начальное время
 * @param eTime - конечное время
 * @return истина если текущее время подходит под диапазон
 */
bool Interface::checkTime(int sTime, int eTime)
{
  int nTime;
  tmElements_t nowTime;                                

  // if(!RTC.read(nowTime)) {return true;}            // Если часы ненастроенны ... открываем дверь
  nTime = nowTime.Hour * 100 + nowTime.Minute;        // Переводим время в удобный формат

  if(sTime < eTime)                             // Проверка на "ночную смену"
  {
    if(sTime <= nTime && eTime >= nTime) {return true;}
  }
  else if((sTime >= nTime) == (eTime >= nTime)) {return true;}

  return false;
}