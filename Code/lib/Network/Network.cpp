#include "Network.h"

// Обработчик прерываний Wi-Fi
void Network::interrupt(WiFiEvent_t event, system_event_info_t info)
{
	if (Serial) switch (event) {
		case SYSTEM_EVENT_STA_START:
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			Serial.println("Connected to :" + String(WiFi.SSID()));
			Serial.print("Got IP: ");
			Serial.println(WiFi.localIP());
			break;
		// case SYSTEM_EVENT_STA_DISCONNECTED:
		//   WiFi.reconnect();
		//   /**/Serial.println("HHH");
		//   break;
		case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
			Serial.println("The device connected to " + String(WiFi.SSID()) + " using WPS.");
			esp_wifi_wps_disable();
			WiFi.begin();
			Serial.print("Device IP Address: ");
			Serial.println(WiFi.localIP());
			Network::WPSWorking = false;
			break;
		case SYSTEM_EVENT_STA_WPS_ER_FAILED:
			Serial.println("Error using WPS.\nTo retry? (y / n)");
			while (!Serial.available()) {}
			if (Serial.readString() == "y") {
				esp_wifi_wps_disable();
				esp_wifi_wps_enable(&config);
				esp_wifi_wps_start(0);
			}
			else {
				Network::WPSWorking = false;
			}
			break;
		case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
			Serial.println("Time out waiting. To retry?");
			while (!Serial.available()) {}
			if (Serial.readString() == "y") {
				esp_wifi_wps_disable();
				esp_wifi_wps_enable(&config);
				esp_wifi_wps_start(0);
			}
			else {
				Network::WPSWorking = false;
			}
			break;
		case SYSTEM_EVENT_STA_WPS_ER_PIN:
			char PIN[9];
			for (int i = 0; i < 8; i++) {
				PIN[i] = info.sta_er_pin.pin_code[i];
			}
			PIN[8] = '\0';
			Serial.println("WPS PIN = " + String(PIN));
			break;
		default:
			break;
	}
}

// Включение WPS
void Network::startWPS()
{
	WiFi.mode(WIFI_MODE_STA);

	config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
	config.wps_type = ESP_WPS_MODE;
	strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
	strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
	strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
	strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);

	WPSWorking = true;
	esp_wifi_wps_enable(&config);
	esp_wifi_wps_start(0);

	Serial.println("WPS working.");
}

// Выбор Wi-Fi сети
bool Network::selectWiFi()
{
	WiFi.disconnect();                            // Перед поиском нужно отключить WiFi контроллер от "сети" (которой нет) 
	delay(100);
	int numberOfNetworks = WiFi.scanNetworks();

	if (numberOfNetworks == 0) {
		Serial.println("No networks available.\nEnable access point? (y/n)");
		timer.timerStart(60);                     // Если никто не ответил в течении 1 минуты, то будут использованы стандартные настройки WiFi
		while(!Serial.available() && timer.timerIsWorking()){}
		if (timer.timerCheckAndStop() && Serial.readString() == "y") {
			WiFiAP = true;
			Serial.println("Enter the SSID of the new access point: ");
			while(!Serial.available()){}
			ssid = Serial.readString();

			Serial.println("Enter the network password or leave an empty field to make the access point open: ");
			while(!Serial.available());
			password = Serial.readString();

			return true;
		}
		else {
            return false;
        }
	}
	else {
		Serial.println("Number of networks found: " + String(numberOfNetworks));
		String networksSSID[numberOfNetworks];    // Массив нужен что бы правильно работал выбор сети по номеру
		bool networksENCRYPT[numberOfNetworks]; 
		for (int i = 0; i < numberOfNetworks; i++) {
			networksSSID[i] = WiFi.SSID(i);
			networksENCRYPT[i] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?false:true;

			// Вывод списка сетей
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(networksSSID[i]);
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println(networksENCRYPT[i]?" закрытая":" открытая");
			delay(10);
		}

		Serial.println("Select a WiFi network or enter 0 to enable an access point: ");
		timer.timerStart(120);
		while(!Serial.available() && timer.timerIsWorking()){}
		if (!timer.timerCheckAndStop()) return false;
		String temp = Serial.readString();

		if (temp == "-1") {                       // WPS
			WiFiAP = false;
			startWPS();
		}
		else if (temp == "0") {                   // AP
		
			WiFiAP = true;
			Serial.println("Enter the SSID of the new access point: ");
			while(!Serial.available()){}
			ssid = Serial.readString();

			Serial.println("Enter the network password or leave an empty field to make the access point open: ");
			while(!Serial.available());
			password = Serial.readString();

			return true;
		}
		else if ([](String string) -> bool {      // По номеру из списка
			for (int i = 0; i < string.length(); i++) {
				if ((0x30 <= string[i] && string[i] <= 0x39) || string[i] == 0x0A || string[i] == 0x0D || string[i] == 0x20) {}
				else {
                    return false;
                }
			}
			return true;
		} (temp) && temp.toInt() <= numberOfNetworks) {
			int numberSSID = temp.toInt() - 1;
			ssid = networksSSID[numberSSID];
			Serial.println("You have selected a network with an SSID '" + ssid + "'.");

			if (networksENCRYPT[numberSSID]) {
				Serial.println("Enter the password: ");
				while (!Serial.available()) {}
				password = Serial.readString();
			}
			else {
                password = "";
            }

			return true;
		}
		else                           // По имени сети
		{
			ssid = temp;
			Serial.println("You entered the SSID '" + ssid + "'.");
			
			Serial.println("Enter the password: ");
			while (!Serial.available()) {}
			password = Serial.readString();

			if (password.isEmpty() || password == "-" || password[0] == 0x0A || password[0] == 0x0D) {
                password = "";
            }

			return true;
		}
	}

	return false;
}

// Создание точки доступа с предустановленными значениями 
void Network::presetupWiFi()
{
	WiFiAP = true;
	ssid = DEFAULT_SSID;
	password = DEFAULT_PASSWORD;
	WiFi.softAP(ssid.c_str(), password.c_str());  // Создание точки доступа

	if (Serial) {
		Serial.println("Attempt to create a network with ssid '" + ssid + "'.");
		IPAddress IPnumber = WiFi.softAPIP();
		Serial.print("Device IP address: ");
		Serial.println(IPnumber);
	}
}

// Настройка Wi-Fi
void Network::setupWiFi()
{
	// Настройка прерывания
	WiFi.onEvent(interrupt);

	// Сброс Wi-Fi модуля
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.softAPdisconnect();             
	delay(100);

	// Получение SSID и пароля из памяти
	if (WiFiAP) {                                 // Если в настройках указанно что нужна точка доступа
	
		ssid = memory.readString(32, EEPROM_ADDRESS_AP);
		if (!ssid.isEmpty()) {                    // Если пусто, то ждём проверки на соединение (результат которой будет ложный)
		
			password = memory.readString(64, EEPROM_ADDRESS_AP + 32);
			if (password.isEmpty()) {             // Создание точки доступа
                WiFi.softAP(ssid.c_str(), NULL);
            }
			else {
                WiFi.softAP(ssid.c_str(), password.c_str());
            }

			if (Serial) {                         // Для отладки
			    Serial.println("Attempt to create a network with ssid '" + ssid + "'.");
				IPAddress IPnumber = WiFi.softAPIP();                      // Получаем IP адрес устройства
				Serial.print("Device IP address: ");
				Serial.println(IPnumber);
			}
			return;
		}
	}
	else for (int i = 0; i < 3; i++) {            // Перебор сохранённых значений
	
		ssid = memory.readString(32, EEPROM_ADDRESS_WiFi + (96) * i);
		/**/Serial.println("WiFi "+ssid);
		if (ssid.isEmpty()) {
            continue;
        }
		password = memory.readString(64, EEPROM_ADDRESS_WiFi + 32 + (96) * i);
		WiFi.begin(ssid.c_str(), password.c_str());                        // Попытка подключения
		if (Serial) {
            Serial.println("Attempt to connect to " + ssid + " network.");
        }
		delay(3000);
		if (WiFi.isConnected()) {
            return;
        }
	}

	while (1) {                                   // Программа выйдет из цикла если подключится к сети, или если пользователь откажется
		if (WiFi.isConnected()) {
            return;
        }
		else if (Serial && selectWiFi()) {        // Сразу проверка на UART и выбор сети, если пользователь ничего не выберет, включается стандартная точка доступа
		
			if (WiFiAP) {
				if (password.isEmpty() || password == "-" || password[0] == 0x0A || password[0] == 0x0D) {
					WiFi.softAP(ssid.c_str(), NULL);                       // Вместо пароля нужно передать NULL, чтобы точка доступа была открытая.
					password = "";
				}  
				else {
                    WiFi.softAP(ssid.c_str(), password.c_str());
                }

				if (Serial) {
					Serial.println("Attempt to create a network with ssid '" + ssid + "'.");

					IPAddress IPnumber = WiFi.softAPIP();
					Serial.print("Device IP address: ");
					Serial.println(IPnumber);

					// Сохранение SSID и пароля, для дальнейшего использования
					Serial.println("Save settings? (y/n)");
					while(!Serial.available()){}
					if (Serial.readString() == "y")	{
						memory.updatebit(0, true, 2);
						memory.updateString(EEPROM_ADDRESS_AP, ssid);
						memory.updateString(EEPROM_ADDRESS_AP + 32, password);
					}
				}

				return;
			}
			else if (WPSWorking) {                // Ожидание окончания работы WPS
			
				while (WPSWorking){} 
				return;
			}
			else {
				WiFi.begin(ssid.c_str(), password.c_str());
				Serial.println("Attempt to connect to " + ssid + " network.");
				delay(4000);

				if (WiFi.isConnected()) {
					Serial.println("Save settings? (y/n)");
					while(!Serial.available()){}
					if (Serial.readString() == "y") while (1)     // Пока пользователь не введёт нужное число программа будет этом цикле
					{
						Serial.println("Under what number (1/2/3)?");
						while(!Serial.available()){}
						String i =  Serial.readString();
						if ((i == "1") || (i == "2") || (i = "3"))

						memory.updatebit(0, false, 2);
						memory.updateString(EEPROM_ADDRESS_WiFi + 96 * (i.toInt() - 1), ssid);
						memory.updateString(EEPROM_ADDRESS_WiFi + 32 + 96 * (i.toInt() - 1), password);

						break;
					}
				}
				else {                            // Если пользователь сделал ошибку, он может повторить попытку соединения, выйти без подключения к Wi-Fi
				
					Serial.println("Invalid password or SSID. Connect to another network? (y/n)");
					while(!Serial.available()){}
					if (Serial.readString() == "y") {
                        continue;
                    }
					else {
                        return;
                    }
				}

				return;
			}
		}
		else {
			presetupWiFi(); 
			return;
		}
	}
}