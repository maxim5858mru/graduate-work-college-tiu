#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include "../EEPROM/EEPROM.h"
#include "../Keypad/Keypad.h"
#include "../RFID/RFID.h"
#include "../Fingerprint/Fingerprint.h"
#include "../Clock/Clock.h"
#include "../Timer/Timer.h"

#define RELE0_PIN 26
#define RELE1_PIN 25
#define TONE_PIN  27

#define SDWorking flags[0]

extern bool flags[3];

extern Keypad keypad;
extern RFID rfid;
extern FingerPrint fingerprint;
extern LiquidCrystal_I2C lcd;
extern Clock RTC;
extern WiFiClient http;

namespace Interface
{
	/** Измерение расстояния до объекта, с помощью ультразвукового датчика
	 * @warning желательно чтобы объект меньше поглощал звуков
	 * @param trig - пин для отправки сигнала активации
	 * @param echo - пин для приёма ШИМ сигнала от датчика
	 * @return примерное расстояние до объекта
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
	/** Считывание пароля
	 * @param nextAuth - является ли вызов функции вторичной проверкой?
	 * @return введённый PIN в виде строки
	 */
	String readPassword(bool nextAuth = false);
	/** Проверка пароля
	 * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
	 * @remarks Открытие двери происходит внутри функции
	 * @param password - проверяемый пароль
	 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
	 */
	bool checkPassword(String password);

	/** Получение и проверка NUID карты RFID
	 * @remarks !!!!!Функция ещё не законченна, нужно реализовать загрузку базы с резервной копии
	 * @remarks Открытие двери происходит внутри функции
	 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
	 */
	bool checkAndGetRFID();
}
#endif