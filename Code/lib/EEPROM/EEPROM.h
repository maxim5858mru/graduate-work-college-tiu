#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <Arduino.h>
#include <Wire.h>

#define ERROR_CODE      0xFF                      // Код ошибки для EEPROM

class EEPROM
{
private:
    // Адрес памяти в шине IIC
    uint8_t _address;

    // bool wasERROR = false; 
    ////////////////Нужно подключить флаг ошибки, чтобы при работе с сервером не было проблем с выявлением ошибки
    ////////////////Для printmap нужно реализовать передачу ссылки на Serial

public:
    // Статус инициализации
    bool status = false;  

    // Флаг битого байта
    bool broken = false;

    /** Память подключаемая по IIC. Инициализация конструктором включает в себя инициализацию памяти (begin), но из-за особенностей компилятора, библиотека Wire (а значит и все функции этой библиотеки) работает только в основном теле кода.
     * @param address - адрес памяти в шине IIC, начинается с 0b1010XXX
     * @param clock - частота шины I2C [10k, 100k, 1M]
     */
    EEPROM(uint8_t address, int clock = 10000);

    /** Инициализация EEPROM
     * @param address - адрес памяти в шине IIC, начинается с 0b1010XXX
     * @param clock - частота шины I2C [10k, 100k, 1M]
     * @return результат попытки подключения к памяти
     */
    bool begin(uint8_t address, int clock = 10000);

    /** Запись одного байта (предназначена для других функций)
     * @param memoryAddress - двухбайтовый адрес для записи данных
     * @param data - записываемая информация
     * @param checkTryAgain - повторная попытка (x3) в случае неудачи
     * @return код окончания передачи, чтобы его можно было использовать в других функциях
     */
    int writebyte(int memoryAddress, byte data, bool checkTryAgain = true);

    /** Считывание байта
     * @param memoryAddress - двухбайтовый адрес для чтения данных
     * @param checkTryAgain - повторная попытка (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @param called - для вывода ошибки, чтобы можно было понять какая функция вызвала readbyte
     * @return значение считываемого байта, в случае ошибки вернёт код ошибки, которая определенна в директиве как ERROR_CODE
     */
    byte readbyte(int memoryAddress, bool checkTryAgain = true, bool showError = false, const char* called = NULL);

    /** Считывание бита. Функция (почти) является обвёрткой для readbyte
     * @param bitNumber - номер считываемого бита
     * @param memoryAddress - двухбайтовый адрес для чтения данных
     * @param checkTryAgain - повторная попытка (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return булевое значение бита
     */
    bool readbit(int bitNumber, int memoryAddress, bool checkTryAgain = true, bool showError = false);

    /** Считывание строки
     * @remarks Функция "самостоятельная" (не зависит и не использует другие функции)
     * @param length - длина считываемой строки
     * @param memoryAddress - двухбайтовый адрес начала строки, для её чтения
     * @param checkTryAgain - повторная попытка чтения (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return возвращает строку (к сожалению не удалось перейти на массив символов, из-за библиотеки Wire)
     */
    String readString(int length, int memoryAddress, bool checkTryAgain = true, bool showError = false);

    /** Запись байта, с предварительным считыванием (для экономии циклов перезаписи ячеек)
     * @param memoryAddress - двухбайтовый адрес для предварительного чтения и записи данных
     * @param data - записываемый байт
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return код окончания передачи данных
     */
    int updatebyte(int memoryAddress, byte data, bool checkTryAgain = true, bool showError = false);

    /** Запись бита, с предварительным чтением
     * @param memoryAddress - двухбайтовый адрес для предварительного чтения и записи бита (по факту изменённого байта)
     * @param value - булевое значение записываемого бита
     * @param bitNumber - номер считываемого бита
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return код окончания передачи данных
     */
    int updatebit(int memoryAddress, bool value, int bitNumber, bool checkTryAgain = true, bool showError = false);

    /** Запись строки
     * @param memoryAddress - двухбайтовый адрес начала записи строки
     * @param data - записываемая строка
     * @param endString - нужен ли символ окончания строки '\0'
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return код ошибки (если возникнет ошибка на каком-то этапе, после повторных попыток будет сделан return с последним кодом ошибки, в случае если ошибки не будет функция вернёт 0 (код успешного окончания передачи)
     */
    int updateString(int memoryAddress, String data, bool endString = true, bool checkTryAgain = true, bool showError = false);

    /** Вывод карты памяти, через Serial интерфейс. 
     * @warning Функция требует Serial (UART) интерфейс, если его не будет функция сделает выход.
     * @remarks Эта функция самая большая, и большую её часть занимает обработка служебных символов (чтобы не сломать весь вывод в Serial).
     * @param start - начало вывода карты памяти
     * @param size - размер карты (количество считываемых ячеек)
     * @param sizeLine - размер одной строчки
     * @param printText - параллельный вывод расшифровки байтов согласно кодировке  
     * @param checkTryAgain - повторная попытка чтения байта (x3) в случае неудачи
     */
    void printmap(int start, int size, int sizeLine, bool printText, bool checkTryAgain = true);
};

#endif
