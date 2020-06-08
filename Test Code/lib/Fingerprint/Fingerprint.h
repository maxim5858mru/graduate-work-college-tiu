#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Fingerprint.h>

class FingerPrint : Adafruit_Fingerprint
{
private:
public:
    bool status;

    /** Конструкторы для сканера отпечатков
     * @param _Serial - ссылка на объект монитора 
     * @param password - пароль для включения сканера отпечатков пальцев
     */
    FingerPrint(HardwareSerial* _Serial, uint32_t password = 0x00) : Adafruit_Fingerprint(_Serial, password) {};
    FingerPrint(SoftwareSerial* _Serial, uint32_t password = 0x00) : Adafruit_Fingerprint(_Serial, password) {};

    /** Инициализация сканера отпечатков пальцев
     * @param baud - скорость обмена
     * @return результат авторизации 
     */
    bool init(uint32_t baud = 57600);

    /** Полный процесс получения номера отпечатка пальцев
     * @return номер отпечатка, если нету или не прошёл проверку, то возвращается 0xFFFF
     */
    uint16_t read();
    // bool enroll();
    // bool empty();
    // bool deleteTempl();
};


#endif