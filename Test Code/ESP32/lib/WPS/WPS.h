#ifndef WPS_H
#define WPS_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wps.h>

#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

namespace WPS
{
  static esp_wps_config_t config;

 void interrupt(WiFiEvent_t event, system_event_info_t info)
  {
    switch(event){
      case SYSTEM_EVENT_STA_START:
        Serial.println("Station Mode Started");
        break;
      case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("Connected to :" + String(WiFi.SSID()));
        Serial.print("Got IP: ");
        Serial.println(WiFi.localIP());
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("Disconnected from station, attempting reconnection");
        WiFi.reconnect();
        break;
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
        esp_wifi_wps_disable();
        delay(10);
        WiFi.begin();
        break;
      case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("WPS Failed, retrying");
        esp_wifi_wps_disable();
        esp_wifi_wps_enable(&config);
        esp_wifi_wps_start(0);
        break;
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("WPS Timedout, retrying");
        esp_wifi_wps_disable();
        esp_wifi_wps_enable(&config);
        esp_wifi_wps_start(0);
        break;
      // case SYSTEM_EVENT_STA_WPS_ER_PIN:
      //   String pin;
      //   for(int i=0;i<8;i++){
      //     pin[i] = info.sta_er_pin.pin_code[i];
      //   }
      //   pin[8] = '\0';
      //   Serial.println("WPS_PIN = " + pin);
      //   break;
      default:
        break;
    }
  }

  //Включение WPS
  void start()
  {
    WiFi.onEvent(interrupt);
    WiFi.mode(WIFI_MODE_STA);

    config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
    config.wps_type = ESP_WPS_MODE;
    strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
    strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
    strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
    strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);

    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  }

}

#endif