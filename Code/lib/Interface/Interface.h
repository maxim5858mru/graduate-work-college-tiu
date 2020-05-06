#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
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

  /** Сброс экрана
   * @param lcd - ссылка на дисплей 
   */
  void goHome(LiquidCrystal_I2C &lcd);

  /** Вывод пароля (точнее его изменения) на дисплей
   * @param password - вводимый пароль
   * @param becomeMore - пароль увеличился или уменьшился?
   * @param lcd - ссылка на дисплей 
   */
  void showPassword(String password, bool becomeMore, LiquidCrystal_I2C &lcd);
  /** Проверка пароля
   * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
   * @remarks Открытие двери происходит внутри функции
   * @param password - проверяемый пароль
   * @param lcd - ссылка на дисплей
   * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
   */
  bool checkPassword(String password, LiquidCrystal_I2C &lcd);

  /** Получение ID (NUID) карты RFID
   * @warning из-за ограниченного размера переменной функуия совестима только с картами с размером 1кб
   * @param rfid - сылка на считыватель карт
   * @return NUID карты RFID
   */
  int getRFID(MFRC522 &rfid);
  /** Проверка NUID карты RFID
   * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных
   * @remarks Открытие двери происходит внутри функции
   * @param password - проверяемый NUID
   * @param lcd - ссылка на дисплей
   * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
   */
  bool checkRFID(int key, LiquidCrystal_I2C &lcd);
}
#endif