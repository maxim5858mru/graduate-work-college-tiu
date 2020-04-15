#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wps.h>

namespace Network
{
  static String ssid;
  static String password;
  static bool WPSWorking = false;
  static esp_wps_config_t config;

  //Обработчик прерываний Wi-Fi
  void interrupt(WiFiEvent_t event, system_event_info_t info);

  //Включение WPS
  void startWPS();
  
  //Выбор Wi-Fi сети
  bool selectWiFi();

  //Создание точки доступа с предустановленными значениями 
  void presetupWiFi();

  //Настройка Wi-Fi
  void setupWiFi();
}
#endif