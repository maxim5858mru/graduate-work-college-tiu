#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include "../EEPROM/EEPROM.h"
#include "../Keypad/Keypad.h"
#include "../RFID/RFID.h"
#include "../Fingerprint/Fingerprint.h"
#include "../Interface/Interface.h"

// Идентификаторы настроек
#define NeedSerial settings[0]
#define Buzzer settings[1]
#define WiFiAP settings[2]
#define ShowError settings[3]

// Идентификаторы флагов
#define SDWorking flags[0]
#define MDSNWorking flags[1]
#define SPIFFSWorking flags[2]

extern bool settings[8];
extern byte delayTime;                            
extern byte openTime;                            
extern int databaseCap;                             
extern String hostURL;                              
extern String databaseURL;                          
// extern String databaseLogin;
// extern String databasePass;
extern bool flags[3];

extern LiquidCrystal_I2C lcd;
extern EEPROM memory;
extern Keypad keypad;
extern RFID rfid;
extern FingerPrint fingerprint;

/* Блок работы с экраном загрузки */

/** Инициализация дисплея
 * @remarks дальнейшие изменения экрана загрузки выполнять с помощью writeLoadCent и setLoadFlag 
 */
void lcdInit();

/** Вывод процентов
 * @param cent - процент загрузки
 */
void writeLoadCent(int cent);

/** Установка флага состояния загрузки элемента
 * @remark по умолчанию все флаги не определены ("/"), после проверки и загрузки необходимо установить точный флаг +/-
 * @param numberFlag - номер флага
 * @param state - устанавливаемое состояние флага
 */
void setLoadFlag(uint8_t numberFlag, bool state);

/*** Блок работы с остальными компонентами ***/

// Инициализация компонентов
void componentsInit();

// Вывод резульатов самодиагностики
void showResultTest();

/*** Прерывания ***/

// Настройка прерывания
void setInterrupt();

// Прерывание на открытие двери при нажатии кнопки
void open();

#endif