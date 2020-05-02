#include <Arduino.h>
#define PIN_TRIG 8
#define PIN_ECHO 9
long duration, cm;
void setup() {
  // Инициализируем взаимодействие по последовательному порту
  Serial.begin (9600);
  //Определяем вводы и выводы
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}
void loop() {
  // Сначала генерируем короткий импульс длительностью 2-5 микросекунд.
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  //  Время задержки акустического сигнала на эхолокаторе.
  duration = pulseIn(PIN_ECHO, HIGH);
  // Теперь осталось преобразовать время в расстояние
  cm = (duration / 2) / 29.1;
  Serial.print("Расстояние до объекта: ");
  Serial.print(cm);
  Serial.println(" см.");
  // Задержка между измерениями для корректной работы скеча
  delay(250);
}






// // #include <LiquidCrystal_I2C.h>
// // #include <AmperkaKB.h>
// // #include <Wire.h>

// //Для STM32 Blue Pill 
// //IIC SCL - B6
// //IIC SDA - B7
// //UART TX - A9
// //UART RX - A10
// //SPI MOSI - 
// //SPI MISO - 
// //SPI SCK - 

// // LiquidCrystal_I2C lcd(0x27, 16, 2);
// // AmperkaKB keypad(25, 26, 27, 28, 29, 30, 31, 32);

// #include <SPI.h>
// #include <MFRC522.h>

// #define SS_PIN 33
// #define RST_PIN 38
 
// // MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// // /**
// //  * Check firmware only once at startup
// //  */
// // void setup() {
// //   Serial.begin(9600);   // Initialize serial communications with the PC
// //   while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
// //   SPI.begin();          // Init SPI bus
// //   mfrc522.PCD_Init();   // Init MFRC522 module
  
// //   Serial.println(F("*****************************"));
// //   Serial.println(F("MFRC522 Digital self test"));
// //   Serial.println(F("*****************************"));
// //   mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
// //   Serial.println(F("-----------------------------"));
// //   Serial.println(F("Only known versions supported"));
// //   Serial.println(F("-----------------------------"));
// //   Serial.println(F("Performing test..."));
// //   bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
// //   Serial.println(F("-----------------------------"));
// //   Serial.print(F("Result: "));
// //   if (result)
// //     Serial.println(F("OK"));
// //   else
// //     Serial.println(F("DEFECT or UNKNOWN"));
// //   Serial.println();
// // }

// // void loop() {} // nothing to do

// long getDistance(uint8_t pin)
// {
//   long duration, cm;

//   //Сначала генерируем короткий импульс длительностью 2-5 микросекунд
//   pinMode(pin, OUTPUT);
//   digitalWrite(pin, LOW);
//   delayMicroseconds(5);
//   digitalWrite(pin, HIGH);

//   //Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
//   delayMicroseconds(10);
//   digitalWrite(pin, HIGH);

//   // Время задержки акустического сигнала на эхолокаторе.
//   pinMode(pin,INPUT);
//   duration = pulseIn(pin, HIGH);
//   cm = (duration / 2) / 29.1;
//   delay(250);
//   return cm;
// }

// void showPassword(String password)
// {
//   lcd.setCursor(0,1);
//   lcd.print("                ");
//   lcd.setCursor(0,1);
//   lcd.print(password);
// }

// void setup() 
// {
//   lcd.init();
//   keypad.begin(KB4x4);
//   Serial.begin(9600);

//   lcd.clear();
//   lcd.setCursor(4, 0);
//   lcd.print("Good Day");
// }
// void loop() 
// {
//   if (getDistance(19) < 60)   //Для экономии подсветки
//   {
//     lcd.backlight();

//     keypad.read();            //Получение значения с клавиатуры
//     if (keypad.justPressed())
//     {
//       String password = "";        //Переменные лучше не объявлять в switch, иначе break может не сработать
//       switch (keypad.getNum)
//       {
//         //Режим ввода ПИН-кода
//         case 0:
//         case 1:
//         case 2:
//         case 3:
//         case 4:
//         case 5:
//         case 6:
//         case 7:
//         case 8:
//         case 9:
//           lcd.clear();
//           lcd.setCursor(0,0);
//           lcd.print("PIN");

//           //Ввод кода
//           lcd.setCursor(0,1);
//           password = keypad.getChar;
//           showPassword(password);

//           while (password.length() > 0 || password.length() < 16)         //Режим ввода ПИН-кода будет, пока пользователь не введёт или сбросит пароль
//           {
//             keypad.read();                                                  //Получение нового значения, прошлое уже записано ранее
//             if (keypad.justPressed()) 
//             {
//               showPassword(password);

//               switch (keypad.getNum)
//               {
//                 //ПИН-код может состоять только из чисел
//                 case 0:
//                 case 1:
//                 case 2:
//                 case 3:
//                 case 4:
//                 case 5:
//                 case 6:
//                 case 7:
//                 case 8:
//                 case 9:
//                   password += keypad.getChar;
//                   break;
//                 //Кнопка * - удаляет символ
//                 case 14:
//                   password.remove(password.length() - 1);    
//                   break;
//                 //Кнопка # - окончание ввода пароля
//                 case 15:
//                   ////////////////////////////
//                   password = "";   
//                   break;    
//                 //A B C D - сбрассывают пароль   
//                 default:
//                   password = "";

//                   break;
//               }
//             }
//           }
          
//           lcd.clear();
//           lcd.setCursor(4, 0);
//           lcd.print("Good Day");

//           break;
//         //Вход в меню
//         case 10:
//         case 11:
//         case 12:
//         case 13:
//           lcd.clear();
//           lcd.setCursor(0, 0);
//           lcd.print("Menu");
//         default:
//           break;
//       }
//     }
//   }
//   else {lcd.noBacklight();}
// }