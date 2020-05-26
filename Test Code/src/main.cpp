#include <Arduino.h>
#include <soc/rtc_wdt.h>

// Выводы МК, к которым подключены реле, кнопки и пьезодинамик
#define RELE0_PIN   25
#define RELE1_PIN   26
// #define BUTTON0_PIN 12
#define BUTTON1_PIN 13
#define TONE_PIN    27

/* 
Arduino 
    A4 - SDA
    A5 - SCL
ESP32
    D22 - SCL
    D21 - SDA
    D23 - MOSI
    D19 - MISO
    D18 - SCK
    D5  - CS (для SD) 
ESP8266
    D1 - SCL
    D2 - SDA
STM32F1
    B6 - SCL1
    B7 - SDA1
*/

// extern int __bss_end;
// extern void *__brkval;

// // Функция, возвращающая количество свободного ОЗУ (RAM)
// int memoryFree()
// {
//   int freeValue;
//   if((int)__brkval == 0) {freeValue = ((int)&freeValue) - ((int)&__bss_end);}
//   else {freeValue = ((int)&freeValue) - ((int)__brkval);}
//   return freeValue;
// }

// int serialReadInt(String say)
// {
//   Serial.println(say);
//   while(!Serial.available()) {}
//   return Serial.readString().toInt();
// }
int i = 0;

void open()
{
  noInterrupts();                                 // Борьба с дребезгом 
  delayMicroseconds(16383);
  interrupts();

  Serial.println(i);
  for (int y = 0; y < 200; y++) 
  {
    delayMicroseconds(1000);
  }
  for (int y = 0; y < 200; y++) 
  {
    delayMicroseconds(1000);
  }
  i++;
  Serial.println(i);
  // // lcd.clear();
  // // lcd.print("Open");
  
  // // if (digitalRead(BUTTON0_PIN) == HIGH) {digitalWrite(RELE0_PIN, LOW);}
  // digitalWrite(RELE1_PIN, LOW);

  // ledcAttachPin(TONE_PIN, 0);                     // Включаем пьезодинамик
  // for (int i = 0; i < 5000; i++) 
  // {
  //   delayMicroseconds(100);
  //   rtc_wdt_feed();
  // }
  // ledcDetachPin(TONE_PIN);

  // // digitalWrite(RELE0_PIN, HIGH);                   // Закрытие двери
  // digitalWrite(RELE1_PIN, HIGH);   

  // // Interface::goHome();
  // // rtc_wdt_enable();
  // // rtc_wdt_protect_on();
  // // attachInterrupt(digitalPinToInterrupt(BUTTON0_PIN), open, RISING);       
  // // attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), open, RISING);
}

void setup() 
{
  Serial.begin(115200);

  // while (!Serial) {}
  // pinMode(13, INPUT_PULLDOWN);
  // pinMode(14, INPUT_PULLDOWN);

  // rtc_wdt_protect_off();
  // rtc_wdt_disable();
  // rtc_wdt_set_length_of_reset_signal(RTC_WDT_SYS_RESET_SIG, RTC_WDT_LENGTH_3_2us);
  // rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_OFF);
  // rtc_wdt_set_time(RTC_WDT_STAGE0, 7000);
  // rtc_wdt_enable();
  // rtc_wdt_protect_on();
  if (rtc_wdt_is_on()) {Serial.println("WDT's working.");}
  else {Serial.println("WDT is not working.");}

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(RELE1_PIN, OUTPUT);
  digitalWrite(RELE1_PIN, HIGH);       
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), open, FALLING);
}

void loop() 
{
  // Serial.println(analogRead(13));
  // Serial.println(analogRead(14));
  // Serial.println("\n");
}