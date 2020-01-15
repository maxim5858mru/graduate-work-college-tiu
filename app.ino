#include "Arduino.h"
#include "Wire.h"
#include "Ticker.h"
#include "WiFi.h"
#include "ESPmDNS.h"
#include "esp_wps.h"

#define ESP_WPS_MODE      WPS_TYPE_PBC                                              //Для WPS
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"
#define DEFAULT_SSID      "SKUD1001"
#define DEFAULT_PASSWORD  "10012019-C"

#define EEPROM_ADDRESS 0b1010000                                                    //GPIO21 и GPIO22 - IIC

#define EEPROM_ADDRESS_CONF 0                                                       //Адреса памяти
#define EEPROM_ADDRESS_WiFi 1
#define EEPROM_ADDRESS_AP   289
#define EEPROM_ADDRESS_MDNS 385

#define EEPROM_SIZE_CONF 1                                                          //Размер секторов и ед. информации
#define EEPROM_SIZE_MDNS 10

bool NeedAP = false;                                                                //Параметры загружаемые с EEPROM
bool NeedSerial = true;
bool Buzzer = true;

String ssid;
String password;
String host;

bool EEPROMWorking = false;                                                         //Переменные используемые в ходе работы 
bool WPSWorking = false;                                                            
bool MDSNWorking = false;
bool TimerWorking = false;

Ticker Timer;
static esp_wps_config_t config;

//Прототипы функций
void ConnectToWiFi(bool fromEEPROM = false);

//Основной код
void setup()
{
    delay(5000);

    if (EEPROM_begin() != 0) {EEPROMWorking = false;}                               //Получение данных
    else 
    {
        /**/pinMode(2, OUTPUT);
        /**/digitalWrite(2, HIGH);

        EEPROMWorking = true;
        /**/EEPROM_update(EEPROM_ADDRESS_MDNS, "esp32");
        /**/EEPROM_updatebyte(EEPROM_ADDRESS_MDNS + String("esp32").length(), 0x00);
        /**/EEPROM_updatebyte(0, 0x00);
        /**/EEPROM_updatebit(0, 1, true);
        /**/EEPROM_updatebit(0, 2, true);
        String temp_read = EEPROM_read(EEPROM_ADDRESS_CONF, EEPROM_SIZE_CONF);
        NeedAP = int(temp_read[0]) & 0b1;
        NeedSerial = int(temp_read[0]) & 0b10;
        Buzzer = int(temp_read[0]) & 0b100;
    }

    if (NeedSerial) {Serial.begin(115200);}                                         //Настройка Serial
    if (Buzzer) {ledcSetup(0, 2000, 8);}                                            //Настройка ШИМ для Buzzer'а

    WiFi.mode(WIFI_STA);                                                            //Подключение к Wi-Fi используя данные EEPROM
    WiFi.disconnect();
    if (NeedAP)
    {
        ssid = EEPROM_read(EEPROM_ADDRESS_AP, 32);
        password = EEPROM_read(EEPROM_ADDRESS_AP + 32, 64);
        if (ssid.isEmpty()) {}
        else {ConnectToWiFi(true);}
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            ssid = EEPROM_read(EEPROM_ADDRESS_WiFi + (32 + 64) * i, 32);
            if (ssid.isEmpty()) continue;
            password =  EEPROM_read(EEPROM_ADDRESS_WiFi + 32 + (32 + 64) * i, 64);
        
            ConnectToWiFi(true);
            if (WiFi.status() == WL_CONNECTED) break;
        }
        if (NeedSerial & WiFi.status() != WL_CONNECTED) 
        {
            SetConnectToWiFi();
            ConnectToWiFi();
        }
        else if (WiFi.status() != WL_CONNECTED)                                      //На случий отключения Serial
        {
            ssid = DEFAULT_SSID;
            password = DEFAULT_PASSWORD;
            NeedAP = true;
            ConnectToWiFi(); 
        }
    }

    if (EEPROMWorking) 
    {
        host = EEPROM_read(EEPROM_ADDRESS_MDNS, EEPROM_SIZE_MDNS);
        if (MDNS.begin(host.c_str())) 
        {
            if (NeedSerial) Serial.println("MDNS включён, локальный адресс: http://" + String(host) + ".local/");
            MDSNWorking = true;
        }
    }
    /**/EEPROM_printmap(0, 512, 16, true);
}

void loop()
{

}

//Работа с EEPROM
int EEPROM_begin()
{
    Wire.begin();
    Wire.setClock(10000);
    Wire.beginTransmission(EEPROM_ADDRESS);
    int result = Wire.endTransmission();
    return result;
}

void EEPROM_update(int memory_address, String data)
{

    char temp_char;

    for (int i = 0; i < data.length(); i++)
    {
        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.write((memory_address + i) >> 8);
        Wire.write((memory_address + i) & 0xFF);
        Wire.endTransmission();

        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.requestFrom(memory_address + i, 1);
        temp_char = Wire.read();
        Wire.endTransmission();        

        if (temp_char != data[i]) 
        {
            Wire.beginTransmission(EEPROM_ADDRESS);
            Wire.write((memory_address + i) >> 8);
            Wire.write((memory_address + i) & 0xFF);
            Wire.write(data[i]);
            Wire.endTransmission();        
            delay(7);
        }
    } 
}

void EEPROM_updatebyte(int memory_address, byte data)
{
    int temp;

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write(memory_address >> 8);
    Wire.write(memory_address & 0xFF);
    Wire.endTransmission();

    Wire.requestFrom(memory_address, 1);
    temp = Wire.read();
    Wire.endTransmission();        

    if (temp != data) 
    {
        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.write(memory_address >> 8);
        Wire.write(memory_address & 0xFF);
        Wire.write(data);
        Wire.endTransmission();        
        delay(7);
    }
}

void EEPROM_updatebit(int memory_address, int numberbit, bool value)
{
    byte temp;
    byte now = EEPROM_read(memory_address, 1)[0];

    if (value) {temp = now | (0b1 << numberbit);}
    else {temp = ~(~now | (1 << numberbit));}
    
    EEPROM_updatebyte(memory_address, temp);
}

String EEPROM_read(int memory_address, int size)
{
    bool stop = false;
    String result;

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write(memory_address >> 8);
    Wire.write(memory_address & 0xFF);
    Wire.endTransmission();        

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.requestFrom(EEPROM_ADDRESS, size);
    while (Wire.available() != 0)
    {
        int temp = Wire.read();
        if (temp == 0) stop = true;
        if (!stop) result += (char)temp;
    }
    stop = false;
    Wire.endTransmission();
    return result;
}

void EEPROM_printmap(int start, int size, int sizeline, bool printtext)
{   
    if (Serial) 
    {
        String temp_string;
        int temp;

        for (int i = start; i < size + start;)
        {
            Serial.print(String(i) + ": ");
            for (int i2 = 0; i2 < sizeline; i2++)
            {
                temp = (int)EEPROM_read(i, 1)[0];
                if (printtext) 
                {
                    switch (temp)                       //Обработка спецсимволов
                    {
                        case 0x00:
                            temp_string += " NUL ";
                            break;
                        case 0x01:
                            temp_string += " SOH ";
                            break;
                        case 0x02:
                            temp_string += " STX ";
                            break;
                        case 0x03:
                            temp_string += " ETX ";
                            break;
                        case 0x04:
                            temp_string += " EOT ";
                            break;
                        case 0x05:
                            temp_string += " ENQ ";
                            break;
                        case 0x06:
                            temp_string += " ACK ";
                            break;
                        case 0x07:
                            temp_string += " BEL ";
                            break;
                        case 0x08:
                            temp_string += " BS ";
                            break;
                        case 0x09:
                            temp_string += " TAB ";
                            break;
                        case 0x0A:
                            temp_string += " LF ";
                            break;
                        case 0x0B:
                            temp_string += " VT ";
                            break;
                        case 0x0C:
                            temp_string += " FF ";
                            break;
                        case 0x0D:
                            temp_string += " CR ";
                            break;
                        case 0x0E:
                            temp_string += " SO ";
                            break;
                        case 0x0F:
                            temp_string += " SI ";
                            break;
                        case 0x10:
                            temp_string += " DLE ";
                            break;
                        case 0x11:
                            temp_string += " DC1 ";
                            break;
                        case 0x12:
                            temp_string += " DC2 ";
                            break;
                        case 0x13:
                            temp_string += " DC3 ";
                            break;
                        case 0x14:
                            temp_string += " DC4 ";
                            break;
                        case 0x15:
                            temp_string += " NAK ";
                            break;
                        case 0x16:
                            temp_string += " SYN ";
                            break;
                        case 0x17:
                            temp_string += " ETB ";
                            break;
                        case 0x18:
                            temp_string += " CAN ";
                            break;
                        case 0x19:
                            temp_string += " EM ";
                            break;
                        case 0x1A:
                            temp_string += " SUB ";
                            break;
                        case 0x1B:
                            temp_string += " ESC ";
                            break;
                        case 0x7F:
                            temp_string += " DEL ";
                            break;
                        default:
                            temp_string += " ";
                            temp_string += (char)temp; 
                            temp_string += " ";
                            break;
                    }
                }
                Serial.print(temp);
                Serial.print(" ");
                i++;
            }
            if (printtext) 
            {
                Serial.print((char)0x09 + temp_string);
                temp_string = "";
            }
            Serial.println();
        }
    } 
}

void ConnectToWiFi(bool fromEEPROM)
{
    if (!NeedAP & !WPSWorking)
    {
        WiFi.begin(ssid.c_str(), password.c_str());                                 //Подключение к Wi-Fi
        while (WiFi.status() != WL_CONNECTED & !NeedAP & !WPSWorking)               //Если пользователь решит использовать WPS или AP
        {
            delay(2500);
            Serial.println();
            Serial.println("Попытка подключения к WiFi: " + String(WiFi.status()));
            if (fromEEPROM) break;
            if (WiFi.status() == 4 || WiFi.status() == 6)                           //В случае ошибки предлагать выбор - повторное подключение, либо изменение настроек
            {
                Serial.println("Неправильный пароль или SSID. Подключиться к другой сети? (y/n)");
                while (!Serial.available()) {}
                if (Serial.readString() == "y") {SetConnectToWiFi();} 
                if (!NeedAP & !WPSWorking) {WiFi.begin(ssid.c_str(), password.c_str());}
            }
        }
        if (WiFi.status() == 3 & !NeedAP & !WPSWorking) 
        {
            Serial.println("ESP32 подключён к сети: " + String(ssid));
            Serial.print("IP адрес устройства: ");
            Serial.println(WiFi.localIP());
            if (!fromEEPROM) 
            {
                Serial.println("Сохранить данные? Под каким номером (1/2/3/n)");
                while (!Serial.available()) {}
                String i =  Serial.readString();
                if ((i == "1") || (i == "2") || (i = "3"))                          //Сохранение значений в EEPROM
                {
                    int num = atoi(i.c_str()) - 1;
                    EEPROM_update(EEPROM_ADDRESS_WiFi + (32+64) * num, ssid);
                    EEPROM_updatebyte(EEPROM_ADDRESS_AP + (32+64) * num + ssid.length(), 0x00);
                    EEPROM_update(EEPROM_ADDRESS_AP + 32 + (32+64) * num, password);                 
                    EEPROM_updatebyte(EEPROM_ADDRESS_AP + 32 + (32+64) * num + password.length(), 0x00);
                    EEPROM_updatebit(EEPROM_ADDRESS_CONF, 1, true);
                }
            }
        }
        else if (NeedAP) 
        {
            ssid = "";                                                              //Очистка переменных от старых данных
            password = "";
            ConnectToWiFi();
        }
    }
    else if (NeedAP)
    {
        if (ssid.isEmpty())                                                         //Проверка на наличие данных в EEPROM
        {
            Serial.println("Введите SSID создаваемой точки доступа: ");
            while (!Serial.available()){}
            ssid = Serial.readString();
            Serial.println("Введите пароль сети: ");
            while (!Serial.available()){}
            password = Serial.readString();

            if (EEPROMWorking) 
            {
                Serial.println("Сохранить данные? (y/n)");
                while (!Serial.available()) {}
                if (Serial.readString() == "y") 
                {
                    EEPROM_update(EEPROM_ADDRESS_AP, ssid);                         //Сохранение значений в EEPROM
                    EEPROM_updatebyte(EEPROM_ADDRESS_AP + ssid.length(), 0x00);
                    EEPROM_update(EEPROM_ADDRESS_AP + 32, password);                 
                    EEPROM_updatebyte(EEPROM_ADDRESS_AP + 32 + password.length(), 0x00);
                    EEPROM_updatebit(EEPROM_ADDRESS_CONF, 1, true);
                }
            }
        }

        WiFi.softAP(ssid.c_str(), password.c_str());                                //Создание сети
        IPAddress IPaddr = WiFi.softAPIP();

        Serial.print("Точка доступа запущена. IP адрес устройства: ");
        Serial.println(IPaddr);
    }
}

void SetConnectToWiFi()                                                             //Подключение к WiFi
{
    WiFi.mode(WIFI_STA);                                                            //Отключение от прошлой сети
    WiFi.disconnect();
    delay(100);

    while (1)                                               
    {
        Serial.println();
        int n = WiFi.scanNetworks();
        if (n == 0)
        {
            Serial.println("Нет доступных сетей");
            Serial.println("Включить точку доступа? (y/n)");
            while(!Serial.available()){}
            if (Serial.readString() != "y") 
            {
                NeedAP = true;
                break;
            }
            else {Serial.println("Запуск повторного поиска.");}   
        } 
        else 
        {
            Serial.println("Количество найденных сетей: " + String(n));             //Вывод доступных сетей
            for (int i = 0; i < n; ++i) 
            {
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(")");
                Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" открытая":" закрытая");
                delay(10);
            }

            Serial.println("Выберите WiFi сеть или введите 0 для включения точки доступа: ");
            while(!Serial.available()){}
            String temp = Serial.readString();

            if (temp == "-1")                                                       //WPS
            {
                NeedAP = false;
                WiFi.onEvent(WPSWork, SYSTEM_EVENT_STA_WPS_ER_SUCCESS);
                WiFi.onEvent(WPSWork, SYSTEM_EVENT_STA_WPS_ER_FAILED);
                WiFi.onEvent(WPSWork, SYSTEM_EVENT_STA_WPS_ER_TIMEOUT);
                WiFi.onEvent(WPSWork, SYSTEM_EVENT_STA_WPS_ER_PIN);
                WPSInitConfig();
                Serial.println("Включение WPS.");
                esp_wifi_wps_enable(&config);
                esp_wifi_wps_start(0);
                WPSWorking = true;
                break;
            }
            else if (temp == "0")                                                   //AP                                                  
            {
                NeedAP = true;
                ssid = "";
                password = "";
                break;
            }
            else if (atoi(temp.c_str()) / 10 != temp.length() - 1 || !(1 <= atoi(temp.c_str()) <= n)) //Ручной ввод ssid
            {
                NeedAP = false;
                ssid = temp;
                Serial.println("Вы выбрали " + String(ssid));
            }
            else                                                                    //Автоматический выбор имени сети
            {
                ssid = WiFi.SSID(atoi(temp.c_str())-1);             
                NeedAP = false;
                Serial.println("Вы выбрали " + String(ssid));
            }
            Serial.println("Выбрать другую сеть? (y/n)");
            while(!Serial.available()){}
            if (Serial.readString() != "y") {break;}   
        }
    }

    if (!NeedAP & !WPSWorking)                                                      //Ввод пароля
    {
        Serial.println("Введите пароль:");
        while(!Serial.available()){}
        password = Serial.readString();
    }    
}

void WPSInitConfig()
{
    config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
    config.wps_type = ESP_WPS_MODE;
    strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
    strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
    strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
    strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

//Прерывания
void TimerDone()
{  
    Timer.detach();
    TimerWorking = false;
}

void WPSWork(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("Устройство подключилось к " + String(WiFi.SSID()) + " с помощью WPS.");
        esp_wifi_wps_disable();
        WiFi.begin();
        Serial.print("IP адрес устройства: ");
        Serial.println(WiFi.localIP());
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("Ошибка при использовании WPS.");
        Serial.println("Повторить попытку? (y/n)");
        while (!Serial.available()) {}
        if (Serial.readString() == "y") 
        {
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&config);
            esp_wifi_wps_start(0);
        }
        else
        {
            ESP.restart();
        }
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("Вышло вермя ожидания. Повторить попытку?");
        while (!Serial.available()) {}
        if (Serial.readString() == "y") 
        {
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&config);
            esp_wifi_wps_start(0);
        }
        else
        {
            ESP.restart();
        }
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        char PIN[9];
        for (int i = 0; i < 8; i++)
        {
            PIN[i] = info.sta_er_pin.pin_code[i];
        }
        PIN[8] = '\0';
        Serial.println("WPS PIN = " + String(PIN));
        break;
    default:
        break;
    }
}

/*  EEPROM MAP          524288 bit = 512 Kbit = 65536 byte = 64 Kbyte
    Системная информация
        Переменные          [0]
            AP
            Serial
            Buzzer
            ...
        WiFi                [1, 288]
            SSID            32 * 3 символа
            password        64 * 3  
        Default AP          [289, 384]
            SSID            32
            password        64
        MDNS host           [385, 395]
        Кол. доп. устройств    
    Database            [288,]
        DatabaseLink
        Количество поль.
        Количество ком.
        WorkTime
            Имя         15
            Отчество    15
            Фамилия     15
            UID карты   10
            PIN         4
            ID-отпе.    3
            Время       
            Требования
        Комнаты    
            Тип-доп. устр.
            GPON
    Журнал
        ID пользователя
        Время/Дата
        Комната
        Возм. врем. вых.
    

*/


/*
            EEPROM_update(EEPROM_ADDRESS_WiFi + (32 + 64) * 0, "Eltex");
            EEPROM_updatebyte(EEPROM_ADDRESS_WiFi + (32 + 64) * 0 + String("Eltex").length(), 0x00);
            EEPROM_update(EEPROM_ADDRESS_WiFi + 32 + (32 + 64) * 0, "123456789_M");
            EEPROM_updatebyte(EEPROM_ADDRESS_WiFi + 32 + (32 + 64) * 0 + String("123456789_M").length(), 0x00);
*/