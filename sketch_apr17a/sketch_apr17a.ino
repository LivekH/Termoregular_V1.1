//Блок 1 установка библиотек, назначение пинов установка констант для стрелок, цветов, режимов работы
// --- 1. ПОДКЛЮЧАЕМ ВСЕ НЕОБХОДИМЫЕ БИБЛИОТЕКИ ---

#include <Arduino_GFX_Library.h> // Библиотека для работы с дисплеем
#include <EEPROM.h>              // Библиотека для работы с энергонезависимой памятью (EEPROM)

// --- БИБЛИОТЕКИ ДЛЯ ДАТЧИКОВ И КОМПОНЕНТОВ ---
#include <Wire.h>               // Библиотека для работы с шиной I2C (нужна для SHT31 и DS3231)
#include <Adafruit_SHT31.h>     // Библиотека для датчика температуры и влажности SHT31
#include <RTClib.h>             // Библиотека для часов реального времени DS3231
#include <GyverEncoder.h>       // Библиотека для работы с энкодером

// --- 2. ОПРЕДЕЛЯЕМ (ASSIGN) ВСЕ ПИНЫ ---
// --- ПИНЫ ДЛЯ УПРАВЛЕНИЯ НАГРЕВОМ/ОХЛАЖДЕНИЕМ ---
#define RELAY_PIN 5             // Пин, на котором подключено реле управления

// --- ПИНЫ ДЛЯ ЭНКОДЕРА ---
#define CLK 2
#define DT 3
#define SW 4

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
Encoder enc1(CLK, DT, SW); // Создаем объект энкодера

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

// --- КОНСТАНТЫ ДЛЯ ПУНКТОВ МЕНЮ ---
// --- НОВЫЙ БЛОК: КОНСТАНТЫ ДЛЯ ПУНКТОВ МЕНЮ ---
#define MENU_ITEM_SET_TIME 1
#define MENU_ITEM_SET_TEMP 2
#define MENU_ITEM_SET_HYST 3
#define MENU_ITEM_SET_FROST 4
#define MENU_ITEM_SET_TIMER 5
#define MENU_ITEM_EXIT 6

#define MENU_ITEM_TIMER_1 1
#define MENU_ITEM_TIMER_2 2
#define MENU_ITEM_TIMER_3 3
#define MENU_ITEM_TIMER_4 4
#define MENU_ITEM_EXIT_TIMER 5

      // Режим работы по умолчанию
      String activeMode = "AUTO"; 

      // Значения температуры по умолчанию
      float targetTemperature = 0; 
      int hysteresis = 2; // Гистерезис по умолчанию

      // --- 6. ЗАДАЕМ ЗНАЧЕНИЯ ПО УМОЛЧАНИЮ (ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ) ---
      // --- ЗНАЧЕНИЯ ТАЙМЕРОВ ПО УМОЛЧАНИЮ ---
      // Используем тип 'byte', так как мы будем хранить значения от 0 до 9 (1 байт)
      byte timer1 = 0; // Таймер 1 по умолчанию выключен
      byte timer2 = 0; // Таймер 2 по умолчанию выключен
      byte timer3 = 0; // Таймер 3 по умолчанию выключен
      byte timer4 = 0; // Таймер 4 по умолчанию выключен

      
      void drawBackground();
      void drawDinamointerface();
      void drawSetpage();
      void drawTimerpage();

      // ---  ПЕРЕМЕННЫЕ ДЛЯ ЛОГИКИ ---

      // Флаг для отрисовки фона (твоя идея)
      bool isStaticDrawn = false;
      // флаг для отрисовки страницы настройки
      bool isSetPageDrawn = false;
      // Флаг для страницы таймеров 
      bool isSetTimerDrawn = false;

      // Переменная для хранения текущей страницы
      String currentPage = "MAIN_PAGE"; // Начинаем на главной странице

      // Переменные для отслеживания бездействия пользователя
      unsigned long inactivityTimer = 0; // Таймер для отслеживания последнего действия
      const long inactivityTime = 10000; // Время бездействия в мс (10000 мс = 10 сек)

      // Переменные для логики работы с меню
      bool isSelecting = false; // Флаг: находимся ли мы в режиме выбора пункта меню

      // Используем BYTE для экономии памяти, как ты и предложил.
      byte selectedMenuItem = MENU_ITEM_SET_TIME; // Текущий выбранный пункт в SET_PAGE
      byte selectedTimerItem = MENU_ITEM_TIMER_1; // Текущий выбранный пункт в SET_TIMER

      
//Блок 2 установки
void setup() {
      // --- 1. ИНИЦИАЛИЗАЦИЯ ВСЕХ КОМПОНЕНТОВ ---
      Serial.begin(9600); // запуск порта для отладки
      // Инициализируем дисплей
      tft->begin();
      tft->setRotation(3);

      // Инициализируем шину I2C для датчиков
      Wire.begin();
      
      // Инициализируем датчики и часы
      shtSensor.begin();
  

      // Инициализируем пины
      pinMode(RELAY_PIN, OUTPUT);
      digitalWrite(RELAY_PIN, HIGH); // Реле выключено по умолчанию
      // --- НАСТРОЙКА ЭНКОДЕРА ---
      enc1.setType(TYPE2); // Настраиваем тип энкодера (самый распространенный)
    
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


//-------Логика работы перехода по страницам--------

void loop() {
  // --- ОБЯЗАТЕЛЬНЫЙ ОПРОС ЭНКОДЕРА ---
  enc1.tick();

  // --- ЛОГИКА ГЛАВНОЙ СТРАНИЦЫ (MAIN_PAGE) ---
  if (currentPage == "MAIN_PAGE") {
      // --- ПРОВЕРКА УДЕРЖАНИЯ КНОПКИ (> 5 сек) ---
      if (enc1.isHolded()) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          inactivityTimer = millis();
          return;
      }

      // --- ОТРИСОВКА MAIN_PAGE ---
      if (!isStaticDrawn) { 
          drawBackground();
          isStaticDrawn = true; 
      }
      drawDinamointerface(); 

      // Заменяем delay(50) на неблокирующую задержку
      static unsigned long lastMainLoopTime = 0;
      if (millis() - lastMainLoopTime < 50) return;
      lastMainLoopTime = millis();
  }
  
  // --- ЛОГИКА СТРАНИЦЫ НАСТРОЕК (SET_PAGE) ---
  else if (currentPage == "SET_PAGE") {
      
      // --- ОБРАБОТКА СОБЫТИЙ В САМОМ НАЧАЛЕ ---
      // Это нужно, чтобы события не сбрасывались задержкой в конце.
      
      //  Проверка БЕЗДЕЙСТВИЯ (10 сек)
      if (millis() - inactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "MAIN_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          return;
      }
      
      //  Проверка ВРАЩЕНИЯ ЭНКОДЕРА
      if (enc1.isRight()) {
          inactivityTimer = millis(); 
          selectedMenuItem++; 
          if (selectedMenuItem > MENU_ITEM_EXIT) selectedMenuItem = MENU_ITEM_SET_TIME;
          drawSetpage(); 
      }
      if (enc1.isLeft()) {
          inactivityTimer = millis(); 
          selectedMenuItem--; 
          if (selectedMenuItem < MENU_ITEM_SET_TIME) selectedMenuItem = MENU_ITEM_EXIT;
          drawSetpage(); 
      }
      
      // --- ОБЪЕДИНЕННАЯ ЛОГИКА КЛИКА ПО КНОПКЕ ---
      if (enc1.isClick()) {
          // Если курсор на "EXIT" - выходим на главную
          if (selectedMenuItem == MENU_ITEM_EXIT) {
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "MAIN_PAGE";
              isStaticDrawn = false;
              isSetPageDrawn = false;
              return;
          }
          // Если курсор на "Set Timer" - переходим на страницу таймеров
          else if (selectedMenuItem == MENU_ITEM_SET_TIMER) {
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_TIMER";
              isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimerDrawn = false;
              inactivityTimer = millis();
              return;
          }
      }
      
      
      // --- ОТРИСОВКА СТРАНИЦЫ (ЕСЛИ НУЖНО) ---
      // Отрисовка происходит только если мы не вышли из функции по 'return' выше.
      if (!isSetPageDrawn) {
          drawSetpage();
          isSetPageDrawn = true; 
          inactivityTimer = millis(); 
      }

      // --- ЗАДЕРЖКА В КОНЦЕ ---
      // Теперь задержка не мешает обработке событий.
      static unsigned long lastSetLoopTime = 0;
      if (millis() - lastSetLoopTime < 50) return;
      lastSetLoopTime = millis();
  }

  // --- ЛОГИКА СТРАНИЦЫ НАСТРОЕК (SET_TIMER) ---
  else if (currentPage == "SET_TIMER") {
      // ---  ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - inactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "MAIN_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimerDrawn = false; // Не забываем сбросить флаг этой страницы!
          return;
      }

      // --- ОТРИСОВКА СТРАНИЦЫ SET_TIMER ---
      // Здесь будет код для отрисовки страницы таймеров
      // --- ОТРИСОВКА СТРАНИЦЫ SET_TIMER ---
          if (!isSetTimerDrawn) {
          drawTimerpage(); 
          isSetTimerDrawn = true; 
          inactivityTimer = millis(); 
          }
      // --- В) ЗАДЕРЖКА В КОНЦЕ ---
      static unsigned long lastSetTimerTime = 0;
      if (millis() - lastSetTimerTime < 50) return;
      lastSetTimerTime = millis();
  }
}


// Загружаем главную страницу с отрисовкой надписей и шкал приборов
void drawBackground() {
      // --- НАСТРОЙКА ПАРАМЕТРОВ ТЕКСТА ---
      // Задаем цвет (белый)
      tft->setTextColor(COLOR_WHITE); 
      // Задаем размер. Размер 2 увеличит каждый символ в 2 раза.
      tft->setTextSize(2); 
      // --- ПОЗИЦИОНИРОВАНИЕ И ВЫВОД НАДПИСИ ---
      // Устанавливаем курсор в координаты x=54, y=3 
      tft->setCursor(54, 3); 
      // Выводим надпись <Sauna Burovichok>
      tft->print("<Sauna Burovichok>");
      // Устанавливаем курсор для вывода надписи время в координаты x=10, y=20
      tft->setCursor(10, 25); 
      // Выводим надпись <TIME>
      tft->print("<TIME>");
      
      // --- Отрисовка шкалы влажности
      // --- ОТРИСОВКА СЕРОЙ ОКАНТОВКИ ---
      // Углы для влажности: от 180° до 360°
      tft->fillArc(HUM_CENTER_X, HUM_CENTER_Y, 88, 76, 180, 270, COLOR_GRAY);

      // --- РАСЧЕТ УГЛОВ ДЛЯ ВЛАЖНОСТИ (22,5 градусов на сектор) ---
      // Красный сектор (75-100%)
      tft->fillArc(HUM_CENTER_X, HUM_CENTER_Y, 75, 65, 180, 202, COLOR_RED);
      
      // Желтый сектор (50-75%)
      tft->fillArc(HUM_CENTER_X, HUM_CENTER_Y, 75, 65, 204, 224, COLOR_YELLOW);
      
      // Зеленый сектор (25-50%)
      tft->fillArc(HUM_CENTER_X, HUM_CENTER_Y, 75, 65, 226, 246, COLOR_GREEN);
      
      // Синий сектор (0-25%)
      tft->fillArc(HUM_CENTER_X, HUM_CENTER_Y, 75, 65, 248, 270, COLOR_BLUE); 
  
      // --- ОТРИСОВКА МЕТОК И ПОДПИСЕЙ ДЛЯ ВЛАЖНОСТИ ---
      int textSizeVal = 1; // Размер текста для значений

      // Задаем цвет меток и текста
      tft->setTextColor(COLOR_WHITE);
      tft->setTextSize(textSizeVal);
      tft->setCursor(18, 152);
      tft->print(0);
      tft->setCursor(36, 97);
      tft->print(50);
      tft->setCursor(89, 73);
      tft->print(100);

      // --- ПОДПИСЬ ДЛЯ ВЛАЖНОСТИ ---
      // Координаты (80, 165), //х=80, у=173
      tft->setCursor(80, 173);
      tft->setTextSize(2);
      tft->print("Hu");
      
      // --- Отрисовка шкалы температуры
      // --- ОТРИСОВКА СЕРОЙ ОКАНТОВКИ ---
      // Используем синтаксис углов: от 180 до 360 градусов
      tft->fillArc(TEMP_CENTER_X, TEMP_CENTER_Y, 102, 90, 180, 360, COLOR_GRAY);

      // --- ВЕРНЫЙ РАСЧЕТ УГЛОВ ---
      // Вся шкала: от 180 до 360 градусов. Итого: 180 градусов.
      // Делим на 4 сектора: 180 / 4 = 45 градусов на сектор.
      
      // Синий сектор (0% - 25%) - Начинаем с 180° 
      tft->fillArc(TEMP_CENTER_X, TEMP_CENTER_Y, 89, 79, 180, 224, COLOR_BLUE);
      
      // Зеленый сектор (25% - 50%) - Следующие 45 градусов
      tft->fillArc(TEMP_CENTER_X, TEMP_CENTER_Y, 89, 79, 226, 269, COLOR_GREEN);
      
      // Желтый сектор (50% - 75%)
      tft->fillArc(TEMP_CENTER_X, TEMP_CENTER_Y, 89, 79, 271, 314, COLOR_YELLOW);
      
      // Красный сектор (75% - 100%) - Заканчиваем на 360°
      tft->fillArc(TEMP_CENTER_X, TEMP_CENTER_Y, 89, 79, 316, 360, COLOR_RED);
   
      // --- ОТРИСОВКА МЕТОК И ПОДПИСЕЙ ДЛЯ ТЕМПЕРАТУРЫ ---
      //координаты меток температуры
      tft->setTextColor(COLOR_WHITE);
      tft->setTextSize(textSizeVal);
      tft->setCursor(108, 152);
      tft->print(0);
      tft->setCursor(130, 87);
      tft->print(30);
      tft->setCursor(199, 59);
      tft->print(60);
      tft->setCursor(269, 85);
      tft->print(90);
      tft->setCursor(287, 152);
      tft->print(120);  
     
      // --- ПОДПИСЬ ДЛЯ ТЕМПЕРАТУРЫ ---
      // Координаты (205, 190) - под центром шкалы температуры
      tft->setTextColor(COLOR_WHITE);
      tft->setTextSize(2); // Размер 2 для подписей
      tft->setCursor(221, 173);// координаты установки х=235, у=173
      tft->print((char)248);   // символ градусов!
      tft->setCursor(233, 173);// координаты установки х=230, у=173
      tft->print("C");
     }

// Рисуем динамические части интерфейса, стрелки digital indicator,clock, меню выбора работы   
void drawDinamointerface() {

      // временно размещаем индикацию часов для определения координат!!!! 
      tft->setCursor(15, 50); 
      // Выводим надпись <00:00>
      tft->print("00:00");
      
      // ---  СТРЕЛКИ ТЕМПЕРАТУРЫ ---
      // Линия из центра (TEMP_CENTER_X, TEMP_CENTER_Y)
      // в точку (TEMP_CENTER_X - длина, TEMP_CENTER_Y)
      tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, TEMP_CENTER_X - TEMP_NEEDLE_LENGTH, TEMP_CENTER_Y, COLOR_YELLOW);
      
      // ---  СТРЕЛКИ ВЛАЖНОСТИ ---
      // Линия из центра (HUM_CENTER_X, HUM_CENTER_Y)
      // в точку (HUM_CENTER_X - длина, HUM_CENTER_Y)
      tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, HUM_CENTER_X - HUM_NEEDLE_LENGTH, HUM_CENTER_Y, COLOR_YELLOW);
      
      //координаты цифровых индикаторов влажности
      tft->setTextColor(COLOR_WHITE);
      tft->setTextSize(2);    // Размер 2 для подписей
      tft->setCursor(40, 173);// координаты установки х=40, у=173
      tft->print("000");
      
      //координаты цифровых индикаторов температуры
      tft->setTextColor(COLOR_WHITE);
      tft->setTextSize(2);     // Размер 2 для подписей
      tft->setCursor(185, 173);// координаты установки х=185, у=173
      tft->print("000");
       
      //координаты установки иконки нагрева.
      tft->fillCircle(144, 179, 16, COLOR_DARKGREY);
      tft->setTextSize(2);
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(139, 173); //(приблизительно по центру)
      tft->print("H");
        
      //координаты иконки разморозки
      tft->fillCircle(270, 179, 16, COLOR_DARKGREY);
      tft->setTextSize(2);
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(266, 173); //(приблизительно по центру)
      tft->print("F");

      //Координаты надписи ON
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(40, 210); 
      tft->print("ON");
        
      //Координаты надписи AUTO
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(140, 210); 
      tft->print("AUTO");

      //Координаты надписи OFF
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(240, 210); 
      tft->print("OFF");
  } 

// Рисуем страницу настроек
void drawSetpage() {
      tft->fillScreen(COLOR_BACKGROUND);
      tft->setTextSize(2);
  
      // --- ЗАГОЛОВОК СТРАНИЦЫ ---
      tft->setCursor(120, 10);
      tft->setTextColor(COLOR_WHITE); 
      tft->print("SETTING");

      tft->setTextSize(1);
  
      // --- ПУНКТ 1: Set Time ---
      // Проверяем, выбран ли этот пункт (курсор на нем)
      if (selectedMenuItem == MENU_ITEM_SET_TIME) {
      tft->setTextColor(COLOR_YELLOW); // Цвет для выбранного пункта
  } 
      else {
      tft->setTextColor(COLOR_WHITE); // Цвет для невыбранных пунктов
  }
      tft->setCursor(20, 40); 
      tft->print("Set Time: ......... ");
  
      // --- ПУНКТ 2: Set Temperature ---
      if (selectedMenuItem == MENU_ITEM_SET_TEMP) {
      tft->setTextColor(COLOR_YELLOW);
  } 
      else {
      tft->setTextColor(COLOR_WHITE);
  }
      tft->setCursor(20, 55);
      tft->print("Set Temperature: .. ");

      // --- ПУНКТ 3: Set Hysteresis ---
      if (selectedMenuItem == MENU_ITEM_SET_HYST) {
      tft->setTextColor(COLOR_YELLOW);
  } 
      else {
      tft->setTextColor(COLOR_WHITE);
  }
      tft->setCursor(20, 70);
      tft->print("Set Hysteresis: ... ");

      // --- ПУНКТ 4: Set Frosting ---
      if (selectedMenuItem == MENU_ITEM_SET_FROST) {
      tft->setTextColor(COLOR_YELLOW);
  } 
      else {
      tft->setTextColor(COLOR_WHITE);
  }
      tft->setCursor(20, 85);
      tft->print("Set Frosting: ..... ");

      // --- ПУНКТ 5: Set Timer ---
      if (selectedMenuItem == MENU_ITEM_SET_TIMER) {
      tft->setTextColor(COLOR_YELLOW);
  } 
      else {
      tft->setTextColor(COLOR_WHITE);
  }
      tft->setCursor(20, 100);
      tft->print("Set Timer:");

      // --- ПУНКТ 6: EXIT ---
      if (selectedMenuItem == MENU_ITEM_EXIT) {
      tft->setTextColor(COLOR_YELLOW);
  } 
      else {
      tft->setTextColor(COLOR_WHITE);
  }
      tft->setCursor(20, 115);
      tft->print("EXIT");
}

//Рисуем страницу для установки таймеров
void drawTimerpage() {
     tft->fillScreen(COLOR_BACKGROUND);
     tft->setTextColor(COLOR_WHITE); 
     tft->setTextSize(2);

     // --- ЗАГОЛОВОК СТРАНИЦЫ ---
     tft->setCursor(85, 10); 
     tft->print("TIMER SETTINGS");

     // --- ТАЙМЕР 1 ---
     tft->setTextSize(1);
     tft->setCursor(30, 55);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer 1");

     // Меню таймера
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->setCursor(15, 70);
     tft->print("Timer ON: .... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // Отключение по таймеру
     tft->setCursor(15, 85);
     tft->print("Timer OFF: ... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // запись в EPROM/ удаление из EPROM 
     tft->setCursor(15, 100);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer Memory/Clear");

     // --- ТАЙМЕР 2 ---
     tft->setCursor(30, 130);
     tft->setTextColor(COLOR_WHITE);
     tft->print("Timer 2");

     // Меню таймера
     tft->setCursor(15, 145);
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("Timer ON: .... ");
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // Отключение по таймеру
     tft->setCursor(15, 160);
     tft->print("Timer OFF: ... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // запись в EPROM/ удаление из EPROM 
     tft->setCursor(15, 175);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer Memory/Clear");

     // --- ТАЙМЕР 3 ---
     tft->setTextSize(1);
     tft->setCursor(190, 55);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer 3");

     // Меню таймера
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->setCursor(175, 70);
     tft->print("Timer ON: .... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // Отключение по таймеру
     tft->setCursor(175, 85);
     tft->print("Timer OFF: ... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // запись в EPROM/ удаление из EPROM 
     tft->setCursor(175, 100);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer Memory/Clear");

     // --- ТАЙМЕР 4 ---
     tft->setCursor(190, 130);
     tft->setTextColor(COLOR_WHITE);
     tft->print("Timer 4");

     // Меню таймера
     tft->setCursor(175, 145);
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("Timer ON: .... ");
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // Отключение по таймеру
     tft->setCursor(175, 160);
     tft->print("Timer OFF: ... ");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 
     tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

     // запись в EPROM/ удаление из EPROM 
     tft->setCursor(175, 175);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer Memory/Clear");

     // Устанавливаем надпись выход на "drawBackground" на главнуй страницу
     tft->setCursor(148, 200);
     tft->print("EXIT");
  }  
