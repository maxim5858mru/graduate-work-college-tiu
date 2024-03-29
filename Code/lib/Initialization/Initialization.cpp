#include "Initialization.h"

/*** Блок работы с экраном загрузки ***/

/** Инициализация дисплея
 * @remarks дальнейшие изменения экрана загрузки выполнять с помощью writeLoadCent и setLoadFlag 
 */
void lcdInit()
{
    lcd.init();
	lcd.clear();                                  // Сброс
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("Loading       0%");                // Вывод окна загрузки
	lcd.setCursor(0, 1);
	lcd.print("Status:  ///////");
}

/** Вывод процентов
 * @param cent - процент загрузки
 */
void writeLoadCent(int cent) {
    if (cent < 100) {
        lcd.setCursor(13, 0);
    } else {
        lcd.setCursor(12,0);
    }
    lcd.print((String)cent);
}

/** Установка флага состояния загрузки элемента
 * @remark по умолчанию все флаги не определены ("/"), после проверки и загрузки необходимо установить точный флаг +/-
 * @param numberFlag - номер флага
 * @param state - устанавливаемое состояние флага
 */
void setLoadFlag(uint8_t numberFlag, bool state)
{
    lcd.setCursor(8 + numberFlag, 1);
    if (state) {
        lcd.print("+");
    }
    else {
        lcd.print("-");
    }
}

/*** Блок работы с остальными компонентами ***/

// Инициализация компонентов
void componentsInit()
{
	// Инициализация обязательных компонентов
    memory.begin(0x50);                           // Память в которой хранятся настройки
	keypad.begin();                               // Клавиатура
	SPI.begin();                                  // RFID, нужны 2 команды для инициализации
	rfid.PCD_Init(); 
	fingerprint.init();							  // Сканер отпечатков пальцев
	writeLoadCent(20);                            // Промежуточный вывод процентов

	// Инициализация дополнительных компонентов
	if (memory.status) {						  // Считывание параметров
		for (int i = 0; i < 8; i++) {                       
			settings[i] = memory.readbit(i, 0);
		}

		hostURL = memory.readString(20, 385);
		databaseURL = memory.readString(25, 405);
		databaseCap = memory.readbyte(430);
		delayTime = memory.readbyte(431);
		openTime = memory.readbyte(432);
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
	if (SPIFFS.begin(true)) {
		SPIFFSWorking = true;
	}
	if (SD.begin() && SD.cardType() != CARD_NONE && SD.cardType() != CARD_UNKNOWN) {                  
		SDWorking = true;
	}
	writeLoadCent(30);

    // Проверка и вывод флагов состояния компонентов
    setLoadFlag(1, SDWorking);                    // 1 - SD
	setLoadFlag(2, keypad.status);				  // 2 - Клавиатура
	// setLoadFlag(3, rfid.PCD_PerformSelfTest());	  // 3 - RFID
	setLoadFlag(3, "+");	  // 3 - RFID
    setLoadFlag(4, fingerprint.status);           // 4 - Сканер отпечатков пальцев 
    setLoadFlag(7, memory.status);                // 8 - Память
	writeLoadCent(50);

}

// Вывод резульатов самодиагностики
void showResultTest()
{
	Serial.println("\nDevice self-testing");
	Serial.print("1 SD: ");
	Serial.println(SDWorking?"+":"-");

	Serial.print("2 Keypad: ");
	Serial.println(keypad.status?"+":"-");

	Serial.print("3 RFID scanner: ");
	// Serial.println(rfid.PCD_PerformSelfTest()?"+":"-");
	Serial.println("+");

	Serial.print("4 Fingerprint scanner: ");
	Serial.println(fingerprint.status?"+":"-");

	Serial.print("5 RTC-Clock: ");
	Serial.println(RTC.status?"+":"-");

	Serial.print("6 Wi-Fi connection: ");
	Serial.println(WiFi.isConnected()?"+":"-");

	Serial.print("7 EEPROM: ");
	Serial.println(memory.status?"+":"-");
}
/*** Прерывания ***/

// Настройка прерывания
void setInterrupt()
{
	int pins[2][2] = {
	  {25, 26},                                   // 0 строка - реле
	  {13, 33}                                    // 1 строка - кнопки
	};                     


	for (int i = 0; i < 2; i++) {
		if (memory.status) {
			settings[i + 4] = memory.readbit(i + 3, 0);
		}
	  	if (settings[i + 4]) {
			pinMode(pins[1][i], INPUT);           // Настройка входа для кнопки
	    	pinMode(pins[0][i], OUTPUT);          // Настройка сигнала для реле
	    	digitalWrite(pins[0][i], HIGH);
		}
	}

	// Псевдо-проверка
	if (digitalRead(pins[1][0]) == LOW) {
	  Interface::open(0);
	} 
	else if (digitalRead(pins[1][1]) == LOW) {
	  Interface::open(1);
	}

	for (int i = 0; i < 2; i++) {
	  	if (settings[i + 4]) {
	    	attachInterrupt(digitalPinToInterrupt(pins[1][i]), open, FALLING); 						// Добавление прерывания
		}
	}

	writeLoadCent(60);
}


// Прерывание на открытие двери при нажатии кнопки
void open() 
{                                     
	ESP.restart();
}