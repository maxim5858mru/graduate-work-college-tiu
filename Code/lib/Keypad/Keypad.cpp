#include "Keypad.h"

/** Клаватура подключаемая по IIC
 * @param type - тип клавиатуры [4x4, 3x4, 1x4]
 * @param address - адрес расширителя портов в IIC шине начинается с 0b0100XXX (для PCF8574) или 0b0111XXX (для PCF8574A)
 * @param timeHold - время за которое можно считать кнопку не просто нажатой, а удержанной
 */
Keypad::Keypad(uint8_t address, uint8_t type, uint32_t timeHold)
{
    // Сохранение (копирование) значений параметров клавиатуры

    _address = address;
    _type = type;
    _timeHold = timeHold;

    // Первичная настройка порта
    switch (_type) {
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

/** Изменение регистра порта
 * @param data - новое значение регистра порта
 * @return код окончания передачи, для получения ошибки (если нужно будет)
 */
int Keypad::_changePORT(byte data)
{
    Wire.beginTransmission(_address);
    Wire.write(data);
    return Wire.endTransmission();
}

/** Изменение регистра и получение нового значения порта PCF8574
 * @param data - новое значение регистра порта
 * @return байт порта после изменения его регистра
 */
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

// Инициализация клавиатуры
void Keypad::begin()
{
    // Передача значения расширителю портов
    Wire.begin();
    int code = _changePORT(_PORT);

    // По результату окончания передачи можно сделать вывод о работе микросхемы
    if (code == 0) {
        _status = true;
    }
    else {
        _status = false;
    }
}

/** Считывание клавиши
 * @return true если нажата кнопка
 */
bool Keypad::read()
{
    bool _forReturn = false;
    byte _nowPORT; // !Если объявить переменную внутри switch, то дальше 1 условия он не двинется.

    // Сброс переменной состояния
    state = false;

    // Определение номера нажатой кнопки
    _numbNow = 255;

    switch (_type) {
    case KB1x4:
        // Изменение значения строки
        _nowPORT = _changeAndGetBack(0b00001111);

        for (uint8_t i = 0; i < 4; i++) {
            if (!(_nowPORT & (0b1 << i)))
                _numbNow = i;
        }

        // Возвращаем прошлое значение
        _changePORT(_PORT);

        break;
    case KB4x3:
        for (uint8_t i = 0; i < 4; i++) {
            _nowPORT = _changeAndGetBack(~(~_PORT | (0b1 << i)));

            for (uint8_t y = 0; y < 3; y++)
            {
                if (!(_nowPORT & (0b1 << y + 4)))
                    _numbNow = i * 3 + y;
            }

            _changePORT(_PORT);
        }
        break;
    case KB4x4:
        for (uint8_t i = 0; i < 4; i++) {
            _nowPORT = _changeAndGetBack(~(~_PORT | (0b1 << i)));

            for (uint8_t y = 0; y < 4; y++) {
                if (!(_nowPORT & (0b1 << y + 4)))
                    _numbNow = i * 4 + y;
            }

            _changePORT(_PORT);
        }
        break;
    default:
        break;
    }

    // Определение состояния кнопки
    if (_numbNow != 255 && _numbNow != _numbWas) {
        Numb = _NumberKey[_type][_numbNow];
        Char = _CharKey[_type][_numbNow];
        _forLongPr = _numbNow;
        _timePr = millis();

        state = ON_PRESS;
        _forReturn = true;
    }

    if (_numbNow == 255 && _numbNow != _numbWas && _numbWas != 255 && _forLongPr != 255) {
        Numb = _NumberKey[_type][_numbWas];
        Char = _CharKey[_type][_numbWas];

        state = ON_RELEASE;
        _forReturn = true;
    }

    if (_numbNow != 255 && _forLongPr != 255 && millis() - _timePr > _timeHold) {
        Numb = _NumberKey[_type][_numbNow];
        Char = _CharKey[_type][_numbNow];
        _forLongPr = 255;
        _timePr = millis();

        state = ON_PRESS_LONG;
        _forReturn = true;
    }

    // Сохранение нажатой кнопки
    _numbWas = _numbNow;
    return _forReturn;
}