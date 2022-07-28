// ================ БИБЛИОТЕКИ =====================

#include <Arduino.h>
#include <EncButton.h>

#include <Wire.h>
#include "LiquidCrystal_I2C.h"

#include "Parser.h"
#include "AsyncStream.h"

#include "mqtt.h"
#include "config.h"

// =============== НАСТРОЙКА ПИНОВ =================

#define ledPin 13
#define relayPin 8
#define pwmPin 1
// #define LED 2

#define encBtn 2
#define encS1 3
#define encS2 4

#define buzzerPin 7
#define hallSensorPin 9

// ==================== SETTINGS ===================

EncButton<EB_TICK, encS2, encS1, encBtn> enc;

LiquidCrystal_I2C lcd(0x3f, 16, 2);

AsyncStream<50> serial(&Serial, ';'); // обработчик и стоп символ

// =================== ПРОТОКОЛ ====================

// протокол:
// 0 - светодиод (0/1)
// 1 - вентилятор (0/1)
// 2 - пищалка (0/1)
// 3 - шим сигнал (0/255)
// 4 - текст
// 5 - mqtt switch (0/1)

// ===================== ФУНКЦИИ ===================

void serialSend(byte a, byte b)
{ // ФУНКЦИЯ ОТПРАВКИ ДАННЫХ НА КОМПЬЮТЕР
  Serial.print(a);
  Serial.print(',');
  Serial.print(b);
}

void parsing()
{ // ПРИЕМ ДАННЫХ С КОМПЬЮТЕРА
  if (serial.available())
  {
    Parser data(serial.buf, ','); // отдаём парсеру
    int ints[10];                 // массив для численных данных
    data.parseInts(ints);         // парсим в него

    switch (ints[0]) // КЛЮЧ
    {
    case 0: // СВЕТОДИОД
      digitalWrite(ledPin, ints[1]);

      lcd.setCursor(4, 0);
      lcd.print(ints[1]);
      break;

    case 1: // РЕЛЕ
      digitalWrite(relayPin, ints[1]);

      lcd.setCursor(15, 0);
      lcd.print(ints[1]);
      break;

    case 2: // ПИЩАЛКА
      digitalWrite(buzzerPin, !ints[1]);

      lcd.setCursor(4, 1);
      lcd.print(ints[1]);
      break;

    case 3: // ШИМ СИГНАЛ
      analogWrite(pwmPin, ints[1]);

      if (ints[1] > 0)
      {
        lcd.setCursor(13, 1);
        lcd.print(1);
      }
      else if (ints[1] == 0)
      {
        lcd.setCursor(13, 1);
        lcd.print(0);
      }
      break;

    case 4: // ТЕКСТ  НА ДИСПЛЕЙ
      data.split();
      lcd.clear();
      lcd.home();
      lcd.print(data[1]);
      break;
    }
  }
}

void display()
{ // ВЫВОД НАЧАЛЬНОГО ТЕКСТА НА ДИСПЛЕЙ
  lcd.setCursor(3, 0);
  lcd.print("Starting...");
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LED:0    RELAY:0");
  lcd.setCursor(0, 1);
  lcd.print("BZR:0    PWM:0");
}

void encoder()
{ // ЭНКОДЕР
  enc.tick();
  if (enc.isClick())
    serialSend(1, 0);
  if (enc.isRight())
    serialSend(1, 1);
  if (enc.isLeft())
    serialSend(1, 2);
}

boolean lastRead = false;
void hallSensor()
{ // ДАТЧИК ХОЛЛА
  if (digitalRead(hallSensorPin) != lastRead)
  {
    serialSend(2, !digitalRead(hallSensorPin));
    lastRead = digitalRead(hallSensorPin);
  }
}

// ==================== SETUP ======================

void setup()
{
  lcd.init();
  lcd.backlight();
  display();

  Serial.begin(9600);
  Serial.setTimeout(5);

  serialSend(100, 1);

  pinMode(LED, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);

  WiFi_connect();
  mqtt_connect();
}

// ===================== LOOP ======================

void loop()
{
  parsing();
  encoder();
  hallSensor();

  delayMicroseconds(50);

  client.loop();
}

// =================================================