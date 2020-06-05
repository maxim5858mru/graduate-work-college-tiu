#include "Interface.h"

/** Измерение расстояния до объекта, с помощью ультразвукового датчика
 * @warning желательно чтобы объект меньше поглощал звуков
 * @param trig - пин для отправки сигнала активации
 * @param echo - пин для приёма ШИМ сигнала от датчика
 * @return примерное расстояние до объекта
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

    // Задержка между измерениями для корректной работы скетча
    delay(250);
    return cm;
}

// Сброс экрана
void Interface::goHome()
{
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Good Day");
}

/** Открытие двери
 * @param door - открываемая дверь
 */
void Interface::open(uint8_t door)
{
    int pin = -1; // Оперделение номе вывода МК для реле
    if (door == 0) {
        pin = RELE0_PIN;
    }
    else if (door == 1) {
        pin = RELE1_PIN;
    }

    lcd.clear(); // Вывод информации на дисплей
    lcd.backlight();
    lcd.print("Open");

    if (door != -1) {
        digitalWrite(pin, LOW);
    }                           // Открытие двери
    ledcAttachPin(TONE_PIN, 0); // Включаем пьезодинамик
    delay(5000);
    ledcDetachPin(TONE_PIN);
    if (door != -1) {
        digitalWrite(pin, HIGH);
    } // Закрытие двери

    goHome(); // Сброс дисплея
    if (getDistance(15, 4) > 60) {
        lcd.noBacklight();
    }
}

/** Отказ в доступе
 * @param - причиной отказа является время?
 */
void Interface::accessDeny(bool byTime)
{
    lcd.clear();
    if (byTime) {
        lcd.print("Wrong Time");
    }
    else {
        lcd.print("Access Deny");
    }
    delay(2000);

    goHome();
}

/** Вывод пароля (точнее его изменения) на дисплей
 * @param password - вводимый пароль
 * @param becomeMore - пароль увеличился или уменьшился?
 */
void Interface::showPassword(String password, bool becomeMore)
{
    if (becomeMore) {
        lcd.setCursor(password.length() - 1, 1);
        lcd.print(password[password.length() - 1]);
        delay(250);

        lcd.setCursor(password.length() - 1, 1);
        lcd.print("*");
    }
    else {
        lcd.setCursor(password.length(), 1);
        lcd.print(" ");
    }
}

/** Считывание пароля
 * @param nextAuth - является ли вызов функции вторичной проверкой?
 * @return введённый PIN в виде строки
 */
String Interface::readPassword(bool nextAuth)
{
    String password;

    lcd.clear();
    lcd.print("PIN");

    // Ввод кода
    while (nextAuth) {                            // Нужно получить первую цифру от пользователя, чтобы следующий цикл работал
        keypad.read();
        if (keypad.state == ON_PRESS) {
            if (keypad.Numb == 10 || keypad.Numb == 11 || keypad.Numb == 12 || keypad.Numb == 13 || keypad.Numb == 14 || keypad.Numb == 15) {
                return "";
            }
            else {
                break;
            }
        }
    }
    password = keypad.Char;
    Interface::showPassword(password, true);

    while (password.length() > 0 && password.length() < 16) {              // Режим ввода ПИН-кода будет, пока пользователь не введёт или сбросит пароль
        keypad.read(); // Получение нового значения, прошлое уже записано ранее
        if (keypad.state == ON_PRESS) {
            switch (keypad.Numb) {
            // ПИН-код может состоять только из чисел
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                password += keypad.Char;
                Interface::showPassword(password, true);
                break;
            // Кнопка * - удаляет символ
            case 14:
                password.remove(password.length() - 1);
                Interface::showPassword(password, false);
                break;
            // Кнопка # - окончание ввода пароля
            case 15:
                return password;
                break; // На Хабре уже была статья на этот счёт. Так что break лучше ставить после каждого case
            // A B C D - сбрасывают пароль
            default:
                return "";
                break;
            }
        }
    }

    return password;
}

/** Проверка пароля
 * @remarks !!!!!Функция ещё не законченна, нужно подключить базу данных 
 * @remarks Открытие двери происходит внутри функции
 * @param password - проверяемый пароль
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkPassword(String password)
{
    if (password.length() == 0) {
        goHome();
    }
    else {
        // Открытие папки с записями базы данных
        File folder = SD.open("/Database");
        if (!folder || !folder.isDirectory()) {
            // !!!!! Скачиваем базу данных
        }

        // Перебор записей. К сожалению, всю базу не запихнуть в буфер, по этому перебор записей пользователей
        File file = folder.openNextFile();
        while (file) {
            DynamicJsonDocument json(600); // Буфер для файла
            deserializeJson(json, file);

            // Объявление переменных для файла
            JsonArray JMethod = json["Method"];
            if (JMethod[0]){                      // Массив флагов методов авторизации, которыми должен воспользоваться пользователь
                if (password != json["PIN"]) {
                    file = folder.openNextFile();
                }
                else {
                    break;
                }
            }

            //  Проверка на время
            int sMin = json["Start Time"];
            sMin %= 100;
            int sHour = json["Start Time"];
            sHour /= 100;
            int eMin = json["End Time"];
            eMin %= 100;
            int eHour = json["End Time"];
            eHour /= 100;
            if (!RTC.compare(sHour, sMin, eHour, eMin)) {
                accessDeny(true);
                return false;
            }

            if (JMethod[1]) {                     // Fingerprint
                lcd.clear();
                lcd.println("Put your finger");
            }

            if (JMethod[2]) {                     // RFID
                lcd.clear();
                lcd.println("Attach your pass");
            }

            open(json["Door"]); // Открытие двери
            return true;
        }

        accessDeny(false); // Отказ, если PIN не найден в базе
        return false;
    }
}

/** Получение и проверка NUID карты RFID
 * @remarks !!!!!Функция ещё не законченна, нужно реализовать загрузку базы с резервной копии
 * @remarks ↓↓↓ Алгоритм ↓↓↓
 * открой папку
 * это не папка или она вообще не существует -> скачивай базу данных
 * открой следующий в папке (первый) файл
 * цикл перебора файлов
 *   буфер и переменные JSON
 *   в записи нет RFID авторизации -> новый файл
 *     не прошёл проверку массива NUID -> открой новый файл и continue
 *     не попал по времени -> вывод ошибки по времени и return
 *     есть еще тербования (методы) -> проверяй. Не получилось? -> вывод ошибки и return
 *     открой дверь и return  
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkAndGetRFID()
{
    byte RFID[10];
    for (int i = 0; i < 10; i++) {
        RFID[i] = rfid.uid.uidByte[i];
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(100);

    // Открытие папки базы данных, точнее папки с её записями
    File folder = SD.open("/Database");
    if (!folder || !folder.isDirectory()) {   
        // На случай если папку удалят, либо если карта памяти умрёт
        // !!!!! Скачиваем базу данных
    }

    // Перебор записей. К сожалению, всю базу не запихнуть в буфер, по этому перебор записей пользователей
    File file = folder.openNextFile();
    while (file) {
        DynamicJsonDocument json(600); // Буфер для файла. Благодаря ему я использую этот тупой костыль-метод
        deserializeJson(json, file);

        // Объявление переменных для файла
        JsonArray JMethod = json["Method"];
        if (JMethod[2]) {                         // Массив Method указывает, какими методами авторизации должен воспользоваться пользователь
        
            bool result = true;
            JsonArray JRFID = json["RFID"];       // NUID RFID карты состоит из 10 байтов

            for (int i = 0; i < 10; i++) {
                if (JRFID[i] != RFID[i]) {
                    file = folder.openNextFile(); // Каждая запись пользователя в отдельном файле, спасибо буфер...
                    result = false;
                    break;
                }
            }

            if (!result) {                        // Костыль для реализации break label цикла перебора  
                continue;
            } 

            //  Проверка на время, но тут в случае ошибки accessDeny
            int sMin = json["Start Time"];
            sMin %= 100;
            int sHour = json["Start Time"];
            sHour /= 100;
            int eMin = json["End Time"];
            eMin %= 100;
            int eHour = json["End Time"];
            eHour /= 100;
            if (!RTC.compare(sHour, sMin, eHour, eMin)) {
                accessDeny(true);
                return false;
            }

            // Проверка на доп. требования
            if (JMethod[0]) {                     // PIN
                String password = readPassword(true);
                if (password != json["PIN"]) {
                    accessDeny(false);
                    return false;
                }
            }
            // if (JMethod[1])                             // Fingerprint
            // {

            // }

            open(json["Door"]);
            return true;
        }
        else {
            file = folder.openNextFile();
        }
    }

    accessDeny(false);
    return false;
}