#include <Arduino.h>
#include <Wire.h>
#include "IIC.h"

#define ERROR_CODE 0xFF

EEPROM::EEPROM(uint8_t address, int clock)
{
  _address = address;
  begin(address, clock);
}

/** Инициализация EEPROM
 * @param address - адрес памяти в шине IIC, начинается с 0b1010XXX
 * @param clock - частота шины I2C [10k, 100k, 1M]
 * @return результат попытки подключения к памяти
 */
bool EEPROM::begin(uint8_t address, int clock)
{
  _address = address;
  for (int i = 0; i < 3; i++)                     // Память через раз отвечает на пустой запрос, поэтому используется цикл
  {
    Wire.begin();
    delay(50);
    Wire.setClock(clock);
    Wire.beginTransmission(address);
    int result = Wire.endTransmission();
    status = result == 0?true:false;
    if (status) 
    {
      return true;
    }
  }

  return status;
}

/** Запись одного байта (предназначена для других функций)
 * @param memoryAddress - двухбайтовый адрес для записи данных
 * @param data - записываемая информация
 * @param checkTryAgain - повторная попытка (x3) в случае неудачи
 * @return код окончания передачи, чтобы его можно было использовать в других функциях
 */
int EEPROM::writebyte(int memoryAddress, byte data, bool checkTryAgain)
{
  int code;

  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    Wire.beginTransmission(_address);             // Начало передачи
    Wire.write((memoryAddress) >> 8);             // Отправка адреса
    Wire.write((memoryAddress) & 0xFF);
    Wire.write(data);                             // Отправка данных
    code = Wire.endTransmission();                // Окончание передачи и получени кода передачи
    delay(7);                                     // Время на запись данных
            
    // Обработка кода ошибки
    if (code == 0)                                // Если передача успешная - проверка на битый байт
    {
      i = 3;
      if (data != readbyte(memoryAddress, false, false, "writebyte")) {broken = true; return -1;}
    }
  }
  return code;
}

/** Считывание байта
 * @param memoryAddress - двухбайтовый адрес для чтения данных
 * @param checkTryAgain - повторная попытка (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @param called - для вывода ошибки, чтобы можно было понять какая функция вызвала readbyte
 * @return значение считываемого байта, в случае ошибки вернёт код ошибки, которая определенна в директиве как ERROR_CODE
 */
byte EEPROM::readbyte(int memoryAddress, bool checkTryAgain, bool showError, const char* called)
{
  byte data;
  int code;

  // Цикл попыток получения данных
  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    // Цикл попыток отправки адреса
    for (int y = checkTryAgain?1:3; y <= 3; y++)
    {
      Wire.beginTransmission(_address);
      Wire.write(memoryAddress >> 8);
      Wire.write(memoryAddress & 0xFF);
      code = Wire.endTransmission();

      // Обработка кода ошибки
      if (code != 0 && showError)                 // Вывод ошибки... если нужно
      {
        if (called) {Serial.printf("Error (%d,%d of 3) sending address 0x%x for readbyte, called by %s: 0d%d", i, y, memoryAddress, called, code);}
        else {Serial.printf("Error (%d,%d of 3) sending address 0x%x for readbyte: 0d%d", i, y, memoryAddress, code);}
      }

      if (code == 0) {y = 3;}                     // Досрочный выход из цикла попыток отправки адреса
      else if (code != 0 && y == 3) {return ERROR_CODE;}                   // Если количество попыток исчерпано, то бесмыслено продолжать
    }

    // Начало передачи для получения данных
    Wire.beginTransmission(_address); 
    Wire.requestFrom(_address, 1);
    data = Wire.read();
    code = Wire.endTransmission();

    // Обработка кода ошибки
    if (code != 0 && showError)
    {
      if (called) {Serial.printf("Error (%d of 3) receiving data at address 0x%x for readbyte, called by %s: 0d%d", i, memoryAddress, called, code);}
      else {Serial.printf("Error (%d of 3) receiving data at address 0x%x for readbyte: 0d%d", i, memoryAddress, code);}
    }

    if (code == 0) {return data;}
  }

  return ERROR_CODE;
}

/** Считыване бита. Функция (почти) является обвёрткой для readbyte
 * @param bitNumber - номер считываемого бита
 * @param memoryAddress - двухбайтовый адрес для чтения данных
 * @param checkTryAgain - повторная попытка (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return булевое значение бита
 */
bool EEPROM::readbit(int bitNumber, int memoryAddress, bool checkTryAgain, bool showError)
{
  byte data = readbyte(memoryAddress, checkTryAgain, showError, "readbit");
  return data & (0b1<<bitNumber);
}

/** Считывние строки
 * @remarks Функция "самостоятельная" (не зависит и не использует другие функции)
 * @param length - длина считываемой строки
 * @param memoryAddress - двухбайтовый адрес начала строки, для её чтения
 * @param checkTryAgain - повторная попытка чтения (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return возвращает строку (к сожелению не удалось перейти на массив символов, из-за библиотеки Wire)
 */
String EEPROM::readString(int length, int memoryAddress, bool checkTryAgain, bool showError)
{
  int code;

  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    String data = "";                             // Чтобы в случае ошибки данные не накапливались.

    for (int y = checkTryAgain?1:3; y <= 3; y++)
    {
      Wire.beginTransmission(_address);
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

    // Начало передачи для получения данных
    Wire.beginTransmission(_address); 
    Wire.requestFrom(_address, length);
    while (Wire.available() != 0)
    {
      char temp = Wire.read();
      if (temp == 0) break;
      data += temp;
    }
    code = Wire.endTransmission();

    // Обработка кода ошибки
    if (code != 0 && showError)
    {
      Serial.printf("Error (%d of 3) receiving data at address 0x%x for readString(%d): 0d%d", i, memoryAddress, length, code);
    }
 
    if (code == 0) {return data;}
    else if (code != 0 && i == 3) {return "";}
  }
}

/** Запись байта, с предварительным считыванием (для экономии циклов перезаписи ячеек)
 * @param memoryAddress - двухбайтовый адрес для предварительного чтения и записи данных
 * @param data - записываемый байт
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return код окончания передачи данных
 */
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

/** Запись бита, с предварительным чтением
 * @param memoryAddress - двухбайтовый адрес для предварительного чтения и записи бита (по факту изменённого байта)
 * @param value - булевое значение записываемого бита
 * @param bitNumber - номер считываемого бита
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return код окончания передачи данных
 */
int EEPROM::updatebit(int memoryAddress, bool value, int bitNumber, bool checkTryAgain, bool showError)
{
  int code;
  byte data;
  byte dataNow = readbyte(memoryAddress, checkTryAgain, showError, "updatebit");

  // Измененее байта
  if (value) {data = dataNow | (0b1<<bitNumber);}
  else {data = ~(~dataNow | (0b1<<bitNumber));}

  // Запись
  for (int i = checkTryAgain?1:3; i <= 3; i++)
  {
    code = writebyte(memoryAddress, data, false);
    if (code == 0 || code == -1) {i = 3;}
    if (code !=0  && showError)
    {
      Serial.printf("Error (%d of 3) writing data at address 0x%x for updatebit(0x%x, %d): 0d%d", i, memoryAddress, data, bitNumber, code);
    }
  }
        
  return code;
}

/** Запись строки
 * @param memoryAddress - двухбайтовый адрес начала записи строки
 * @param data - записываемая строка
 * @param endString - нужен ли символ окончания строки '\0'
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return код ошибки (если возникнет ошибка на каком-то этапе, после повторных попыток будет сделан return с последним кодом ошибки, в случае если ошибки не будет функция вернёт 0 (код успешного окончания передачи)
 */
int EEPROM::updateString(int memoryAddress, String data, bool endString, bool checkTryAgain, bool showError)
{
  // Код ошибки, возвращается код последней неудачной передачи
  int code = 0;

  // Запись данных
  for (int i = 0; i < data.length(); i++)
  {
    byte dataNow = readbyte(i + memoryAddress, checkTryAgain, showError, "updateString");           // Считыванее текущего сммвола
    if (dataNow != data[i]) 
    {
      for (int y = 0; y <= (checkTryAgain?1:3); y++)
      {
        code = writebyte(i + memoryAddress, data[i], false);
        if (code == 0) {y = 3;}                   // Выход из цикла повторных попыток
        else if (showError)
        {
          Serial.printf("Error (%d of 3) writing data at address 0x%x for updateString(0x%x, %d): 0d%d", y, memoryAddress, data[i], i, code);
          return code;
        }
      }
    }
  }

  // Запись символа окончания строки
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

/** Вывод карты памяти, через Serial интерфейс. 
 * @warning Функция требует Serial (UART) интерфейс, если его не будет функция сделает выход.
 * @remarks Эта функция самая большая, и большую её часть занимает обработка служебных символов (чтобы не сломать весь вывод в Serial).
 * @param start - начало вывода карты памяти
 * @param size - размер карты (количество считываемых ячеек)
 * @param sizeLine - размер одной строчки
 * @param printText - параллельный вывод расшифровки байтов согласно кодировке  
 * @param checkTryAgain - повторная попытка чтения байта (x3) в случае неудачи
 */
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
          switch (data)                           // Обработка спецсимволов
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

int Keypad::_changePORT(byte data)
{
  Wire.beginTransmission(_address);
  Wire.write(data);
  return Wire.endTransmission(); 
}

byte Keypad::_changeAndGetBack(byte data)
{
  _changePORT(data);

  // Получение значения порта
  Wire.beginTransmission(_address);
  Wire.requestFrom(_address, 1);
  byte _newPORT = Wire.read();
  Wire.endTransmission();
  return _newPORT;
}

Keypad::Keypad(uint8_t address, uint8_t type, uint32_t timeHold)
{
  // Сохранение (копирование) значений параметров клавиатуры

  _address = address;
  _type = type;
  _timeHold = timeHold;

  // Первичная настройка порта
  switch (_type)
  {
  case KB1x4:
    _PORT = 0b00011111;
    break;
  case KB4x3:
    _PORT = 0b01111111;
    break;
  case KB4x4:
    _PORT = 0b11111111;
    break;
  default:
    break;
  }
}

void Keypad::begin()
{
  // Передача значения расширителю портов
  Wire.begin();
  int code = _changePORT(_PORT);

  // По результату окончания передачи можно сделать вывод о работе микросхемы
  if (code == 0) {_status = true;}
  else {_status = false;}
}

bool Keypad::read()
{
  bool _forReturn = false;
  byte _nowPORT;                                  //!Если объявить переменную (кроме переменных цикла) внутри switch, то дальше 1 условия он не двинется.

  // Сброс переменной состояния
  state = false;

  // Определение номера нажатой кнопки
  _numbNow = 255;

  switch (_type)
  {
  case KB1x4:
    // Изменение значения строки
    _nowPORT = _changeAndGetBack(0b00001111);

    for (uint8_t i = 0; i < 4; i++) {if (!(_nowPORT & (0b1 << i))) _numbNow = i;}

    // Возвращаем прошлое значение
    _changePORT(_PORT);

    break;
  case KB4x3:
    for (uint8_t i = 0; i < 4; i++)
    {
      _nowPORT = _changeAndGetBack(~(~_PORT | (0b1 << i)));

      for(uint8_t y = 0; y < 3; y++) {if (!(_nowPORT & (0b1 << y + 4))) _numbNow = i * 3 + y;}
      
      _changePORT(_PORT);
    }
    break;
  case KB4x4:
    for (uint8_t i = 0; i < 4; i++)
    {
      _nowPORT = _changeAndGetBack(~(~_PORT | (0b1 << i)));

      for(uint8_t y = 0; y < 4; y++) {if (!(_nowPORT & (0b1 << y + 4))) _numbNow = i * 4 + y;}
      
      _changePORT(_PORT);
    }
    break;
  default:
    break;
  }

  // Определение состояния кнопки
  if (_numbNow != 255 && _numbNow != _numbWas)
  {
    Numb = _NumberKey[_type][_numbNow];
    Char = _CharKey[_type][_numbNow];
    _forLongPr = _numbNow;
    _timePr = millis();

    state = ON_PRESS;
    _forReturn = true;
  }

  if (_numbNow == 255 && _numbNow != _numbWas && _numbWas != 255 && _forLongPr != 255)
  {
    Numb = _NumberKey[_type][_numbWas];
    Char = _CharKey[_type][_numbWas];

    state = ON_RELEASE;
    _forReturn = true;
  }

  if (_numbNow != 255 && _forLongPr != 255 && millis() - _timePr > _timeHold)
  {
    Numb = _NumberKey[_type][_numbNow];
    Char = _CharKey[_type][_numbNow];
    _forLongPr = 255;
    _timePr = millis();

    state = ON_PRESS_LONG;
    _forReturn = true;
  }

  //Сохранение нажатой кнопки
  _numbWas = _numbNow;
  return _forReturn;
}