#include "Clock.h"

/** Часы подключаемые по шине I2C
 * @param address - адрес устройства в шине I2C
 * @param check - выполнение проверки подключения при инициализации, вне основного кода не сработает 
 */
Clock::Clock(int address, bool check)
{
    _address = address;
    if (check)
    {
        begin();
    }
}

/** Перевод числа из 10 в 16 систему счисления и обратно
 * @param numb - число
 * @param toBCD - True из DEC в HEX, False из HEX в DEC
 */
int Clock::_transNumber(int numb, bool toBCD)
{
    if (toBCD) {
        return ((numb / 10 * 16) + (numb % 10));
    }
    else {
        return ((numb / 16 * 10) + (numb % 16));
    }
}

/** Инициализация часов
 * @return true - если не было ошибки
 */
bool Clock::begin()
{
    Wire.begin();
    Wire.beginTransmission(_address);
    int code = Wire.endTransmission();

    if (code != 0) {
        status = false;
    }
    else {
        status = true;
        read();
    }
}

/** Считывание всех данных (кроме ОЗУ) часов
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return true - если код окончания равен 0 (все хорошо)
 */
bool Clock::read(bool checkTryAgain, bool showError)
{
    byte temp[8]; // Временная переменная для хранения байта
    int code;     // Переменная для хранения кода ошибки
    wasError = false;

    byte second; // Секунды
    byte minute; // Минуты
    byte hour;   // Часы
    byte day;    // День месяца
    byte month;  // Месяц
    int year;    // Год

    for (int i = checkTryAgain ? 1 : 3; i <= 3; i++)  {// Вначале отправляется адрес памяти (принцип похожий с обычной памятью), поэтому используется 2-й цикл
        for (int y = checkTryAgain ? 1 : 3; y <= 3; y++) {
            Wire.beginTransmission(_address);
            Wire.write(0x00);
            code = Wire.endTransmission();
            if (code != 1 && code != 2 && code != 3 && code != 4) {
                y = 4;
            } // Wire.endTransmission() научился возвращать 8, хотя должен возвращать число [0, 4]
            else if (showError) {
                Serial.printf("Error (%d,%d of 3) sending address 0x00 for getting data: 0d%d\n", i, y, code);
            }
        }

        Wire.requestFrom(_address, 8);
        for (int y = 0; y < 8; y++) {
            temp[y] = Wire.read();
        }
        code = Wire.endTransmission();

        if (code == 1 || code == 2 || code == 3 || code == 4) {
            if (showError) {
                Serial.printf("Error (%d of 3) getting data: 0d%d\n", i, code);
            }
            continue;
        }

        // Обработка нулевого байта. Содержит вкл./выкл. и секунды
        working = temp[0] & 0b10000000;
        second = _transNumber(temp[0] & 0b1111111, false);

        // Первый байт - минуты.
        minute = _transNumber(temp[1], false);

        // Второй байт - часы и их режим
        mode = temp[2] & 0b1000000;
        if (mode) {
            bool AM_PM = temp[2] & 0b100000;
            hour = _transNumber(temp[2] & 0b11111, false) + AM_PM ? 12 : 0;
        }
        else {
            hour = _transNumber(temp[2] & 0b111111, false);
        }

        // Четвёртый байт - день (число)
        day = _transNumber(temp[4] & 0b111111, false);

        // Пятый байт - месяц
        month = _transNumber(temp[5] & 0b11111, false);

        // Шестой байт - год
        year = _transNumber(temp[6], false) + 2000;

        // Седьмой байт - параметры часов
        OUT = temp[7] & 0b10000000;
        SQWE = temp[7] & 0b10000;
        quarts = temp[7] & 0b11;
    }
    if (code == 1 || code == 2 || code == 3 || code == 4) {
        wasError = true;
        return false;
    }
    else {
        setTime(hour, minute, second, day, month, year);
        return true;
    }
}

/** Считывание ОЗУ часов
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return строку из 64 символов
 */
String Clock::readRAM(bool checkTryAgain, bool showError)
{
    String temp;
    wasError = false;

    for (int i = checkTryAgain ? 1 : 3; i <= 3; i++) {
        for (int y = checkTryAgain ? 1 : 3; y <= 3; y++) {
            Wire.beginTransmission(_address); // Отправка адреса начала данных RAM
            Wire.write(0x08);
            int code = Wire.endTransmission();
            if (code == 1 || code == 2 || code == 3 || code == 4) {
                wasError = true;
                if (showError) {
                    Serial.printf("Error (%d,%d of 3) sending address 0x08 for getting RAM data: 0d%d\n", i, y, code);
                }
            }
            else {
                y = 4;
            }
        }

        temp = ""; // Сброс переменой temp
        Wire.requestFrom(_address, 56);

        while (Wire.available()) {
            temp += (char)Wire.read();
        }

        int code = Wire.endTransmission();
        if (code == 1 || code == 2 || code == 3 || code == 4) {
            wasError = true;
            if (showError) {
                Serial.printf("Error (%d of 3) getting RAM data: 0d%d\n", i, code);
            }
        }
    }
    return temp;
}

/**  Синхронизация часов
 * @param http - ссылка на объект "клиента"
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 * @return true - если не было ошибки
 */
bool Clock::sync(WiFiClient &http, bool checkTryAgain, bool showError)
{
    wasError = false;

    byte second; // Секунды
    byte minute; // Минуты
    byte hour;   // Часы
    byte day;    // День месяца
    byte month;  // Месяц
    int year;    // Год

    // Подключение к серверу
    http.connect("worldtimeapi.org", 80);
    http.println("GET /api/ip HTTP/1.1\r\nHost: worldtimeapi.org\r\nConnection: close\r\n\r\n");
    delay(200);
    if (!http.find("\r\n\r\n")) {
        wasError = true;
        return false;
    }

    // От сервера приходит JSON файл, его необходимо предварительно обработать
    const size_t capacity = JSON_OBJECT_SIZE(15) + 300;
    DynamicJsonDocument temp(capacity);
    DeserializationError error = deserializeJson(temp, http);
    if (error) {
        wasError = true;
        return false;
    }

    String dateTime = temp["datetime"];

    // Перенос полученных данных в переменные для удобства
    second = dateTime.substring(17, 19).toInt();
    minute = dateTime.substring(14, 16).toInt();
    hour = dateTime.substring(11, 13).toInt();
    day = dateTime.substring(8, 10).toInt();
    month = dateTime.substring(5, 8).toInt();
    year = dateTime.substring(0, 4).toInt();
    setTime(hour, minute, second, day, month, year);

    working = true;
    write();

    return true;
}

/** Проверка на попадание в диапазон времени
 * @warning если часы сбиты, то пропускаем проверку
 * @param sHours - часы первого диапазона
 * @param sMin - минуты первого диапазона
 * @param eHours - часы второго диапазона
 * @param eMin - минуты второго диапазона
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 */
bool Clock::compare(byte sHours, byte sMin, byte eHours, byte eMin, bool checkTryAgain, bool showError)
{
    int nTime = hour() * 100 + minute();
    int sTime = sHours * 100 + sMin;
    int eTime = eHours * 100 + eMin;

    if (timeStatus() && WiFi.status() != WL_CONNECTED) {
        return true;
    } // Если часы не настроены ... открываем дверь
    else {
        WiFiClient http;
        if (!sync(http))
            return true;
    }

    if (sTime < eTime) {                          // Проверка на "ночную смену"
        if (sTime <= nTime && eTime >= nTime) {
            return true;
        }
    }
    else if ((sTime >= nTime) == (eTime >= nTime)) {
        return true;
    }

    return false;
}

/** Запись данных в RTC часы с внутренних часов
 * @param numberByte - номер записываемого байта 
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 */
bool Clock::write(byte numberByte, bool checkTryAgain, bool showError)
{
    byte temp;
    int code;
    wasError = false;

    for (int i = checkTryAgain ? 1 : 3; i <= 3; i++) {
        Wire.beginTransmission(_address);
        if (numberByte == -1) {
            Wire.write(0x00);
        } // Установка адреса начала записи
        else {
            Wire.write(numberByte);
        }

        switch (numberByte) {
        case 0:
            Wire.write(working << 7 | _transNumber(second(), true));
            break;
        case 1:
            Wire.write(_transNumber(minute(), true));
            break;
        case 2:
            if (mode) {
                temp |= 1 << 6;
                temp |= (hour() / 12) << 5;
                temp |= _transNumber(hour() % 12, true);
            }
            else {
                temp = _transNumber(hour(), true);
            }

            Wire.write(_transNumber(temp, true));
            break;
        case 3:
            Wire.write(0);
            break;
        case 4:
            Wire.write(_transNumber(day(), true));
            break;
        case 5:
            Wire.write(_transNumber(month(), false));
            break;
        case 6:
            Wire.write(_transNumber(year() % 100, true));
            break;
        case 7:
            Wire.write((OUT << 7) | (SQWE << 4) | quarts);
            break;

        default:
            break;
        }

        code = Wire.endTransmission();
        if (code == 1 || code == 2 || code == 3 || code == 4) {
            if (showError) {
                Serial.printf("Error (%d of 3) writing data (0x%x): 0d%d\n", i, numberByte, code);
            }
            continue;
        }
    }

    if (code == 1 || code == 2 || code == 3 || code == 4) {
        wasError = true;
        return false;
    }
    else {
        return true;
    }
}

/** Запись всех данных в RTC часы с внутренних часов
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 */
bool Clock::write(bool checkTryAgain, bool showError)
{
    byte temp;
    int code;
    wasError = false;

    for (int i = checkTryAgain ? 1 : 3; i <= 3; i++) {
        Wire.beginTransmission(_address);
        Wire.write(0x00); // Установка адреса начала записи

        Wire.write(working << 7 | _transNumber(second(), true));
        Wire.write(_transNumber(minute(), true));
        if (mode) {
            temp |= 1 << 6;
            temp |= (hour() / 12) << 5;
            temp |= _transNumber(hour() % 12, true);
        }
        else {
            temp = _transNumber(hour(), true);
        }
        Wire.write(_transNumber(temp, true));
        Wire.write(0);
        Wire.write(_transNumber(day(), true));
        Wire.write(_transNumber(month(), false));
        Wire.write(_transNumber(year() % 100, true));
        Wire.write((OUT << 7) | (SQWE << 4) | quarts);

        code = Wire.endTransmission();
        if (code == 1 || code == 2 || code == 3 || code == 4) {
            if (showError) {
                Serial.printf("Error (%d of 3) writing data: 0d%d\n", i, code);
            }
            continue;
        }
    }

    if (code == 1 || code == 2 || code == 3 || code == 4) {
        wasError = true;
        return false;
    }
    else {
        return true;
    }
}

/** Запись байта данных в RTC часы
 * @param numberByte - номер записываемого байта 
 * @param data - записываемый байт
 * @param checkTryAgain - повторная попытка чтения и записи (x3) в случае неудачи
 * @param showError - вывод ошибки
 */
bool Clock::write(byte numberByte, byte data, bool checkTryAgain, bool showError)
{
    int code;
    wasError = false;

    for (int i = checkTryAgain ? 1 : 3; i <= 3; i++) {
        Wire.beginTransmission(_address);
        Wire.write(numberByte); // Установка адреса начала записи
        Wire.write(data);

        code = Wire.endTransmission();
        if (code == 1 || code == 2 || code == 3 || code == 4) {
            if (showError) {
                Serial.printf("Error (%d of 3) writing data (0x%x, 0x%x): 0d%d\n", i, numberByte, data, code);
            }
            continue;
        }
    }

    if (code == 1 || code == 2 || code == 3 || code == 4) {
        wasError = true;
        return false;
    }
    else {
        return true;
    }
}
