// --- БЛОК 1: ЗАГОЛОВОК И ОПРЕДЕЛЕНИЯ ---

// 1.1 Подключаем графическую библиотеку для дисплея
#include <Arduino_GFX_Library.h>

// 1.2 Определяем пины подключения дисплея
#define TFT_CS    10  // Выбор кристалла (Chip Select)
#define TFT_DC    9  // Команда/Данные (Data/Command)
#define TFT_RST   8  // Сброс (Reset)

// --- СОЗДАНИЕ ОБЪЕКТОВ ---
//#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *tft = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *tft = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
#endif /* !defined(DISPLAY_DEV_KIT) */

// 1.4 Задаем константы для центров приборов (координаты оснований стрелок)
#define TEMP_CENTER_X 205 
#define TEMP_CENTER_Y 157 
#define HUM_CENTER_X 100  
#define HUM_CENTER_Y 157  

// 1.5 Задаем длину стрелок в пикселях
#define TEMP_NEEDLE_LENGTH 77 
#define HUM_NEEDLE_LENGTH 63 

// 1.6 Задаем цвета в формате RGB565
#define COLOR_BACKGROUND tft->color565(0, 0, 0)     // Черный
#define COLOR_RED        tft->color565(255, 0, 0)   // Красный
#define COLOR_YELLOW     tft->color565(255, 255, 0) // Желтый
#define COLOR_GREEN      tft->color565(0, 255, 0)   // Зеленый
#define COLOR_BLUE       tft->color565(0, 0, 255)   // Синий
#define COLOR_GRAY       tft->color565(128,128,128) // Серый
#define COLOR_WHITE      tft->color565(255, 255, 255)// Белый



// --- БЛОК 2: ФУНКЦИИ SETUP И LOOP ---

// 2.1 Блок setup() выполняется один раз при запуске
void setup() {
  // Инициализируем дисплей (подаем питание, настраиваем шину)
  tft->begin();
  
  
  // Устанавливаем ориентацию экрана. '1' - горизонтальный режим.
  tft->setRotation(1); 

  // Заливаем весь экран черным цветом для очистки
  tft->fillScreen(COLOR_BACKGROUND); 
  
  // --- ВЫЗЫВАЕМ ФУНКЦИИ ОТРИСОВКИ ПРИБОРОВ ---
  
  // Рисуем шкалу влажности в центре (88, 157)
  drawHumidityScale();
  
  // Рисуем шкалу температуры в центре (203, 157)
  drawTempScale(); 
  
  // Рисуем стрелку влажности от (88, 157)
  drawHUM_NEEDLE();
  
  drawTEMP_NEEDLE();
}
void drawTEMP_NEEDLE() { 
// ---  СТРЕЛКИ ТЕМПЕРАТУРЫ ---
  // Линия из центра (TEMP_CENTER_X, TEMP_CENTER_Y)
  // в точку (TEMP_CENTER_X - длина, TEMP_CENTER_Y)
  tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, TEMP_CENTER_X - TEMP_NEEDLE_LENGTH, TEMP_CENTER_Y, COLOR_YELLOW);
}
void drawHUM_NEEDLE() {
  // ---  СТРЕЛКИ ВЛАЖНОСТИ ---
  // Линия из центра (HUM_CENTER_X, HUM_CENTER_Y)
  // в точку (HUM_CENTER_X - длина, HUM_CENTER_Y)
  tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, HUM_CENTER_X - HUM_NEEDLE_LENGTH, HUM_CENTER_Y, COLOR_YELLOW);
}  
// 2.2 Блок loop() выполняется бесконечно
//void loop() {
  // Так как интерфейс статичный и отрисован в setup(),
  // здесь ничего делать не нужно.
//}


// --- БЛОК 3: ШКАЛА ВЛАЖНОСТИ (ВЕРСИЯ 2 - С ИСПОЛЬЗОВАНИЕМ ПЕРЕМЕННЫХ) ---
void drawHumidityScale() {
  int centerX = HUM_CENTER_X;
  int centerY = HUM_CENTER_Y;

  // --- ТВОЯ СТРУКТУРА РАСЧЕТА РАДИУСОВ ---
  int baseRadius = HUM_NEEDLE_LENGTH;
  int outerRadius = baseRadius + 22; // Внешний радиус серой дуги
  int innerRadius = baseRadius + 12; // Внутренний радиус серой дуги

  // --- ОТРИСОВКА СЕРОЙ ОКАНТОВКИ ---
  // Углы для влажности: от 180° до 360°
  tft->fillArc(centerX, centerY, outerRadius, innerRadius, 180, 270, COLOR_GRAY);

  // --- ОТРИСОВКА ЦВЕТНЫХ СЕКТОРОВ ---
  int colorOuterRadius = outerRadius - 10;
  int colorInnerRadius = innerRadius - 10;

  // --- РАСЧЕТ УГЛОВ ДЛЯ ВЛАЖНОСТИ (22,5 градусов на сектор) ---
  // Красный сектор (75-100%)
  tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 180, 202.5, COLOR_RED);
  
  // Желтый сектор (50-75%)
  tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 202.5, 225, COLOR_YELLOW);
  
  // Зеленый сектор (25-50%)
  tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 225, 247.5, COLOR_GREEN);
  
  // Синий сектор (0-25%)
  tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 247.5, 270, COLOR_BLUE); 
}

// --- БЛОК 4: ШКАЛА ТЕМПЕРАТУРЫ ---
void drawTempScale() {
   int centerX = TEMP_CENTER_X;
   int centerY = TEMP_CENTER_Y;

   // --- ТВОИ РАСЧЕТЫ РАДИУСОВ ---
   int baseRadius = TEMP_NEEDLE_LENGTH;
   int outerRadius = baseRadius + 22;
   int innerRadius = baseRadius + 12;

   // --- ОТРИСОВКА СЕРОЙ ОКАНТОВКИ ---
   // Используем твой рабочий синтаксис углов: от 180 до 360 градусов
   tft->fillArc(centerX, centerY, outerRadius, innerRadius, 180, 360, COLOR_GRAY);

   // --- ОТРИСОВКА ЦВЕТНЫХ СЕКТОРОВ ---
   int colorOuterRadius = outerRadius - 10;
   int colorInnerRadius = innerRadius - 10;

   // --- ВЕРНЫЙ РАСЧЕТ УГЛОВ ---
   // Вся шкала: от 180 до 360 градусов. Итого: 180 градусов.
   // Делим на 4 сектора: 180 / 4 = 45 градусов на сектор.
   
   // Синий сектор (0% - 25%) - Начинаем с 180° 
   tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 180, 225, COLOR_BLUE);
   
   // Зеленый сектор (25% - 50%) - Следующие 45 градусов
   tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 225, 270, COLOR_GREEN);
   
   // Желтый сектор (50% - 75%)
   tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 270, 315, COLOR_YELLOW);
   
   // Красный сектор (75% - 100%) - Заканчиваем на 360°
   tft->fillArc(centerX, centerY, colorOuterRadius, colorInnerRadius, 315, 360, COLOR_RED);
}

// 2.2 Блок loop() выполняется бесконечно
void loop() {
   //Так как интерфейс статичный и отрисован в setup(),
   //здесь ничего делать не нужно.
}
