#include "Arduino.h"
#include "Wire.h"

#define EEPROM_ADDRESS 0b1010000

bool NeedSerial = true;

bool begin(uint8_t address, int clock = 10000);

class EEPROM
{
public:
    bool status = false;                            //Статус инициализации
    bool broken = false;
    uint8_t addressI2C;

    EEPROM(uint8_t address, int clock)
    {
        addressI2C = address;
        begin(address, clock);
    }

    //Инициализация EEPROM
    bool begin(uint8_t address, int clock)
    {
        addressI2C = address;

        Wire.begin();
        Wire.setClock(clock);
        Wire.beginTransmission(address);
        int result = Wire.endTransmission();
        status = result == 0?true:false;
        return status;
    }

    //Запись одного байта (предназначена для других функций)
    int writebyte(int memoryAddress, byte data, bool checkTryAgain = true)
    {
        int code;

        Wire.beginTransmission(addressI2C);         //Начало передачи
        Wire.write((memoryAddress) >> 8);           //Отправка адреса
        Wire.write((memoryAddress) & 0xFF);
        Wire.write(data);                           //Отправка данных
        code = Wire.endTransmission();              //Окончание передачи
        delay(7);                                   //Время на запись данных

        if (checkTryAgain)                          //Проверка
        {
            if (code != 0)                          //В случае ошибки передачи - повторные 3 попытки
            {
                for (int i = 0; i < 3; i++)
                {
                    code = writebyte(memoryAddress, data, false);
                    if (code != 0) {}               //Если повторная запись не удалась - продолжаем повтор
                    else                            //Если передача успешная - проверка на битый байт
                    {
                        if (data != readbyte(memoryAddress, false)) {broken = true; return -1;}
                        else return 0;              //Досрочное завершение
                    }
                }
            }
            else if (data != readbyte(memoryAddress, false)) {broken = true; return -1;} //Проверка на битый байт
        }
        return code;
    }

    //Считывание байта
    byte readbyte(int memoryAddress, bool checkTryAgain)
    {

    }

    //Значения переменных используемых в функциях
    /*  
        clock - частота шины I2C [10k, 100k, 1M]
        memoryAddress - двухбайтовый адрес для записи/чтения данных
        data - записываемая/считываемая информация
        code - код результата передачи данных [-1 - битый байт; 0 - норма]
        checkTryAgain - проверка и повторная попытка в случае неудачи 
    */
};

void setup()
{
    Serial.begin(115200);
    while (!Serial){};
    Serial.println("Hi");
    EEPROM memory(EEPROM_ADDRESS, 10000);
    if (memory.status == true) Serial.println("Find EEPROM");
}

void loop()
{

}