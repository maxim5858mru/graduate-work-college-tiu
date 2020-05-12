#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include "../IIC/IIC.h"
#include "../Timer/Timer.h"

namespace Interface
{
  /** Измерение растояния до объекта, с помощью ультразвукового датчика
  * @warning желательно чтобы объект меньше поглощал звуков
  * @param trig - пин для отправки сигнала активации
  * @param echo - пин для приёма ШИМ сигнала от датчика
  * @return примерное растояние до объекта
  */
  long getDistance(uint8_t trig, uint8_t echo);

  // Сброс экрана
  void goHome();
  /** Открытие двери
   * @param door - открываемая дверь
   */
  void open(uint8_t door);
  /** Отказ в доступе
   * @param byTime - причиной отказа является время?
   */
  void accessDeny(bool byTime);

  /** Вывод пароля (точнее его изменения) на дисплей
   * @param password - вводимый пароль
   * @param becomeMore - пароль увеличился или уменьшился?
   */
  void showPassword(String password, bool becomeMore);
  /** Проверка пароля
   * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
   * @remarks Открытие двери происходит внутри функции
   * @param password - проверяемый пароль
   * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
   */
  bool checkPassword(String password);

  /** Получение и проверка NUID карты RFID
   * @remarks !!!!!Функция ещё не законченна, нужно реализовать щагрузку базы с резервной копии
   * @remarks Открытие двери происходит внутри функции
   * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
   */
  bool checkAndGetRFID();
}
#endif