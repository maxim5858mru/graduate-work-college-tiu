#include <Arduino.h>
#include <SD.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
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
bool flags[2] = {
	false,                                        // SD
	false                                         // MDNS
};

String host;

EEPROM memory(0x50, 10000);                       // Память
Keypad keypad(0x20, KB4x4);                       // Клавиатура
RFID rfid(14, 2);	                              // Считыватель RFID карт
Clock RTC(0x68, false);                           // RTC часы
Timer timer;                                      // Таймер
LiquidCrystal_I2C lcd(0x27, 16, 2);               // Дисплей
AsyncWebServer server(80);                        // Веб сервер
WiFiClient http;                                  // Клиент для различных обращений к другим серверам

void open() {                                     // Прерывание на открытие двери при нажатии кнопки
	ESP.restart();
}

void setup()
{
	/* 1 этап - Инициализация обязательных компонентов */
	// Подготовка дисплея
	lcd.init();
	lcd.clear();
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("Loading   0%");
	lcd.setCursor(0, 1);
	lcd.print("Status:  ///////");

	// Инициализация обязательных компонентов
	memory.begin(0x50);
	SPI.begin();
	rfid.PCD_Init(); 
	keypad.begin();
	writeLoadCent(20);

	// Инициализация дополнительных компонентов
	// Считывание параметров
	if (memory.status) for (int i = 0; i < 8; i++) {
		settings[i] = memory.readbit(i, 0);
	}
	if (NeedSerial) {                             // UART
		Serial.begin(115200);
	}  
	writeLoadCent(25);
	ledcSetup(0, 1000, 8);                        // Динамик                          
	if (Buzzer) {
		ledcWrite(0, 200);
	} 
	else {
		ledcWrite(0, 0);
	}  
	writeLoadCent(27);  

	// Монтирование файловой системы
	lcd.setCursor(9, 1);
	if (SD.begin() && SD.cardType() != CARD_NONE && SD.cardType() != CARD_UNKNOWN) {                  
		SDWorking = true;
		lcd.print("+");
	} 
	else if (ShowError) {
		Serial.println("Error mounting SD");
		lcd.print("-");
	}
	writeLoadCent(30);

	// Проверка компонентов
	

	writeLoadCent(50);

	// /* 2 этап - Псевдо-прерывание */
	//   int pins[2][2] = {
	//   {26, 25},                                   // 0 строка - реле
	//   {13, 12}                                    // 1 строка - кнопки
	// };                     

	// if (memory.status) for (int i = 1; i <= 2; i++) {
	//   settings[i + 3] = memory.readbit(i + 3, 0);
	//   if (settings[i + 3]) {
	//     pinMode(pins[1][i], INPUT);               // Настройка входа для кнопки
	//     pinMode(pins[0][i], OUTPUT);              // Настройка сигнала для реле
	//     digitalWrite(pins[0][i], HIGH);
	//     attachInterrupt(digitalPinToInterrupt(pins[1][i]), open, FALLING); // Добавление прерывания
	//   }
	// }

	// // Псевдо-проверка
	// if (digitalRead(pins[1][0]) == LOW) {
	//   Interface::open(0);
	// } 
	// else if (digitalRead(pins[1][1]) == LOW) {
	//   Interface::open(1);
	// }
 
	writeLoadCent(60);

	/* 3 этап - Настройка Wi-Fi */
	if (memory.status) {
		Network::setupWiFi();
	} 
	else {
		Network::presetupWiFi();                  // Создание точки доступа с предустановленными значениями
	}            
	lcd.setCursor(14, 1);
	if (WiFi.isConnected()) {
		lcd.print("+");
	} 
	else {
		lcd.print("-");
	}              
	writeLoadCent(70);

	/* 4 этап - Web-сервер */
	// Синхронизация часов
	lcd.setCursor(15, 1);
	if (RTC.begin()) {
		lcd.print("+");
	} 
	else {
		lcd.print("-");
	}
	RTC.sync(http);
	writeLoadCent(72);

	// Настройка Web сервера
	if (SDWorking && WiFi.status() == WL_CONNECTED) 
	{
		if (memory.status)                        // Включение MDNS
		{
			host = memory.readString(10, 385);
			if (!host.isEmpty() && MDNS.begin(host.c_str())) 
			{
				if (NeedSerial) {
					Serial.println("MDNS включён, локальный адрес: http://" + String(host) + ".local/");
				}
				MDSNWorking = true;
			}
		}
		writeLoadCent(75);

		// Настройка маршрутизации сервера
		// HTML
		server.onNotFound(handleNotFound);
		server.on("/", HTTP_GET, handleNotFound);  

		// JS & CSS
		server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(SD, "/style.css", "text/css");
		});
		server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(SD, "/script.js", "text/javascript");
		});

		// Используемые плагины
		server.on("/FontAwsome/font-awsome all.css", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(SD, "/FontAwsome/font-awsome all.css", "text/css");
		});
		server.on("/FontAwsome/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(SD, "/FontAwsome/fa-solid-900.woff2", "font/woff");
		});
		server.on("/FontAwsome/fa-solid-900.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(SD, "/FontAwsome/fa-solid-900.svg", "image/svg+xml");
		});
		writeLoadCent(85);

		// Включение сервера
		server.begin();
		if (MDSNWorking) {
			MDNS.addService("http", "tcp", 80);
		}
		writeLoadCent(90);

		// Настройка перрывания для обновления по сети
		ArduinoOTA.onStart([]{
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH) {
				type = "sketch";
			}
			else {
				type = "filesystem";
			}
		});
		ArduinoOTA.begin();
	}
	writeLoadCent(100);
	delay(500);

	// Сброс дисплея
	lcd.noBacklight();
	Interface::goHome();
}

void loop()
{
	ArduinoOTA.handle();
	
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