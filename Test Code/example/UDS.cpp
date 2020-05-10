#include <Arduino.h>

long getDistance(uint8_t trig, uint8_t echo)
{
  long duration, cm;

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);

  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  //  Время задержки акустического сигнала на эхолокаторе.
  duration = pulseIn(echo, HIGH);

  // Теперь осталось преобразовать время в расстояние
  cm = (duration / 2) / 29.1;

  // Задержка между измерениями для корректной работы скеча
  delay(250);
  return cm;
}

void setup()
{
  Serial.begin(115200);
} 

void loop()
{
  Serial.println("Расстояние до объекта: " + (String)getDistance(15, 4) + " см.");
  delay(500);
}