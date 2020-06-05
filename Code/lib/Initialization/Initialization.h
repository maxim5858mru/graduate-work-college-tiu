#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

/** Вывод процентов
 * @param cent - процент загрузки
 */
void writeLoadCent(int cent);

#endif