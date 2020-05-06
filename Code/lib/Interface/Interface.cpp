#include "Interface.h"

/** Измерение растояния до объекта, с помощью ультразвукового датчика
 * @warning желательно чтобы объект меньше поглощал звуков
 * @param trig - пин для отправки сигнала активации
 * @param echo - пин для приёма ШИМ сигнала от датчика
 * @return примерное растояние до объекта
 */
long Interface::getDistance(uint8_t trig, uint8_t echo)
{
  long duration, cm;

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);

  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  //  Время задержки акустического сигнала на эхолокаторе.
  duration = pulseIn(echo, HIGH);

  // Теперь осталось преобразовать время в расстояние
  cm = (duration / 2) / 29.1;

  // Задержка между измерениями для корректной работы скеча
  delay(250);
  return cm;
}

/** Сброс экрана
 * @param lcd - ссылка на дисплей 
 */
void Interface::goHome(LiquidCrystal_I2C &lcd)
{          
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Good Day");
}

/** Вывод пароля (точнее его изменения) на дисплей
 * @param password - вводимый пароль
 * @param becomeMore - пароль увеличился или уменьшился?
 * @param lcd - ссылка на дисплей 
 */
void Interface::showPassword(String password, bool becomeMore, LiquidCrystal_I2C &lcd)
{
  if (becomeMore)
  {
    lcd.setCursor(password.length() - 1, 1);
    lcd.print(password[password.length() - 1]);
    delay(250);

    lcd.setCursor(password.length() - 1, 1);
    lcd.print("*");
  }
  else
  {
    lcd.setCursor(password.length(), 1);
    lcd.print(" ");
  }
}

/** Проверка пароля
 * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
 * @remarks Открытие двери происходит внутри функции
 * @param password - проверяемый пароль
 * @param lcd - ссылка на дисплей
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkPassword(String password, LiquidCrystal_I2C &lcd)
{
  lcd.clear();

  if (password.length() != 0) 
  {
    lcd.print("Open");
    delay(2000);

    return true;
  }
  else 
  {
    lcd.print("Wrong password");
    delay(2000);

    return false;
  }
}

/** Получение ID (NUID) карты RFID
 * @warning из-за ограниченного размера переменной функуия совестима только с картами с размером 1кб
 * @param rfid - сылка на считыватель карт
 * @return NUID карты RFID
 */
int Interface::getRFID(MFRC522 &rfid)
{
  int key = 0;

  for (int i = 0; i < 4; i++)
  {
    key |= rfid.uid.uidByte[i] << ((3-i)*8);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return key;
}

/** Проверка NUID карты RFID
 * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных
 * @remarks Открытие двери происходит внутри функции
 * @param password - проверяемый NUID
 * @param lcd - ссылка на дисплей
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkRFID(int key, LiquidCrystal_I2C &lcd)
{
  lcd.clear();

  if (key == 0xE98F8499)
  {
    lcd.print("Open");
    delay(2000);

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Good Day");

    return true;
  }
  else
  {
    lcd.print("Access Deny");
    delay(2000);

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Good Day");

    return false;
  }
}