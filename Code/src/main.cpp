#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "../lib/EEPROM/EEPROM.h"
#include "../lib/Clock/Clock.h"
#include "../lib/Keypad/Keypad.h"
#include "../lib/Timer/Timer.h"
#include "../lib/Network/Network.h"
#include "../lib/Routing/Routing.h"
#include "../lib/Interface/Interface.h"
#include "../lib/Initialization/Initialization.h"

// Идентификаторы настроек
#define NeedSerial settings[0]
#define Buzzer settings[1]
#define WiFiAP settings[2]
#define ShowError settings[3]

// Идентификаторы флагов
#define SDWorking flags[0]
#define MDSNWorking flags[1]
#define SPIFFSWorking flags[2]

// Параметры загружаемые с EEPROM
bool settings[8] = {
	true,                                         // UART интерфейс
	true,                                         // Динамик
	true,                                         // Точка доступа
	true,                                         // Вывод ошибок через UART
	true,                                         // Дверь №1
	true                                          // Дверь №2
};

// Флаги состояния
bool flags[3] = {
	false,                                        // SD
	false,                                        // MDNS
	false										  // SPIFFS
};

String host;

EEPROM memory(0x50, 10000);                       // Память
Keypad keypad(0x20, KB4x4);                       // Клавиатура
RFID rfid(14, 2);	                              // Считыватель RFID карт
FingerPrint fingerprint(&Serial2);				  // Сканер отпечатков пальцев
Clock RTC(0x68, false);                           // RTC часы
Timer timer;                                      // Таймер
LiquidCrystal_I2C lcd(0x27, 16, 2);               // Дисплей
AsyncWebServer server(80);                        // Веб сервер
WiFiClient http;                                  // Клиент для различных обращений к другим серверам

void setup()
{
	/* 1 этап - Инициализация обязательных компонентов */
	lcdInit();
	componentsInit();

	/* 2 этап - Псевдо-прерывание */
	// setInterrupt();
 	writeLoadCent(60);

	/* 3 этап - Настройка Wi-Fi */
	if (memory.status) {
		Network::setupWiFi();
	} 
	else {
		Network::presetupWiFi();                  // Создание точки доступа с предустановленными значениями
	}            
	setLoadFlag(6, WiFi.isConnected()?"+":"-");
	writeLoadCent(70);

	/* 4 этап - Web-сервер */
	setLoadFlag(5, RTC.begin()?"+":"-");		  // Инициализация часов
	if (WiFi.isConnected()) {					  // Синхронизация часов
		RTC.sync(http);
	}
	writeLoadCent(72);

	// Настройка Web сервера
	if (SPIFFSWorking && WiFi.status() == WL_CONNECTED) 
	{
		if (memory.status)                        // Включение MDNS
		{
			host = memory.readString(10, 385);
			if (!host.isEmpty() && MDNS.begin(host.c_str())) 
			{
				if (NeedSerial) {
					Serial.println("MDNS is enabled, the local address: http://" + String(host) + ".local/");
				}
				MDSNWorking = true;
			}
		}
		writeLoadCent(75);

		setRouting();							  // Настройка маршрутизации сервера
		writeLoadCent(85);

		// Включение сервера
		server.begin();
		if (MDSNWorking) {
			MDNS.addService("http", "tcp", 80);
		}
		writeLoadCent(90);
	}
	showResultTest();
	writeLoadCent(100);
	delay(500);

	// Сброс дисплея
	lcd.noBacklight();
	Interface::goHome();
}

void loop()
{
	// Проверка расстояния 
	if (Interface::getDistance(15, 4) < 60 || timer.timerIsWorking())      // Для энергии на подсветке
	{
		// Таймер постоянно сбрасывается, если человек стоит поблизости
		if (Interface::getDistance(15, 4) < 60) 
		{
			timer.timerCheckAndStop();
			timer.timerStart(30);
		}
		
		lcd.backlight();

		// Получение значения с клавиатуры
		keypad.read();                                
		if (keypad.state == ON_PRESS) switch (keypad.Numb)
		{
			//Режим ввода ПИН-кода
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
				Interface::checkPassword(Interface::readPassword());
				Interface::goHome();

				break;
			//Вход в меню
			case 10:
			case 11:
			case 12:
			case 13:
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Menu");
			default:
				break;
		}

		// RFID
		if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {  // Если карта поднесена только что и считывание удалось, то выполняем проверку
            Interface::checkAndGetRFID();
        }
	}
	else {
		lcd.noBacklight();
	}
}