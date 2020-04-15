#include <Arduino.h>
#include <Wire.h>
#include "IIC.h"

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

EEPROM::EEPROM(uint8_t address, int clock)
{
  addressI2C = address;
  begin(address, clock);
}

//Инициализация EEPROM
bool EEPROM::begin(uint8_t address, int clock)
{
  /**/Serial.begin(115200);
  addressI2C = address;
  for (int i = 0; i < 3; i++)           //Память через раз отвечает на пустой запрос, поэтому используется цикл
  {
    Wire.begin();
    delay(50);
    Wire.setClock(clock);
    Wire.beginTransmission(address);
    int result = Wire.endTransmission();
    status = result == 0?true:false;
    if (status) 
    {
      /**/pinMode(2, OUTPUT);
      /**/digitalWrite(2, HIGH);
      return true;
    }
  }

  return status;
}

//Запись одного байта (предназначена для других функций)
int EEPROM::writebyte(int memoryAddress, byte data, bool checkTryAgain)
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
      if (data != readbyte(memoryAddress, false, false, "writebyte")) {broken = true; return -1;}
    }
  }
  return code;
}

//Считывание байта
byte EEPROM::readbyte(int memoryAddress, bool checkTryAgain, bool showError, const char* called)
{
  byte data;
  int code;

  //Цикл попыток получения данных
  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    //Цикл попыток отправки адреса
    for (int y = checkTryAgain?1:3; y <= 3; y++)
    {
      Wire.beginTransmission(addressI2C);
      Wire.write(memoryAddress >> 8);
      Wire.write(memoryAddress & 0xFF);
      code = Wire.endTransmission();

      //Обработка кода ошибки
      if (code != 0 && showError)          //Вывод ошибки... если нужно
      {
        if (called) {Serial.printf("Error (%d,%d of 3) sending address 0x%x for readbyte, called by %s: 0d%d", i, y, memoryAddress, called, code);}
        else {Serial.printf("Error (%d,%d of 3) sending address 0x%x for readbyte: 0d%d", i, y, memoryAddress, code);}
      }

      if (code == 0) {y = 3;}             //Досрочный выход из цикла попыток отправки адреса
      else if (code != 0 && y == 3) {return ERROR_CODE;}//Если количество попыток исчерпано, то бесмыслено продолжать
    }

    //Начало передачи для получения данных
    Wire.beginTransmission(addressI2C); 
    Wire.requestFrom(addressI2C, 1);
    data = Wire.read();
    code = Wire.endTransmission();

    //Обработка кода ошибки
    if (code != 0 && showError)
    {
      if (called) {Serial.printf("Error (%d of 3) receiving data at address 0x%x for readbyte, called by %s: 0d%d", i, memoryAddress, called, code);}
      else {Serial.printf("Error (%d of 3) receiving data at address 0x%x for readbyte: 0d%d", i, memoryAddress, code);}
    }

    if (code == 0) {return data;}
  }

  return ERROR_CODE;
}

//Считыване бита
bool EEPROM::readbit(int bitNumber, int memoryAddress, bool checkTryAgain, bool showError)
{
  byte data = readbyte(memoryAddress, checkTryAgain, showError, "readbit");
  return data & (0b1<<bitNumber);
}

//Считывние строки
String EEPROM::readString(int length, int memoryAddress, bool checkTryAgain, bool showError)
{
  int code;

  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    String data = "";                   //Чтобы в случае ошибки данные не накапливались.

    for (int y = checkTryAgain?1:3; y <= 3; y++)
    {
      Wire.beginTransmission(addressI2C);
      Wire.write(memoryAddress >> 8);
      Wire.write(memoryAddress & 0xFF);
      code = Wire.endTransmission();

      if (code != 0 && showError)
      {
        Serial.printf("Error (%d,%d of 3) sending address 0x%x for readString(%d): 0d%d", i, y, memoryAddress, length, code);
      }

      if (code == 0) {y = 3;}
      else if (code != 0 && y == 3) {return "";}
    }

    //Начало передачи для получения данных
    Wire.beginTransmission(addressI2C); 
    Wire.requestFrom(addressI2C, length);
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
      Serial.printf("Error (%d of 3) receiving data at address 0x%x for readString(%d): 0d%d", i, memoryAddress, length, code);
    }
 
    if (code == 0) {return data;}
    else if (code != 0 && i == 3) {return "";}
  }
}

//Запись байта, с предварительным считыванием
int EEPROM::updatebyte(int memoryAddress, byte data, bool checkTryAgain, bool showError)
{
  int code;
  byte dataNow = readbyte(memoryAddress, checkTryAgain, showError, "updatebyte");
        
  if (dataNow != data)
  {
    for (int i = checkTryAgain?1:3; i <= 3; i++)
    {
      code = writebyte(memoryAddress, data, false);
      if (code == 0 || code == -1) {i = 3;}
      else 
      {
        Serial.printf("Error (%d of 3) writing data at address 0x%x for updatebyte(0x%x): 0d%d", i, memoryAddress, data, code);
      }
    }
    return code;
  }
  else {return 0;}
}

//Запись бита
int EEPROM::updatebit(int memoryAddress, bool value, int numberBit, bool checkTryAgain, bool showError)
{
  int code;
  byte data;
  byte dataNow = readbyte(memoryAddress, checkTryAgain, showError, "updatebit");

  //Измененее байта
  if (value) {data = dataNow | (0b1<<numberBit);}
  else {data = ~(~dataNow | (0b1<<numberBit));}

  //Запись
  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    code = writebyte(memoryAddress, data, false);
    if (code == 0 || code == -1) {i = 3;}
    if (code !=0  && showError)
    {
      Serial.printf("Error (%d of 3) writing data at address 0x%x for updatebit(0x%x, %d): 0d%d", i, memoryAddress, data, numberBit, code);
    }
  }
        
  return code;
}

//Запись строки
int EEPROM::updateString(int memoryAddress, String data, bool endString, bool checkTryAgain, bool showError)
{
  //Код ошибки, возвращается код последней неудачной передачи
  int code = 0;

  //Запись данных
  for (int i = 0; i < data.length(); i++)
  {
    byte dataNow = readbyte(i + memoryAddress, checkTryAgain, showError, "updateString");//Считыванее текущего сммвола
    if (dataNow != data[i]) 
    {
      for (int y = 0; y <= (checkTryAgain?1:3); y++)
      {
        code = writebyte(i + memoryAddress, data[i], false);
        if (code == 0) {y = 3;}                 //Выход из цикла повторных попыток
        else if (showError)
        {
          Serial.printf("Error (%d of 3) writing data at address 0x%x for updateString(0x%x, %d): 0d%d", y, memoryAddress, data[i], i, code);
          return code;
        }
      }
    }
  }

  //Запись символа окончания строки
  if (endString && 0x00 != readbyte(memoryAddress + data.length(), checkTryAgain, showError, "updateString")) for (int y = 1; y <= (checkTryAgain?1:3); y++)
  {
    code = writebyte(memoryAddress + data.length(), 0x00, checkTryAgain);
    if (code == 0) {return code;}
    else if (showError)
    {
      Serial.printf("Error (%d of 3) writing endchar at address 0x%x for updatebit(0x00): 0d%d", y, memoryAddress + data.length(), code);
    }
  }

  return code;
}

//Вывод карты памяти
void EEPROM::printmap(int start, int size, int sizeLine, bool printText, bool checkTryAgain)
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
        data = readbyte(i, checkTryAgain, false, "printmap");
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