#include "Arduino.h"
#include "Wire.h"

#define EEPROM_ADDRESS 0b1010000
#define ERROR_READ 0xFF

bool NeedSerial = true;
char errorBuffer[70];

bool begin(uint8_t address, int clock = 10000);
int writebyte(int memoryAddress, byte data, bool checkTryAgain = true);

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
    int writebyte(int memoryAddress, byte data, bool checkTryAgain) ///
    {
        int code;

        for (int i = checkTryAgain?1:3; i <= 3; i++)
        {
            Wire.beginTransmission(addressI2C);         //Начало передачи
            Wire.write((memoryAddress) >> 8);           //Отправка адреса
            Wire.write((memoryAddress) & 0xFF);
            Wire.write(data);                           //Отправка данных
            code = Wire.endTransmission();              //Окончание передачи и получени кода передачи
            delay(7);                                   //Время на запись данных
            
            //Обработка кода ошибки
            if (code == 0)                              //Если передача успешная - проверка на битый байт
            {
                i = 3;
                if (data != readbyte(memoryAddress, false, false)) {broken = true; return -1;}
            }
        }
        return code;
    }

    int updatebyte(int memoryAddress, byte data, bool checkTryAgain, bool showError)    ///Связан с updateByte и updateString
    {
        int code;
        byte dataNow = readbyte(memoryAddress, false, showError);
        
        if (dataNow != data)
        {
            for (int i = checkTryAgain?1:3; i <= 3; i++)
            {
                code = writebyte(memoryAddress, data, checkTryAgain);
                if (code == 0) {i = 3;}
                else 
                {
                    sprintf(errorBuffer, "Error (%d of 3) writing data at address 0x%x for updatebyte: 0d%d", i, memoryAddress, code);
                    Serial.println(errorBuffer);
                }
            }
            return code;
        }
        else {return 0;}
    }

    int updatebit(int memoryAddress, bool value, int numberBit, bool checkTryAgain, bool showError)
    {
        byte data;
        byte dataNow = readbyte(memoryAddress, checkTryAgain, showError);

        if (value) {data = dataNow | (0b1<<numberBit);}
        else {data = ~(~dataNow | (0b1<<numberBit));}

        updatebyte(memoryAddress, data, checkTryAgain, showError);
    }

    int updateString(int memoryAddress, String data, bool endString, bool checkTryAgain, bool showError)
    {
        //Код ошибки, возвращается код последней неудачной передачи
        int code = 0;

        //Запись данных
        for (int i = 0; i < data.length(); i++)
        {
            byte dataNow = readbyte(i + memoryAddress, checkTryAgain, showError);

            if (dataNow != data[i]) 
            {
                int codeNow = updatebyte(i + memoryAddress, data[i], checkTryAgain, showError);
                if (codeNow != 0) {code = codeNow;}
            }
        }
        //Запись символа окончания строки
        if (endString && 0x00 != readbyte(memoryAddress, checkTryAgain, showError)) updatebyte(memoryAddress + data.length(), 0x00, checkTryAgain, showError);

        return code;
    }

    //Считывание байта
    byte readbyte(int memoryAddress, bool checkTryAgain, bool showError)    ///
    {
        byte data;
        int code;

        //Цикл попыток получения данных
        for (int i = checkTryAgain?1:3; i <= 3; i++)
        {
            //Цикл попыток отправки адреса
            for (int y = checkTryAgain?1:3; y <= 3; y++)
            {
                Wire.beginTransmission(EEPROM_ADDRESS);
                Wire.write(memoryAddress >> 8);
                Wire.write(memoryAddress & 0xFF);
                code = Wire.endTransmission();

                //Обработка кода ошибки
                if (code != 0 && showError)          //Вывод ошибки... если нужно
                {
                    sprintf(errorBuffer, "Error (%d,%d of 3) sending address 0x%x for readbyte: 0d%d", i, y, memoryAddress, code);
                    Serial.println(errorBuffer);
                }

                if (code == 0) {y = 3;}             //Досрочный выход из цикла попыток отправки адреса
                else if (code != 0 && y == 3) {return ERROR_READ;}//Если количество попыток исчерпано, то бесмыслено продолжать
            }

            //Начало передачи для получения данных
            Wire.beginTransmission(EEPROM_ADDRESS); 
            Wire.requestFrom(EEPROM_ADDRESS, 1);
            data = Wire.read();
            code = Wire.endTransmission();

            //Обработка кода ошибки
            if (code != 0 && showError)
            {
                sprintf(errorBuffer, "Error (%d of 3) receiving data at address 0x%x for readbyte: 0d%d", i, memoryAddress, code);
                Serial.println(errorBuffer);
            }

            if (code == 0) {return data;}
            else if (code != 0 && i == 3) {return ERROR_READ;}
        }
    }

    bool readbit(int bitNumber, int memoryAddress, bool checkTryAgain, bool showError)
    {
        byte data = readbyte(memoryAddress, checkTryAgain, showError);
        return data & (0b1<<bitNumber);
    }

    String readString(int length, int memoryAddress, bool checkTryAgain, bool showError)
    {
        int code;

        for (int i = checkTryAgain?1:3; i <= 3; i++)
        {
            String data = "";                   //Чтобы в случае ошибки данные не накапливались.

            for (int y = checkTryAgain?1:3; y <= 3; y++)
            {
                Wire.beginTransmission(EEPROM_ADDRESS);
                Wire.write(memoryAddress >> 8);
                Wire.write(memoryAddress & 0xFF);
                code = Wire.endTransmission();

                if (code != 0 && showError)
                {
                    sprintf(errorBuffer, "Error (%d,%d of 3) sending address 0x%x for readString %d: 0d%d", i, y, memoryAddress, length, code);
                    Serial.println(errorBuffer);
                }

                if (code == 0) {y = 3;}
                else if (code != 0 && y == 3) {return "";}
            }

            //Начало передачи для получения данных
            Wire.beginTransmission(EEPROM_ADDRESS); 
            Wire.requestFrom(EEPROM_ADDRESS, length);
            while (Wire.available() != 0)
            {
                char temp = Wire.read();
                if (temp == 0) break;
                data += temp;
            }
            code = Wire.endTransmission();

            //Обработка кода ошибки
            if (code != 0 && showError)
            {
                sprintf(errorBuffer, "Error (%d of 3) receiving data at address 0x%x for readString %d: 0d%d", i, memoryAddress, length, code);
                Serial.println(errorBuffer);
            }

            if (code == 0) {return data;}
            else if (code != 0 && i == 3) {return "";}
        }
    }

    void printmap(int start, int size, int sizeLine, bool printText, bool checkTryAgain)
    {
        if (Serial)
        {
            String tempString;
            byte data;

            for (int i = start; i < size + start;)
            {
                Serial.print(String(i) + ": ");
                for (int y = 0; y < sizeLine; y++)
                {
                    data = readbyte(i, checkTryAgain, false);
                    if (printText) 
                    {
                        switch (data)                       //Обработка спецсимволов
                        {
                            case 0x00:
                                tempString += " NUL ";
                                break;
                            case 0x01:
                                tempString += " SOH ";
                                break;
                            case 0x02:
                                tempString += " STX ";
                                break;
                            case 0x03:
                                tempString += " ETX ";
                                break;
                            case 0x04:
                                tempString += " EOT ";
                                break;
                            case 0x05:
                                tempString += " ENQ ";
                                break;
                            case 0x06:
                                tempString += " ACK ";
                                break;
                            case 0x07:
                                tempString += " BEL ";
                                break;
                            case 0x08:
                                tempString += " BS ";
                                break;
                            case 0x09:
                                tempString += " TAB ";
                                break;
                            case 0x0A:
                                tempString += " LF ";
                                break;
                            case 0x0B:
                                tempString += " VT ";
                                break;
                            case 0x0C:
                                tempString += " FF ";
                                break;
                            case 0x0D:
                                tempString += " CR ";
                                break;
                            case 0x0E:
                                tempString += " SO ";
                                break;
                            case 0x0F:
                                tempString += " SI ";
                                break;
                            case 0x10:
                                tempString += " DLE ";
                                break;
                            case 0x11:
                                tempString += " DC1 ";
                                break;
                            case 0x12:
                                tempString += " DC2 ";
                                break;
                            case 0x13:
                                tempString += " DC3 ";
                                break;
                            case 0x14:
                                tempString += " DC4 ";
                                break;
                            case 0x15:
                                tempString += " NAK ";
                                break;
                            case 0x16:
                                tempString += " SYN ";
                                break;
                            case 0x17:
                                tempString += " ETB ";
                                break;
                            case 0x18:
                                tempString += " CAN ";
                                break;
                            case 0x19:
                                tempString += " EM ";
                                break;
                            case 0x1A:
                                tempString += " SUB ";
                                break;
                            case 0x1B:
                                tempString += " ESC ";
                                break;
                            case 0x7F:
                                tempString += " DEL ";
                                break;
                            default:
                                tempString += " ";
                                tempString += (char)data; 
                                tempString += " ";
                                break;
                        }
                    }
                    Serial.print(data);
                    Serial.print(" ");
                    i++;
                }
                if (printText) 
                {
                    Serial.print((char)0x09 + tempString);
                    tempString = "";
                }
                Serial.println();
            }
        } 
    }

    //Значения переменных используемых в функциях
    /*  
        clock - частота шины I2C [10k, 100k, 1M]
        memoryAddress - двухбайтовый адрес для записи/чтения данных
        data - записываемая/считываемая информация
        dataNow - текущий записаный байт
        code - код результата передачи данных [-1 - битый байт; 0 - норма; остальное неудача]
        checkTryAgain - проверка и повторная попытка в случае неудачи 
        showError - вывод ошибки
        causedBy - информация о функции которая вызвала функцию, нужно для вывода ошибки.
    */
};

void setup()
{
    Serial.begin(115200);
    EEPROM memory(EEPROM_ADDRESS, 10000);
    if (memory.status == true) {Serial.println("Find EEPROM");}
    else {Serial.println("ERROR");}

    // memory.printmap(0, 50, 10, true, true);
    memory.writebyte(1, 0xAB, true);
    Serial.println(memory.readbyte(1, true, true));
}

void loop()
{

}