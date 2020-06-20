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
 * @return номер отпечатка, если нету или не прошёл проверку, то возвращается 0xFFFF
 */
uint16_t FingerPrint::read()
{
    if (!getImage() && !image2Tz() && !fingerFastSearch()) {
        delay(50);
        return fingerID;
    }
    else {
        delay(50);
        return 0xFFFF;
    }
}