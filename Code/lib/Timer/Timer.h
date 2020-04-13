#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

extern "C" {
  #include "esp_timer.h"
}

static bool TimerWorking = false;

static void endTimer() {TimerWorking = false;}

class Timer
{
public:
  Timer();
  ~Timer();
  
  typedef void (*callback_t)(void);
  typedef void (*callback_with_arg_t)(void*);
  
  void attach(float seconds, callback_t callback)
  {
    _attach_ms(seconds * 1000, true, reinterpret_cast<callback_with_arg_t>(callback), 0);
  }

  void attach_ms(uint32_t milliseconds, callback_t callback)
  {
    _attach_ms(milliseconds, true, reinterpret_cast<callback_with_arg_t>(callback), 0);
  }

  template<typename TArg>
  void attach(float seconds, void (*callback)(TArg), TArg arg)
  {
    static_assert(sizeof(TArg) <= sizeof(uint32_t), "attach() callback argument size must be <= 4 bytes");
    uint32_t arg32 = (uint32_t)arg;
    _attach_ms(seconds * 1000, true, reinterpret_cast<callback_with_arg_t>(callback), arg32);
  }

  template<typename TArg>
  void attach_ms(uint32_t milliseconds, void (*callback)(TArg), TArg arg)
  {
    static_assert(sizeof(TArg) <= sizeof(uint32_t), "attach_ms() callback argument size must be <= 4 bytes");
    uint32_t arg32 = (uint32_t)arg;
    _attach_ms(milliseconds, true, reinterpret_cast<callback_with_arg_t>(callback), arg32);
  }

  void once(float seconds, callback_t callback)
  {
    _attach_ms(seconds * 1000, false, reinterpret_cast<callback_with_arg_t>(callback), 0);
  }

  void once_ms(uint32_t milliseconds, callback_t callback)
  {
    _attach_ms(milliseconds, false, reinterpret_cast<callback_with_arg_t>(callback), 0);	
  }

  template<typename TArg>
  void once(float seconds, void (*callback)(TArg), TArg arg)
  {
    static_assert(sizeof(TArg) <= sizeof(uint32_t), "attach() callback argument size must be <= 4 bytes");
    uint32_t arg32 = (uint32_t)(arg);
    _attach_ms(seconds * 1000, false, reinterpret_cast<callback_with_arg_t>(callback), arg32);
  }

  template<typename TArg>
  void once_ms(uint32_t milliseconds, void (*callback)(TArg), TArg arg)
  {
    static_assert(sizeof(TArg) <= sizeof(uint32_t), "attach_ms() callback argument size must be <= 4 bytes");
    uint32_t arg32 = (uint32_t)(arg);
    _attach_ms(milliseconds, false, reinterpret_cast<callback_with_arg_t>(callback), arg32);
  }

  void detach();
  // bool active();

  //Запуск таймера
  void timerStart(float seconds) 
  {
    // _attach_ms(seconds * 1000, false, dynamic_cast<func>(endTimer), 0);
    once(seconds, endTimer);
    TimerWorking = true;
  }

  //Проверка и сброс флага
  bool timerCheckAndStop() 
  {
    bool temp = TimerWorking; 
    if (TimerWorking) 
    {
      TimerWorking = false; 
      detach();
    } 
    return temp;
  }

  //Проверка работы таймера + антихитрожопратин для компилятора 
  bool timerIsWorking()
  {
    delay(0); //"Переменная в условии пустого while можно считать за константу. Подошла один раз, значит будит подходить всегда. Да, цикл будит бесконечным. Изменится переменная? Данееееееееее. Прерывания????? Что за дичь ты мне втираешь" © Компилятор
    return TimerWorking;
  }
protected:	
  esp_timer_handle_t _timer;

  //Прерывание для таймера

  void _attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uint32_t arg);
};
#endif