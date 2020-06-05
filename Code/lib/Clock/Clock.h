#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

class Clock
{
private:
    // Адрес часов в шине I2C
    uint8_t _address;

    /** Перевод числа из 10 в 16 систему счисления и обратно
     * @param numb - число
     * @param toBCD - True из DEC в HEX, False из HEX в DEC
     */
    int _transNumber(int numb, bool toBCD);

public:
    // Вкл./ Выкл.
    bool working = false;
    // Статус инициализации 
    bool status = false;
    // Флаг отображающий была ли ошибка
    bool wasError = false;

    // Вывод прямоугольного сигнала для реализации прерывания
    bool SQWE = false;
    // Выводимый сигнал на случай отключения SQWE
    bool OUT = false;
    // Кварцевый резонатор
    byte quarts = 0b11;
    // 24-часовой режим. True - 12 часовой ражим, False - 24
    bool mode = false;

    /** Часы подключаемые по шине I2C
     * @param address - адрес устройства в шине I2C
     * @param check - выполнение проверки подключения при инициализации, вне основного кода не сработает 
     */
    Clock(int address = 0b110100, bool check = false);

    /** Инициализация часов
     * @return true - если не было ошибки
     */
    bool begin();

    /** Считывание всех данных (кроме ОЗУ) часов
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return true - если код окончания равен 0 (все хорошо)
     */
    bool read(bool checkTryAgain = true, bool showError = false);
    /** Считывание ОЗУ часов
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return строку из 64 символов
     */
    String readRAM(bool checkTryAgain = true, bool showError = false);

    /** Синхронизация часов
     * @param http - ссылка на объект "клиента"
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return true - если не было ошибки
     */
    bool sync(WiFiClient &http, bool checkTryAgain = true, bool showError = false);
    /** Проверка на попадание в диапазон времени
     * @warning если часы сбиты, то пропускаем проверку
     * @param sHours - часы первого диапазона
     * @param sMin - минуты первого диапазона
     * @param eHours - часы второго диапазона
     * @param eMin - минуты второго диапазона
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     */
    bool compare(byte sHours, byte sMin, byte eHours, byte eMin, bool checkTryAgain = true, bool showError = false);

    /** Запись данных в часы
     * @param numberByte - номер записываемого байта. Для записи всех байтов -1. 
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return true - если код окончания равен 0 (все хорошо)
     */
    bool write(byte numberByte, bool checkTryAgain = true, bool showError = false);
    /** Запись байта данных в RTC часы
     * @param numberByte - номер записываемого байта 
     * @param data - записываемый байт
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     */
    bool write(byte numberByte, byte data, bool checkTryAgain = true, bool showError = false);
    /** Запись всех данных в RTC часы с внутренних часов
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     */
    bool write(bool checkTryAgain = true, bool showError = false);
};

#endif
