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
	lcd.print("Loading   0%");                    // Вывод окна загрузки
	lcd.setCursor(0, 1);
	lcd.print("Status:  ///////");
}

/** Вывод процентов
 * @param cent - процент загрузки
 */
void writeLoadCent(int cent) {
    if (cent < 100) {
        lcd.setCursor(9, 0);
    } else {
        lcd.setCursor(8,0);
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
	if (memory.status) for (int i = 0; i < 8; i++) {                       // Считывание параметров
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
	if (SD.begin() && SD.cardType() != CARD_NONE && SD.cardType() != CARD_UNKNOWN) {                  
		SDWorking = true;
	}
	writeLoadCent(30);

    // Проверка и вывод флагов состояния компонентов
    setLoadFlag(1, SDWorking);                    // 1 - SD
	setLoadFlag(2, keypad.status);				  // 2 - Клавиатура
	setLoadFlag(3, rfid.PCD_PerformSelfTest());	  // 3 - RFID
    setLoadFlag(4, fingerprint.status);           // 4 - Сканер отпечатков пальцев 
    setLoadFlag(7, memory.status);                // 8 - Память
	writeLoadCent(50);
}

/*** Прерывания ***/

// Настройка прерывания
void setInterrupt()
{
	int pins[2][2] = {
	  {26, 25},                                   // 0 строка - реле
	  {13, 12}                                    // 1 строка - кнопки
	};                     

	if (memory.status) for (int i = 1; i <= 2; i++) {
	  settings[i + 3] = memory.readbit(i + 3, 0);
	  if (settings[i + 3]) {
	    pinMode(pins[1][i], INPUT);               // Настройка входа для кнопки
	    pinMode(pins[0][i], OUTPUT);              // Настройка сигнала для реле
	    digitalWrite(pins[0][i], HIGH);
	    attachInterrupt(digitalPinToInterrupt(pins[1][i]), open, FALLING); // Добавление прерывания
	  }
	}

	// Псевдо-проверка
	if (digitalRead(pins[1][0]) == LOW) {
	  Interface::open(0);
	} 
	else if (digitalRead(pins[1][1]) == LOW) {
	  Interface::open(1);
	}

	writeLoadCent(60);
}

// Прерывание на открытие двери при нажатии кнопки
void open() 
{                                     
	ESP.restart();
}