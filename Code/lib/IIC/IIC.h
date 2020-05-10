#ifndef _IIC_H_
#define _IIC_H_

#include <Arduino.h>
#include <Wire.h>

// Код ошибки для EEPROM
#define ERROR_CODE      0xFF

/* Состояния кнопки */

// Кнопка нажата
#define ON_PRESS        1
// Кнопка была удержана	
#define ON_PRESS_LONG   2
// Кнопка отпущена
#define ON_RELEASE      3

/* Типы клавиатур */

#define	KB1x4           0
#define KB4x3           1
#define KB4x4           2

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
    byte readbyte(int memoryAddress, bool checkTryAgain, bool showError, const char* called = NULL);

    /** Считыване бита. Функция (почти) является обвёрткой для readbyte
     * @param bitNumber - номер считываемого бита
     * @param memoryAddress - двухбайтовый адрес для чтения данных
     * @param checkTryAgain - повторная попытка (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return булевое значение бита
     */
    bool readbit(int bitNumber, int memoryAddress, bool checkTryAgain = true, bool showError = false);

    /** Считывние строки
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

class Keypad
{
private:
    // Массив буквенных обозначений номера кнопки для переменной getChar
    char _CharKey[3][16] = {
        {'1', '2', '3', '4', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
        {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#', '0', '0', '0', '0'},
        {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'}
    };

    // Массив цифровых обозначений номера кнопки для переменной getNum
    uint8_t	_NumberKey[3][16] =	{
        {0x1, 0x0, 0x3, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
        {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xE, 0x0, 0xF, 0x0, 0x0, 0x0, 0x0},
        {0x1, 0x2, 0x3, 0xA, 0x4, 0x5, 0x6, 0xB, 0x7, 0x8, 0x9, 0xC, 0xE, 0x0, 0xF, 0xD}
    };

    // Тип подключённой клавиатуры
    uint8_t _type;

    // Время удержания кнопки
    uint8_t _timeHold;

    // Адрес клавиатуры в шине IIC
    uint8_t _address = 0x00;

    // Настройки портов I2C
    uint8_t _PORT = 0x00;

    // Состояние расширителя портов
    bool _status = false;

    // Номер прошлой нажатой кнопки
    uint8_t _numbWas = 255;

    // Номер кнопки нажатой сейчас
    uint8_t _numbNow;

    // Время нажатия кнопки
    uint32_t _timePr;

    // Кандидат на статус "долго удерживаемой" кнопки
    uint8_t _forLongPr;

    // Массив входных данных
    bool _inputData[8];

    /** Изменение регистра и получение нового значения порта PCF8574
     * @param data - новое значение регистра порта
     * @return байт порта после изменения его регистра
     */
    byte _changeAndGetBack(byte data);

    /** Изменение регистра порта
     * @param data - новое значение регистра порта
     * @return код окончания передачи, для получения ошибки (если нужно будет)
     */
    int _changePORT(byte data);
public:
    /** Клаватура подключаемая по IIC
     * @param type - тип клавиатуры [4x4, 3x4, 1x4]
     * @param address - адрес расширителя портов в IIC шине начинается с 0b0100XXX (для PCF8574) или 0b0111XXX (для PCF8574A)
     * @param timeHold - время за которое можно считать кнопку не просто нажатой, а удержанной
     */
    Keypad(uint8_t address, uint8_t type, uint32_t timeHold = 2000);
    
    // Инициализация клавиатуры
    void begin();

    // Цифровое обозначение кнопки
    uint8_t	Numb = 0;

    // Символьное обозначение кнопки
    char Char = 0;

    // Состояние кнопки
    uint8_t state;

    /** Считывание клавиши
     * @return true если нажата кнопка
     */
    bool read();
};

class Clock
{
private:
    // Адрес часов в шине I2C
    uint8_t _address;

    /** Запись данных в часы
     * @param numberByte - номер записываемого байта. Для записи всех байтов -1. 
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     */
    bool _write(int numberByte = -1, bool checkTryAgain = true, bool showError = false);
public:
    // Секунды
    byte seconds;
    // Минуты
    byte minutes;
    // Часы
    byte hours;
    // AM или PM. Для 12 часового режима. True - AM, False - PM
    bool AM_PM = false;
    // День недели. Понедельник - 1, воскресенье - 0
    byte day;
    // День месяца
    byte date;
    // Месяц 
    byte month;
    // Год
    int year;

    // Вкл./ Выкл.
    bool working;
    // Статус инциализации 
    bool status = false;
    // Флаг отображающий была ли ошибка
    bool wasError = false;

    // Вывод прямоугольного сигнала для реализации прерывания
    bool SQWE;
    // Выводимый сигнал на случай отключения SQWE
    bool OUT;
    // Кварцевый резонатор
    byte quarts;
    // 24-часовой режим. True - 12 часовой режим, False - 24
    bool mode;

    /** Часы подключаемые по шине I2C
     * @param address - адресс устройства в шине I2C
     * @param doRead - выполнение считывания текущих значений при инициализации 
     */
    Clock(int address = 0b110100, bool doRead = false);

    /** Считывание всех данных (кроме ОЗУ) часова
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return true - если код окончания равен 0 (все хорошо)
     */
    bool read(bool checkTryAgain = true, bool showError = false);

    /** Запись данных в часы
     * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
     * @param showError - вывод ошибки
     * @return true - если код окончания равен 0 (все хорошо)
     */
    bool write(bool checkTryAgain = true, bool showError = false);
    bool wrSec(byte wSeconds, bool checkTryAgain = true, bool showError = false);
    bool wrMin(byte wMinutes, bool checkTryAgain = true, bool showError = false);
    bool wrHours(byte wHours, bool wMode, bool AMorPM, bool checkTryAgain = true, bool showError = false);
    bool wrDay(byte wDay, bool checkTryAgain = true, bool showError = false);
    bool wrDate(byte wDate, bool checkTryAgain = true, bool showError = false);
    bool wrMonth(byte wMonth, bool checkTryAgain = true, bool showError = false);
    bool wrYear(int wYear, bool checkTryAgain = true, bool showError = false);
    bool wrSet(bool wSQWE, bool wOUT, byte wQuarts, bool checkTryAgain = true, bool showError = false);

    bool wrMin(bool checkTryAgain = true, bool showError = false);
    bool wrHours(bool checkTryAgain = true, bool showError = false);
    bool wrDay(bool checkTryAgain = true, bool showError = false);
    bool wrDate(bool checkTryAgain = true, bool showError = false);
    bool wrMonth(bool checkTryAgain = true, bool showError = false);
    bool wrYear(bool checkTryAgain = true, bool showError = false);
    bool wrSet(bool checkTryAgain = true, bool showError = false);

    //!!! Считывание всего. Запись всего, по отдельности. Создание объекта TimeLib. Сихроннизация. Прерывания
};
#endif
