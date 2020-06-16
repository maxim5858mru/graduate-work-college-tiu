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
    int pin;                                      // Оперделение номе вывода МК для реле
    switch (door)
    {
    case 0:
        pin = 25;
        break;
    default:
        pin = 26;
        break;
    }

    lcd.clear();                                  // Вывод информации на дисплей
    lcd.backlight();
    lcd.print("Open");

    digitalWrite(pin, LOW);                       // Открытие двери
    ledcAttachPin(TONE_PIN, 0);                   // Включаем пьезодинамик
    delay(openTime * 100);
    ledcDetachPin(TONE_PIN);
    digitalWrite(pin, HIGH);                      // Закрытие двери

    goHome();                                     // Сброс дисплея
    if (getDistance(15, 4) > 60) {
        lcd.noBacklight();
    }
}

// Отказ в доступе
void Interface::accessDeny()
{
    lcd.clear();
    lcd.print("Access Deny");
    delay(2000);

    if (WiFi.isConnected() && http.connect(databaseURL.c_str(), 8000)) {
        DynamicJsonDocument doc(500);
        doc["type"] = 2;
        doc["seconds"] = second();
        doc["minutes"] = minute();
        doc["hour"] = hour();
        doc["day"] = day();
        doc["month"] = month();
        doc["year"] = year();
        doc["door"] = 0;
        doc["userID"] = 0;
        doc["user"] = 1;
        
        http.println("POST /api/v1/database/logs/create HTTP/1.1");
        http.println("Host: " + databaseURL + ":8000");
        http.println("Content-Type: application/json");
        http.print("Content-Length: ");
        http.println(measureJson(doc));
        http.println("Connection: close\r\n");
        serializeJson(doc, http);
        http.readString();
        http.stop();
    }
    goHome();
}

/** Отказ в доступе
 * @param id - номер записи пользователя
 * @param byTime - причиной отказа является время?
 */
void Interface::accessDeny(int id, bool byTime)
{
    lcd.clear();
    if (byTime) {
        lcd.print("Wrong Time");
    }
    else {
        lcd.print("Access Deny");
    }
    delay(2000);

    if (WiFi.isConnected() && http.connect(databaseURL.c_str(), 8000)) {
        DynamicJsonDocument doc(500);
        doc["type"] = byTime?4:3;
        doc["seconds"] = second();
        doc["minutes"] = minute();
        doc["hour"] = hour();
        doc["day"] = day();
        doc["month"] = month();
        doc["year"] = year();
        doc["door"] = 0;
        doc["userID"] = id;
        doc["user"] = 1;
        
        http.println("POST /api/v1/database/logs/create HTTP/1.1");
        http.println("Host: " + databaseURL + ":8000");
        http.println("Content-Type: application/json");
        http.print("Content-Length: ");
        http.println(measureJson(doc));
        http.println("Connection: close\r\n");
        serializeJson(doc, http);
        http.readString();
        http.stop();
    }
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
        DynamicJsonDocument json(550);

        File folder;
        File file;
        
        if (SDWorking) {
            // Открытие папки с записями базы данных
            folder = SD.open("/Database");

            // Перебор записей. К сожалению, всю базу не запихнуть в буфер, по этому перебор записей пользователей
            file = folder.openNextFile();
            if (!file) {                          // Если файлов нет, сразу уведомляем об этом
                lcd.clear();
                lcd.print("Database is");
                lcd.setCursor(0, 1);
                lcd.print("empty");
                delay(5000);
                return false;
            }

            while (file)
            {
                deserializeJson(json, file);

                // Первичная проверка
                if (json["methodPIN"]) {
                    String JPIN = json["pin"];
                    if (password == JPIN) {
                        break;
                    }
                }

                file = folder.openNextFile();
            }
        }
        else if (WiFi.isConnected()) {
            int z = 1;                            // Счётчик для Web-database
            while (z < 10)
            {
                // Отправка запроса
                if (!http.connect(databaseURL.c_str(), 8000)) {
                    lcd.clear();
                    lcd.print("Fall to connect");
                    lcd.setCursor(0,1);
                    lcd.print("database");
                    delay(5000);
                    goHome();
                    return false;
                }

                http.printf("GET /api/v1/database/users/detail/%i HTTP/1.1\r\nHost: %s:8000\r\n\r\n", z, databaseURL.c_str());
                delay(200);

                if (http.find("\r\n\r\n")) {
                    deserializeJson(json, http);

                    if (json["methodPIN"]) {
                        String JPIN = json["pin"];
                        if (password == JPIN) {
                            break;
                        }
                    }
                }

                z++;
            }
        }
        else {                                    // Без локальной или сетевой базы данных продолжать попытки бесмысленно
            lcd.clear();
            lcd.print("Database is");
            lcd.setCursor(0, 1);
            lcd.print("empty");
            delay(5000);
            return false;
        }

        // Контрольная проверка, далее идёт работа на выбывание
        String JPIN = json["pin"];
        if (!json["methodPIN"] || password != JPIN) {
            accessDeny();
            return false;
        }

        //  Проверка на время
        int sMin = json["startTime"];
        sMin %= 100;
        int sHour = json["startTime"];
        sHour /= 100;
        int eMin = json["endTime"];
        eMin %= 100;
        int eHour = json["endTime"];
        eHour /= 100;
        if (!RTC.compare(sHour, sMin, eHour, eMin)) {
            accessDeny(json["id"], true);
            return false;
        }

        // Дополнительные проверки

        // RFID
        if (json["methodRFID"]) {
            lcd.clear();
            lcd.print("Attach your pass");

            // Импорт массива
            byte JRFID[10] = {
                json["rfid1"], 
                json["rfid2"], 
                json["rfid3"], 
                json["rfid4"],                        
                json["rfid5"], 
                json["rfid6"], 
                json["rfid7"], 
                json["rfid8"],
                json["rfid9"], 
                json["rfid10"]
            };

            // Получение значения от RFID
            while (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {}
            byte RFID[10];
            for (int i = 0; i < 10; i++) {
                RFID[i] = rfid.uid.uidByte[i];
            }

            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
            delay(100);
            
            // Проверка
            for (int i = 0; i < 10; i++) {
                if (JRFID[i] != RFID[i]) {
                    accessDeny(json["id"], false);
                    return false;
                }
            }
        }

        // Fingerprint
        if (json["methodFPID"]) {                 
            uint16_t tempID = 0xFFFF;             // Переменная для хранения ID отпечатка

            lcd.clear();
            lcd.print("Put your finger");

            while (tempID == 0xFFFF) {            // Ожидание отпечатка пальца
                tempID = fingerprint.read();
                delay(50);
            }

            if (tempID == 0xFF00 || tempID != json["fingerprintID"]) {     // Неправильный отпечаток
                accessDeny(json["id"], false);
                return false;
            }
        }

        open(json["door"]); // Открытие двери
        return true;
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
    DynamicJsonDocument json(550);

    File folder;
    File file;

    // Получение данных
    byte RFID[10];
    for (int i = 0; i < 10; i++) {
        RFID[i] = rfid.uid.uidByte[i];
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
    if (SDWorking) {
        // Открытие папки с записями базы данных
        folder = SD.open("/Database");

        // Перебор записей. К сожалению, всю базу не запихнуть в буфер, по этому перебор записей пользователей
        file = folder.openNextFile();
        if (!file) {                          // Если файлов нет, сразу уведомляем об этом
            lcd.clear();
            lcd.print("Database is");
            lcd.setCursor(0, 1);
            lcd.print("empty");
            delay(5000);
            goHome();
            return false;
        }

        while (file)
        {
            deserializeJson(json, file);

            // Первичная проверка
            if (json["methodRFID"]) {
                bool goWhile = true;

                // Импорт массива
                byte JRFID[10] = {
                    json["rfid1"], 
                    json["rfid2"], 
                    json["rfid3"], 
                    json["rfid4"],                        
                    json["rfid5"], 
                    json["rfid6"], 
                    json["rfid7"], 
                    json["rfid8"],
                    json["rfid9"], 
                    json["rfid10"]
                };

                for (int i = 0; i < 10; i++) {
                    if (JRFID[i] != RFID[i]) {
                        goWhile = false;
                    }
                }

                if (goWhile) {
                    break;
                }
            }

            file = folder.openNextFile();
        }
    }
    else if (WiFi.isConnected()) {
        int z = 1;                                // Счётчик для Web-database
        while (z < 10)
        {
            // Отправка запроса
            if (!http.connect(databaseURL.c_str(), 8000)) {
                lcd.clear();
                lcd.print("Fall to connect");
                lcd.setCursor(0,1);
                lcd.print("database");
                delay(5000);
                goHome();
                return false;
            }
            http.printf("GET /api/v1/database/users/detail/%i HTTP/1.1\r\nHost: %s:8000\r\n\r\n", z, databaseURL.c_str());
            delay(200);

            if (http.find("\r\n\r\n")) {
                deserializeJson(json, http);

                if (json["methodRFID"]) {
                    bool goWhile = true;         // Флаг для реализации выхода из цикла перебора записей
                    byte JRFID[10] = {
                        json["rfid1"], 
                        json["rfid2"], 
                        json["rfid3"], 
                        json["rfid4"],                        
                        json["rfid5"], 
                        json["rfid6"], 
                        json["rfid7"], 
                        json["rfid8"],
                        json["rfid9"], 
                        json["rfid10"]
                    };

                    for (int i = 0; i < 10; i++) {
                        if (JRFID[i] != RFID[i]) {
                            goWhile = false;
                            break;
                        }
                    }

                    if (goWhile) {
                        break;
                    }

                }
            }

            z++;
        }
    }
    else {                                        // Без локальной или сетевой базы данных продолжать попытки бесмысленно
        lcd.clear();
        lcd.print("Database is");
        lcd.setCursor(0, 1);
        lcd.print("empty");
        delay(5000);
        return false;
    }

    // Контрольная проверка, далее идёт работа на выбывание
    byte JRFID[10] = {
        json["rfid1"], 
        json["rfid2"], 
        json["rfid3"], 
        json["rfid4"],                        
        json["rfid5"], 
        json["rfid6"], 
        json["rfid7"], 
        json["rfid8"],
        json["rfid9"], 
        json["rfid10"]
    };

    for (int i = 0; i < 10; i++) {
        if (JRFID[i] != RFID[i]) {
            accessDeny();
            return false;
        }
    }

    //  Проверка на время
    int sMin = json["startTime"];
    sMin %= 100;
    int sHour = json["startTime"];
    sHour /= 100;
    int eMin = json["endTime"];
    eMin %= 100;
    int eHour = json["endTime"];
    eHour /= 100;
    if (!RTC.compare(sHour, sMin, eHour, eMin)) {
        accessDeny(json["id"], true);
        return false;
    }

    // Дополнительные проверки

    // PIN
    if (json["methodPIN"]) {
        lcd.clear();
        lcd.print("Enter your PIN");

        String PIN = readPassword(true);
        String JPIN = json["pin"];
        if (PIN != JPIN) {
            accessDeny(json["id"], false);
            return false;
        }
    }

    // Fingerprint
    if (json["methodFPID"]) {                 
        uint16_t tempID = 0xFFFF;                 // Переменная для хранения ID отпечатка

        lcd.clear();
        lcd.print("Put your finger");

        while (tempID == 0xFFFF) {                // Ожидание отпечатка пальца
            tempID = fingerprint.read();
            delay(50);
        }

        if (tempID == 0xFF00 || tempID != json["fingerprintID"]) {         // Неправильный отпечаток
            accessDeny(json["id"], false);
            return false;
        }
    }

    open(json["door"]);                           // Открытие двери
    return true;
}

/** Проверка ID скана отпечатка
 * @param id - для передачи функции уже считаного значения
 * @return !!!!!Учитывая что открытие двери просходит внутри функции, для чего возратное значение не знаю
 */
bool Interface::checkFingerID(uint16_t id)
{
    DynamicJsonDocument json(550);

    File folder;
    File file;
    
    if (SDWorking) {
        // Открытие папки с записями базы данных
        folder = SD.open("/Database");

        // Перебор записей. К сожалению, всю базу не запихнуть в буфер, по этому перебор записей пользователей
        file = folder.openNextFile();
        if (!file) {                              // Если файлов нет, сразу уведомляем об этом
            lcd.clear();
            lcd.print("Database is");
            lcd.setCursor(0, 1);
            lcd.print("empty");
            delay(5000);
            return false;
        }

        while (file)
        {
            deserializeJson(json, file);

            // Первичная проверка
            if (json["methodFPID"] == true && id == json["fingerprintID"]) {
                break;
            }

            file = folder.openNextFile();
        }
    }
    else if (WiFi.isConnected()) {
        int z = 1;                                // Счётчик для Web-database
        while (z < 10)
        {   
            // Отправка запроса
            if (!http.connect(databaseURL.c_str(), 8000)) {
                lcd.clear();
                lcd.print("Fall to connect");
                lcd.setCursor(0,1);
                lcd.print("database");
                delay(5000);
                goHome();
                return false;
            }

            http.printf("GET /api/v1/database/users/detail/%i HTTP/1.1\r\nHost: %s:8000\r\n\r\n", z, databaseURL.c_str());
            delay(200);

            if (http.find("\r\n\r\n")) {
                deserializeJson(json, http);

                // Первичная проверка
                if (json["methodFPID"] == true && id == json["fingerprintID"]) {
                    break;
                }
            }
            z++;
        }
    }
    else {                                        // Без локальной или сетевой базы данных продолжать попытки бесмысленно
        lcd.clear();
        lcd.print("Database is");
        lcd.setCursor(0, 1);
        lcd.print("empty");
        delay(5000);
        return false;
    }

    // Контрольная проверка, далее идёт работа на выбывание
    if (id != json["fingerprintID"]) {
        accessDeny();
        return false;
    }

    //  Проверка на время
    int sMin = json["startTime"];
    sMin %= 100;
    int sHour = json["startTime"];
    sHour /= 100;
    int eMin = json["endTime"];
    eMin %= 100;
    int eHour = json["endTime"];
    eHour /= 100;
    if (!RTC.compare(sHour, sMin, eHour, eMin)) {
        accessDeny(json["id"], true);
        return false;
    }

    // Дополнительные проверки

    // PIN
    if (json["methodPIN"]) {
        lcd.clear();
        lcd.print("Enter your PIN");

        String PIN = readPassword(true);
        String JPIN = json["pin"];
        if (PIN != JPIN) {
            accessDeny(json["id"], false);
            return false;
        }
    }

    // RFID
    if (json["methodRFID"]) {
        lcd.clear();
        lcd.print("Attach your pass");

        // Импорт массива
        byte JRFID[10] = {
            json["rfid1"], 
            json["rfid2"], 
            json["rfid3"], 
            json["rfid4"],                        
            json["rfid5"], 
            json["rfid6"], 
            json["rfid7"], 
            json["rfid8"],
            json["rfid9"], 
            json["rfid10"]
        };

        // Получение значения от RFID
        while (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {}
        byte RFID[10];
        for (int i = 0; i < 10; i++) {
            RFID[i] = rfid.uid.uidByte[i];
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        delay(100);
        
        // Проверка
        for (int i = 0; i < 10; i++) {
            if (JRFID[i] != RFID[i]) {
                accessDeny(json["id"], false);
                return false;
            }
        }
    }

    open(json["door"]); // Открытие двери
    return true;
}