#include "Timer.h"

Timer::Timer() :
  _timer(nullptr) {}

Timer::~Timer() {
  detach();
}

void Timer::_attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uint32_t arg) {
  esp_timer_create_args_t _timerConfig;
  _timerConfig.arg = reinterpret_cast<void*>(arg);
  _timerConfig.callback = callback;
  _timerConfig.dispatch_method = ESP_TIMER_TASK;
  _timerConfig.name = "Timer";
  if (_timer) {
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
  }
  esp_timer_create(&_timerConfig, &_timer);
  if (repeat) {
    esp_timer_start_periodic(_timer, milliseconds * 1000ULL);
  } else {
    esp_timer_start_once(_timer, milliseconds * 1000ULL);
  }
}

void Timer::detach() {
  if (_timer) {
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
    _timer = nullptr;
  }
}
