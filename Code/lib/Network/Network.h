#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wps.h>
#include "../EEPROM/EEPROM.h"
#include "../Timer/Timer.h"

// Для WPS
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

// Стандартный SSID и пароль
#define DEFAULT_SSID      "AC001"
#define DEFAULT_PASSWORD  "10012019-C"

// Адреса памяти
#define EEPROM_ADDRESS_WiFi 1
#define EEPROM_ADDRESS_AP   289

// Идентификаторы настроек
#define WiFiAP settings[2]
#define ShowError settings[3]

extern bool settings[8];
extern EEPROM memory;
extern Timer timer;

namespace Network
{
	static String ssid;
	static String password;
	static bool WPSWorking = false;
	static esp_wps_config_t config;

	// Обработчик прерываний Wi-Fi
	void interrupt(WiFiEvent_t event, system_event_info_t info);

	// Включение WPS
	void startWPS();
	
	// Выбор Wi-Fi сети
	bool selectWiFi();

	// Создание точки доступа с предустановленными значениями 
	void presetupWiFi();

	// Настройка Wi-Fi
	void setupWiFi();
}

#endif