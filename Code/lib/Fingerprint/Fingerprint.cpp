#include "Fingerprint.h"

/** Инициализация сканера отпечатков пальцев
 * @param baud - скорость обмена
 * @return результат авторизации 
 */
bool FingerPrint::init(uint32_t baud)
{
    begin(baud);
    delay(5);
    status = verifyPassword(); 
    return status;
}

/** Полный процесс получения номера отпечатка пальцев
 * @return номер отпечатка, если нету - возвращается 0xFFFF, если е прошёл проверку - 0xFF00
 */
uint16_t FingerPrint::read()
{
    // Функции сканера при успехе возвращают 0, то есть false 
    if (!getImage() && !image2Tz()) {             // Проверка на наличие нового отпечатка
        if (!fingerFastSearch()) {                // Поиск совпадений
            delay(50);
            return fingerID;
        }
        else {
            delay(50);
            return 0xFF00;
        }
        delay(50);
        return fingerID;
    }
    else {
        delay(50);
        return 0xFFFF;
    }
}