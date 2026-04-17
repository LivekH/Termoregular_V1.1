//Блок 1 установка библиотек, назначение пинов установка констант для стрелок, цветов, режимов работы
// --- 1. ПОДКЛЮЧАЕМ ВСЕ НЕОБХОДИМЫЕ БИБЛИОТЕКИ ---

#include <Arduino_GFX_Library.h> // Библиотека для работы с дисплеем
#include <EEPROM.h>             // Библиотека для работы с энергонезависимой памятью (EEPROM)

// --- БИБЛИОТЕКИ ДЛЯ ДАТЧИКОВ И КОМПОНЕНТОВ ---
#include <Wire.h>               // Библиотека для работы с шиной I2C (нужна для SHT31 и DS3231)
#include <Adafruit_SHT31.h>     // Библиотека для датчика температуры и влажности SHT31
#include <RTClib.h>             // Библиотека для часов реального времени DS3231
#include <Encoder.h>            // Библиотека для работы с энкодером

// --- 2. ОПРЕДЕЛЯЕМ (ASSIGN) ВСЕ ПИНЫ ---

// --- ПИНЫ ДИСПЛЕЯ ---
#define TFT_CS    10  // Выбор кристалла (Chip Select)
#define TFT_DC    9   // Команда/Данные (Data/Command)
#define TFT_RST   8   // Сброс (Reset)

// --- ПИНЫ ДЛЯ УПРАВЛЕНИЯ НАГРЕВОМ/ОХЛАЖДЕНИЕМ ---
#define RELAY_PIN 5    // Пин, на котором подключено реле управления

// --- ПИНЫ ДЛЯ ЭНКОДЕРА ---
#define ENCODER_PIN_A 2  // Пин A энкодера (CLK)
#define ENCODER_PIN_B 3  // Пин B энкодера (DT)
#define ENCODER_BUTTON_PIN 4 // Пин кнопки энкодера

// --- ПИНЫ ДЛЯ ДАТЧИКОВ И ЧАСОВ (I2C) ---
// Для SHT31 и DS3231 используются стандартные пины I2C на Arduino Nano: A4 (SDA) и A5 (SCL)

// --- 3. СОЗДАЕМ ОБЪЕКТЫ (ИНСТАНСЫ) ДЛЯ НАШИХ КОМПОНЕНТОВ ---
// Создаем объект для работы с дисплеем
// --- СОЗДАНИЕ ОБЪЕКТОВ ---
//#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *tft = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

Arduino_DataBus *bus = create_default_Arduino_DataBus();

Arduino_GFX *tft = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
#endif /* !defined(DISPLAY_DEV_KIT) */

// Создаем объекты для датчиков и энкодера
Adafruit_SHT31 shtSensor = Adafruit_SHT31(); // Создаем объект датчика SHT31
RTC_DS3231 rtc;                                 // Создаем объект часов DS3231
Encoder myEnc(ENCODER_PIN_A, ENCODER_PIN_B); // Создаем объект энкодера

// --- 4. ОПРЕДЕЛЯЕМ КОНСТАНТЫ (КООРДИНАТЫ, РАЗМЕРЫ) ---

// --- КООРДИНАТЫ ЦЕНТРОВ ПРИБОРОВ (ОСНОВАНИЙ СТРЕЛОК) ---
#define TEMP_CENTER_X 205 
#define TEMP_CENTER_Y 157 
#define HUM_CENTER_X 100  
#define HUM_CENTER_Y 157  

// --- ДЛИНА СТРЕЛОК В ПИКСЕЛЯХ ---
#define TEMP_NEEDLE_LENGTH 77 
#define HUM_NEEDLE_LENGTH 63 

// --- 5. ЗАДАЕМ ЦВЕТА В ФОРМАТЕ RGB565 (ЧИСЛОВЫЕ ЗНАЧЕНИЯ) ---
// Используем числовые значения, чтобы не зависеть от объекта tft в библиотеке
#define COLOR_BACKGROUND tft->color565(0, 0, 0)     // Черный
#define COLOR_RED        tft->color565(255, 0, 0)   // Красный
#define COLOR_YELLOW     tft->color565(255, 255, 0) // Желтый
#define COLOR_GREEN      tft->color565(0, 255, 0)   // Зеленый
#define COLOR_BLUE       tft->color565(0, 0, 255)   // Синий
#define COLOR_GRAY       tft->color565(128,128,128) // Серый
#define COLOR_WHITE      tft->color565(255, 255, 255)// Белый
#define COLOR_DARKGREY   tft->color565(168, 168, 168)//Темно серый

// Режим работы по умолчанию
String activeMode = "AUTO"; 

// Значения температуры по умолчанию
float targetTemperature = 0.0; 
int hysteresis = 2; // Гистерезис по умолчанию

// --- 6. ЗАДАЕМ ЗНАЧЕНИЯ ПО УМОЛЧАНИЮ (ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ) ---
// --- ЗНАЧЕНИЯ ТАЙМЕРОВ ПО УМОЛЧАНИЮ ---
// Используем тип 'byte', так как мы будем хранить значения от 0 до 9 (1 байт)
byte timer1 = 0; // Таймер 1 по умолчанию выключен
byte timer2 = 0; // Таймер 2 по умолчанию выключен
byte timer3 = 0; // Таймер 3 по умолчанию выключен
byte timer4 = 0; // Таймер 4 по умолчанию выключен

//Блок 2 устано
void setup() {
  // --- 1. ИНИЦИАЛИЗАЦИЯ ВСЕХ КОМПОНЕНТОВ ---
  // Инициализируем дисплей
  tft->begin();
  tft->setRotation(1);

  // Инициализируем шину I2C для датчиков
  Wire.begin();
  
  // Инициализируем датчики и часы
  shtSensor.begin();
  

  // Инициализируем пины
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Реле выключено по умолчанию
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // --- 2. ЧТЕНИЕ ДАННЫХ ИЗ EEPROM ---
  // Читаем сохраненный режим работы (по адресу 0)
  char savedMode = EEPROM.read(0);
  if (savedMode == 'o') activeMode = "ON";
  if (savedMode == 'a') activeMode = "AUTO";
  if (savedMode == 'f') activeMode = "OFF";

  // Читаем сохраненную температуру (по адресу 1)
  char savedTemp = EEPROM.read(1);
  if (savedTemp >= '0' && savedTemp <= '9') {
      targetTemperature = savedTemp - '0'; // Преобразуем символ в число
  }

  // Читаем сохраненный гистерезис (по адресу 2)
  char savedHyst = EEPROM.read(2);
  if (savedHyst >= '0' && savedHyst <= '9') {
      hysteresis = savedHyst - '0';
  }

  // Читаем сохраненные таймеры (по адресам 3, 4, 5, 6)
  timer1 = EEPROM.read(3);
  timer2 = EEPROM.read(4);
  timer3 = EEPROM.read(5);
  timer4 = EEPROM.read(6);


  // --- 3. ДИАГНОСТИЧЕСКИЙ ОТЧЕТ ПРИ СТАРТЕ ---
  // Очищаем экран и готовим стиль для вывода
  tft->fillScreen(COLOR_BACKGROUND);
  tft->setTextSize(1);
  tft->setTextColor(COLOR_GREEN); // Зеленый текст на черном фоне

  // Выводим данные построчно друг за другом
  tft->setCursor(10, 20);
  tft->print("System Start...");
  
  delay(1000);
  
  tft->setCursor(10, 40);
  tft->print("Time: ");

  // --- ИСПРАВЛЕННЫЙ КОД ---
  // 1. Получаем текущее время с часов и сохраняем его внутри объекта rtcClock
  DateTime now = rtc.now();

  // 2. Выводим время на экран, используя данные из объекта
  // Добавляем проверку на "0" для красоты (чтобы было "09", а не "9")
  //if (rtc.now < 10) tft->print("0");
  tft->print(now.hour());
  tft->print(":");
  //if (rtc.minute < 10) tft->print("0");
  tft->print(now.minute());
  
  delay(1000);
  
  tft->setCursor(10, 60);
  tft->print("Temp Sensor: ");
  tft->print(shtSensor.readTemperature()); // Получаем температуру с датчика
  
  delay(1000);
  
  tft->setCursor(10, 80);
  tft->print("Set Temp (EEPROM): ");
  tft->print(targetTemperature); // Значение из памяти
  
  delay(1000);
  
  tft->setCursor(10, 100);
  tft->print("Hysteresis (EEPROM): ");
  tft->print(hysteresis); // Значение из памяти
  
  delay(1000);
  
  tft->setCursor(10, 120);
  tft->print("Timers (EEPROM): ");
  tft->setCursor(10, 140);
  tft->print("1="); tft->print(timer1);
  tft->setCursor(10, 160);
  tft->print("2="); tft->print(timer2);
  tft->setCursor(10, 180);
  tft->print("3="); tft->print(timer3);
  tft->setCursor(10, 200);
  tft->print("4="); tft->print(timer4);
  
  // --- 4. ЗАВЕРШЕНИЕ SETUP ---
  // Ждем 5 секунд, чтобы можно было прочитать отчет
  delay(5000);
  
  // Полностью стираем экран перед переходом в основной цикл
  tft->fillScreen(COLOR_BACKGROUND);
}

void loop() {
   
}
