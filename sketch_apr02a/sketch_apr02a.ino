// --- БЛОК 1: ЗАГОЛОВОК И ПОДКЛЮЧЕНИЕ БИБЛИОТЕКИ ---
#include <Arduino_GFX_Library.h> // Подключаем нужную библиотеку

// --- БЛОК 2: ОПРЕДЕЛЕНИЕ ПИНОВ ДЛЯ ARDUINO NANO ---
// Пин 10 - CS, 9 - DC, 8 - RST
#define TFT_CS    10
#define TFT_DC    9
#define TFT_RST   8

// --- БЛОК 3: СОЗДАНИЕ ОБЪЕКТА ДИСПЛЕЯ (ПРОСТОЙ СПОСОБ) ---
// Создаем объект tft для дисплея ILI9341

#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *tft = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *tft = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
#endif /* !defined(DISPLAY_DEV_KIT) */

// --- БЛОК 4: КОНСТАНТЫ КООРДИНАТ (ИЗ НАШЕГО ПРОЕКТА) ---
// Координаты центров приборов
#define TEMP_CENTER_X 203 
#define TEMP_CENTER_Y 157 
#define HUM_CENTER_X 88  
#define HUM_CENTER_Y 157  

// Длина стрелок
#define TEMP_NEEDLE_LENGTH 77 
#define HUM_NEEDLE_LENGTH 63 

// --- БЛОК 5: ФУНКЦИИ SETUP И LOOP ---
void setup() {
  // Инициализация дисплея
  tft->begin();
  
  // Установка ориентации экрана (1 - горизонтально)
  tft->setRotation(1);

  // Заливка экрана черным цветом для очистки
  tft->fillScreen(tft->color565(0, 0, 0)); 

  // --- ОТРИСОВКА ДВУХ ЛИНИЙ (СТРЕЛОК) ---
  
  // Определяем цвет стрелки (желтый)
  uint16_t NEEDLE_COLOR = tft->color565(255, 255, 0);

  // --- ОТРИСТКА СТРЕЛКИ ТЕМПЕРАТУРЫ ---
  // Линия из центра (TEMP_CENTER_X, TEMP_CENTER_Y)
  // в точку (TEMP_CENTER_X - длина, TEMP_CENTER_Y)
  tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, TEMP_CENTER_X - TEMP_NEEDLE_LENGTH, TEMP_CENTER_Y, NEEDLE_COLOR);

  // --- ОТРИСТКА СТРЕЛКИ ВЛАЖНОСТИ ---
  // Линия из центра (HUM_CENTER_X, HUM_CENTER_Y)
  // в точку (HUM_CENTER_X - длина, HUM_CENTER_Y)
  tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, HUM_CENTER_X - HUM_NEEDLE_LENGTH, HUM_CENTER_Y, NEEDLE_COLOR);
}

void loop() {
  // Ничего не делаем в цикле, все рисуется один раз в setup()
}
