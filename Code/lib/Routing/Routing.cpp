#include "Routing.h"

// Настройка таблицы маршрутизации
void setRouting()
{
	// HTML
	server.onNotFound(handleNotFound);
	server.on("/", HTTP_GET, handleNotFound);  

	// JS & CSS
	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/style.css", "text/css");
	});
	server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/script.js", "text/javascript");
	});

	// Используемые плагины
	server.on("/FontAwsome/font-awsome all.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/FontAwsome/font-awsome all.css", "text/css");
	});
	server.on("/FontAwsome/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/FontAwsome/fa-solid-900.woff2", "font/woff");
	});
	server.on("/FontAwsome/fa-solid-900.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/FontAwsome/fa-solid-900.svg", "image/svg+xml");
	});
}

// Маршрутизация
void handleNotFound(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/NotFound.html", "text/html");
}