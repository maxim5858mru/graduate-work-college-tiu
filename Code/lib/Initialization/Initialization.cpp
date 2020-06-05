#include "Initialization.h"

/** Вывод процентов
 * @param cent - процент загрузки
 */
void writeLoadCent(int cent) {
    if (cent < 100) {
        lcd.setCursor(9, 0);
    } else {
        lcd.setCursor(8,0);
    }
    lcd.print((String)cent);
}