#ifndef _IIC_H_
#define _IIC_H_

#include <Arduino.h>
#include <Wire.h>

#define ERROR_CODE 0xFF

//Значения переменных используемых в функциях
/*  
    clock - частота шины I2C [10k, 100k, 1M]
    memoryAddress - двухбайтовый адрес для записи/чтения данных
    data - записываемая/считываемая информация
    dataNow - текущий записаный байт
    code - код результата передачи данных [-1 - битый байт; 0 - норма; остальное неудача]
    checkTryAgain - проверка и повторная попытка в случае неудачи 
    showError - вывод ошибки
    called - информация о функции которая вызвала функцию, нужно для вывода ошибки.
*/

class EEPROM
{
private:
    uint8_t addressI2C;
    // bool wasERROR = false; //Нужно подключить флаг ошибки, чтобы при работе с сервером не было проблем с выявлением ошибки

public:
    bool status = false;                            //Статус инициализации
    bool broken = false;

    EEPROM(uint8_t address, int clock = 10000);
    bool begin(uint8_t address, int clock = 10000);
    int writebyte(int memoryAddress, byte data, bool checkTryAgain = true);
    byte readbyte(int memoryAddress, bool checkTryAgain, bool showError, const char* called = NULL);
    bool readbit(int bitNumber, int memoryAddress, bool checkTryAgain = true, bool showError = false);
    String readString(int length, int memoryAddress, bool checkTryAgain = true, bool showError = false);
    int updatebyte(int memoryAddress, byte data, bool checkTryAgain = true, bool showError = false);
    int updatebit(int memoryAddress, bool value, int numberBit, bool checkTryAgain = true, bool showError = false);
    int updateString(int memoryAddress, String data, bool endString = true, bool checkTryAgain = true, bool showError = false);
    void printmap(int start, int size, int sizeLine, bool printText, bool checkTryAgain = true);
};

#endif