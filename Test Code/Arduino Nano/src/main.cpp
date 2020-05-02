#include <Arduino.h>
#include "../lib/IIC/IIC.h"

Keypad keypad(0b0100000, KB4x4);

void setup()
{
    Serial.begin(9600);
    while(!Serial);
    keypad.begin(); 
}

void loop()
{
    if (keypad.read())
    {
        Serial.println("\0");
        
        if (keypad.state == ON_PRESS) {
            // печатаем номер кнопки и её символ в последовательный порт
            Serial.print("Key is press ");
            Serial.print(keypad.Numb);
            Serial.print(" = \"");
            Serial.print(keypad.Char);
            Serial.println("\"");
        }
        // определяем отжатие кнопки
        if (keypad.state == ON_RELEASE) {
            // печатаем номер кнопки и её символ в последовательный порт
            Serial.print("Key is release ");
            Serial.print(keypad.Numb);
            Serial.print(" = \"");
            Serial.print(keypad.Char);
            Serial.println("\"");
        }
        // определяем зажатие кнопки на 3 секунды
        if (keypad.state == ON_PRESS_LONG) {
            // печатаем номер кнопки и её символ в последовательный порт
            Serial.print("Key on long press ");
            Serial.print(keypad.Numb);
            Serial.print(" = \"");
            Serial.print(keypad.Char);
            Serial.println("\"");
        }

    }

    delay(200);
}