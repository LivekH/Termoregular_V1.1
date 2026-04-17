// main_sketch.ino

// --- ПОДКЛЮЧАЕМ ВСЕ БИБЛИОТЕКИ ---
#include <Arduino_GFX_Library.h> // Для дисплея
#include "Main_page.h"           // Наша библиотека для статичного фона

// --- БИБЛИОТЕКИ ДЛЯ ДАТЧИКОВ И ЭНКОДЕРА ---
#include <Wire.h>
#include <Adafruit_SHT31.h>      // Для температуры и влажности SHT31
#include <DS3231.h>              // Для часов реального времени DS3231
#include <Encoder.h>             // Для энкодера

// --- ОПРЕДЕЛЯЕМ КОНСТАНТЫ (КООРДИНАТЫ) ---
// Мы определяем их здесь один раз. Библиотека Main_page будет их "видеть".
#define TEMP_CENTER_X 205 
#define TEMP_CENTER_Y 157 
#define HUM_CENTER_X 100  
#define HUM_CENTER_Y 157  

#define TEMP_NEEDLE_LENGTH 77 
#define HUM_NEEDLE_LENGTH 63 
#define ENCODER_BUTTON_PIN 4 

// --- СОЗДАЕМ ОБЪЕКТЫ (ИНСТАНСЫ) ДЛЯ НАШИХ КОМПОНЕНТОВ ---
Adafruit_SHT31 shtSensor = Adafruit_SHT31();
DS3231 rtcClock;
Encoder navigationEnc(2, 3); // Пины энкодера

// --- ПЕРЕМЕННЫЕ ДЛЯ ХРАНЕНИЯ ДАННЫХ ---
float currentTemperature = 0;
float currentHumidity = 0;
String currentTime = "00:00";

void setup() {
  // --- ИНИЦИАЛИЗАЦИЯ ВСЕХ КОМПОНЕНТОВ ---
  
  // Инициализируем дисплей
  tft->begin();
  tft->setRotation(1);
  
  // Инициализируем датчики и часы
  shtSensor.begin();
  DS3231 rtcClock;
  
  // Настраиваем пин кнопки энкодера
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // --- ОТРИСОВКА ГЛАВНОЙ СТРАНИЦЫ ---
  
  // 1. Рисуем статичный фон (шкалы, рамки, подписи)
  drawStaticBackground();
  
  // 2. Рисуем динамические элементы в их начальном (нулевом) состоянии
  
  // А) Цифровые индикаторы (влажность и температура)
  tft->setTextSize(2); // Размер шрифта для цифр
  tft->setTextColor(tft->color565(255, 255, 255));
  
  tft->setCursor(40, 173); // Координаты для влажности
  tft->print(currentHumidity, 0); // Выводим "0"
  
  tft->setCursor(185, 173); // Координаты для температуры
  tft->print(currentTemperature, 0); // Выводим "0"
  
  // Б) Стрелки приборов (в нулевом положении)
  drawTEMP_NEEDLE();
  drawHUM_NEEDLE();
  
  // В) Меню управления ON / AUTO / OFF (согласно нашей договоренности)
  drawON();
  drawAUTO(); // AUTO активен и зеленый
  drawOFF();
}


void loop() {
  // --- ПОКА ЗДЕСЬ ПУСТО ---
  // Мы просто удерживаем состояние.
  // В следующих шагах сюда добавится логика:
  // - Чтение датчиков
  // - Чтение энкодера
  // - Анимация стрелок
  
  delay(100); // Небольшая задержка, чтобы не нагружать процессор
}


// --- ФУНКЦИИ ДЛЯ ОТРИСТКИ ДИНАМИЧЕСКИХ ЭЛЕМЕНТОВ ---
// Они остаются здесь, так как перерисовываются в loop()

void drawTEMP_NEEDLE() {
    tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, TEMP_CENTER_X - TEMP_NEEDLE_LENGTH, TEMP_CENTER_Y, tft->color565(225, 255, 0));
}
void drawHUM_NEEDLE() {
    tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, HUM_CENTER_X - HUM_NEEDLE_LENGTH, HUM_CENTER_Y, tft->color565(225, 255, 0));
}

void drawON(){
    tft->setTextSize(2);
    if(activeMode == "ON"){
        tft->setTextColor(tft->color565(0, 255, 0));
    } else {
        tft->setTextColor(tft->color565(255, 255, 255));
    }
    tft->setCursor(40, 220); 
    tft->print("ON");
}
 
void drawAUTO(){
    tft->setTextSize(2);
    if(activeMode == "AUTO"){
        tft->setTextColor(tft->color565(0, 255, 0));
    } else {
        tft->setTextColor(tft->color565(255, 255, 255));
    }
    tft->setCursor(140, 220); 
    tft->print("AUTO");
} 

void drawOFF(){
    tft->setTextSize(2);
    if(activeMode == "OFF"){
        tft->setTextColor(tft->color565(0, 255, 0));
    } else {
        tft->setTextColor(tft->color565(255, 255, 255));
    }
    tft->setCursor(240, 220); 
    tft->print("OFF");
}
