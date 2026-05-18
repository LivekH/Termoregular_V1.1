//Блок 1 установка библиотек, назначение пинов установка констант для стрелок, цветов, режимов работы
// --- ПОДКЛЮЧАЕМ ВСЕ НЕОБХОДИМЫЕ БИБЛИОТЕКИ ---

#include <Arduino_GFX_Library.h> // Библиотека для работы с дисплеем
#include <EEPROM.h>              // Библиотека для работы с энергонезависимой памятью (EEPROM)

// --- БИБЛИОТЕКИ ДЛЯ ДАТЧИКОВ И КОМПОНЕНТОВ ---
#include <Wire.h>               // Библиотека для работы с шиной I2C (нужна для SHT31 и DS3231)
#include <Adafruit_SHT31.h>     // Библиотека для датчика температуры и влажности SHT31
#include <RTClib.h>             // Библиотека для часов реального времени DS3231
#include <GyverEncoder.h>       // Библиотека для работы с энкодером

// --- 2. ОПРЕДЕЛЯЕМ (ASSIGN) ВСЕ ПИНЫ ---
// --- ПИНЫ ДЛЯ УПРАВЛЕНИЯ НАГРЕВОМ/ОХЛАЖДЕНИЕМ ---
#define RELAY_PIN 39             // Пин, на котором подключено реле управления

// --- ПИНЫ ДЛЯ ЭНКОДЕРА ---
#define CLK 36
#define DT 38
#define SW 40

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

// ---  ОПРЕДЕЛЯЕМ КОНСТАНТЫ (КООРДИНАТЫ, РАЗМЕРЫ) ---

// --- КООРДИНАТЫ ЦЕНТРОВ ПРИБОРОВ (ОСНОВАНИЙ СТРЕЛОК) ---
#define TEMP_CENTER_X 205 
#define TEMP_CENTER_Y 157 
#define HUM_CENTER_X 100  
#define HUM_CENTER_Y 157  

// --- ДЛИНА СТРЕЛОК В ПИКСЕЛЯХ ---
#define TEMP_NEEDLE_LENGTH 77 
#define HUM_NEEDLE_LENGTH 63 

// ---  ЗАДАЕМ ЦВЕТА В ФОРМАТЕ RGB565 (ЧИСЛОВЫЕ ЗНАЧЕНИЯ) ---
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
#define MENU_ITEM_SET_TIMER_1 5
#define MENU_ITEM_SET_TIMER_2 6
#define MENU_ITEM_SET_TIMER_3 7
#define MENU_ITEM_SET_TIMER_4 8
#define MENU_ITEM_EXIT 9
//КОНСТАНТЫ ДЛЯ МЕНЮ SET_TIME
#define TIME_HOURS 1
#define TIME_MINUTES 2
#define TIME_EXIT 3
// --- КОНСТАНТЫ ДЛЯ РЕЖИМА НАСТРОЙКИ ВРЕМЕНИ ---
#define NAVIGATING 1
#define EDITING_HOURS 2
#define EDITING_MINUTES 3
// --- КОНСТАНТЫ ДЛЯ МЕНЮ УСТАНОВКИ ТЕМПЕРАТУРЫ ---
#define SET_TEMP 1
#define TEMP_EXIT 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ НАВИГАЦИИ УСТАНОВКИ ТЕМПЕРАТУРЫ ---
#define TEMP_NAVIGATING 1
#define TEMP_EDITING 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ УСТАНОВКИ ГИСТЕРЕЗИСА ---
#define SET_HYST 1
#define HYST_EXIT 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ НАВИГАЦИИ УСТАНОВКИ ГИСТЕРЕЗИСА ---
#define HYST_NAVIGATING 1
#define HYST_EDITING 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ УСТАНОВКИ ГИСТЕРЕЗИСА ---
#define SET_FROST 1
#define FROST_EXIT 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ НАВИГАЦИИ УСТАНОВКИ ГИСТЕРЕЗИСА ---
#define FROST_NAVIGATING 1
#define FROST_EDITING 2
// --- КОНСТАНТЫ ДЛЯ МЕНЮ УСТАНОВКИ ТАЙМЕРОВ ---
#define TIMER_H_ON 1
#define TIMER_M_ON 2
#define TIMER_H_OFF 3
#define TIMER_M_OFF 4
#define TIMER_SAVE 5
#define TIMER_CLEAR 6
#define TIMER_EXIT 7
// --- КОНСТАНТЫ ДЛЯ МЕНЮ НАВИГАЦИИ УСТАНОВКИ ТАЙМЕРОВ  ---
#define TIMER_NAVIGATING 1
#define TIMER_ON_EDITING_H 2
#define TIMER_ON_EDITING_M 3
#define TIMER_OFF_EDITING_H 4
#define TIMER_OFF_EDITING_M 5




// Режим работы по умолчанию
String activeMode = "AUTO";

     
// --- ЗАДАЕМ ЗНАЧЕНИЯ ПО УМОЛЧАНИЮ (ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ) ---
      // --- ЗНАЧЕНИЯ ТАЙМЕРОВ ПО УМОЛЧАНИЮ ---
      // Используем тип 'byte', так как мы будем хранить значения от 0 до 9 (1 байт)
      byte t1HourON1 = 255; // Таймер 1 ON по умолчанию выключен
      byte t2MinuteON1 = 255; // Таймер 1 ON по умолчанию выключен
      byte t3HourOFF1 = 255; // Таймер 1 OFF по умолчанию выключен
      byte t4MinuteOFF1 = 255; // Таймер  OFF по умолчанию выключен
      
      byte t5HourON2 = 255; // 
      byte t6MinuteON2 = 255; // 
      byte t7HourOFF2 = 255; // 
      byte t8MinuteOFF2 = 255; // 
      
      byte t9HourON3 = 255; // 
      byte t10MinuteON3 = 255; // 
      byte t11HourOFF3 = 255; // 
      byte t12MinuteOFF3 = 255; // 

      byte t13HourON4 = 255; // 
      byte t14MinuteON4 = 255; // 
      byte t15HourOFF4 = 255; // 
      byte t16MinuteOFF4 = 255; // 

      // --- ОБЪЯВЛЕНИЕ ФУНКЦИЙ ---
      void drawBackground();      // Страница отрисовки ыонового изображения
      void drawDinamointerface(); // Страница отрисовки динамического интерфейса
      void drawSetpage();         // Cтраница отрисовки меню Set Page
      void drawSetTimepage();     // Cтраница отрисовки меню Set Time
      void drawSetTemppage();     // Cтраница отрисовки меню Set Temp
      void drawSetHystpage();     // Cтраница отрисовки меню Set Hyst
      void drawSetFrostpage();     // Cтраница отрисовки меню Set Hyst
      void drawSetTimerpage();       
      void updateSetPageItem(byte itemIndex, bool isSelected); // список пунктов меню Set Page
      void updateSetTimeItem(byte itemIndex, bool isSelected); // список пунктов меню Set Time
      void updateSetTempItem(byte itemIndex, bool isSelected); // список пунктов меню Set Temp
      void updateSetHystItem(byte itemIndex, bool isSelected); // список пунктов меню Set Hyst
      void updateSetFrostItem(byte itemIndex, bool isSelected); // список пунктов меню Set Hyst
      void updateSetTimerItem(byte itemIndex, bool isSelected); // список пунктов меню Set Timer
      
      // ---  ПЕРЕМЕННЫЕ ДЛЯ ЛОГИКИ ---
      // Переменная для хранения текущей страницы
      String currentPage = "MAIN_PAGE"; // Начинаем на главной странице
      // Флаг для отрисовки фона (твоя идея)
      bool isStaticDrawn = false;
      // флаг для отрисовки страницы настройки
      bool isSetPageDrawn = false;
      // Флаг для отрисовки страницы установки времени
      bool isSetTimePageDrawn = false;
      // Флаг для отрисовки страницы установки температуры
      bool isSetTempPageDrawn = false;
      // Флаг для отрисовки страницы установки гистерезиса
      bool isSetHystPageDrawn = false;
      // Флаг для отрисовки страницы установки разморозки
      bool isSetFrostPageDrawn = false;
      // Флаг для отрисовки страницы установки таймеров
      bool isSetTimerPageDrawn = false;
      
      // Переменные для отслеживания бездействия пользователя
      const unsigned long inactivityTime = 10000; // 10 секунд в миллисекундах
      unsigned long mainInactivityTimer = 0; // 
      unsigned long setInactivityTimer = 0;  // 
      unsigned long timeInactivityTimer = 0; //
      unsigned long tempInactivityTimer = 0; //
      unsigned long hystInactivityTimer = 0; //
      unsigned long frostInactivityTimer = 0; // 
      unsigned long timerInactivityTimer = 0; 

      // Переменные для логики работы с меню
      bool isSelecting = false; // Флаг: находимся ли мы в режиме выбора пункта меню

      // Используем BYTE для экономии памяти, как ты и предложил.
      byte selectedMenuItem = MENU_ITEM_SET_TIME; // Текущий выбранный пункт в SET_PAGE
      // Переменная для меню часов
      byte selectedTimeItem = TIME_EXIT;
      // Переменная для меню температуры
      byte selectedTempItem = TEMP_EXIT;
      // Переменная для меню гистерезиса
      byte selectedHystItem = HYST_EXIT;
      // Переменная для меню разморозки
      byte selectedFrostItem = FROST_EXIT;
      // Переменная для меню таймеров
      byte selectedTimerItem = TIMER_EXIT;
       // --- ПЕРЕМЕННАЯ СОСТОЯНИЯ ---
      byte timeMenuState = NAVIGATING;      // Начинаем в режиме навигации времени
      byte tempMenuState = TEMP_NAVIGATING; // Начинаем в режиме меню навигации температуры
      byte hystMenuState = HYST_NAVIGATING; // Начинаем в режиме меню навигации гистерезиса
      byte frostMenuState = FROST_NAVIGATING; // Начинаем в режиме меню навигации разморозки
      byte timerMenuState = TIMER_NAVIGATING;



           
      // Новая переменная: хранит номер таймера, который мы настраиваем (1,2,3 или 4)
      byte activeTimerNumber = 0;

      // Значения температуры по умолчанию
      int targetTemp = 0;
      
      // Для температуры и влажности стрелочных указателей
      int lastTemperature = -1;
      int lastHumidity = -1;

       // Значения для Гистерезиса
      int targetHyst = 2; // Гистерезис по умолчанию

      // Значения для Разморозки
      int targetFrost = 0; // Гистерезис по умолчанию
      
      // Для установки часов и минут
      int targetHour = 0;
      int targetMinute = 0;
      int lastHour = -1;   // Используем -1 для инициализации (заставит отрисоваться при старте)
      int lastMinute = -1;



// --- Блок 2 установки ---

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
      rtc.begin();

      // Инициализируем пины
      pinMode(RELAY_PIN, OUTPUT);
      digitalWrite(RELAY_PIN, HIGH); // Реле выключено по умолчанию
      
      // --- НАСТРОЙКА ЭНКОДЕРА ---
      enc1.setType(TYPE2); // Настраиваем тип энкодера (самый распространенный)
    
      // --- ЧТЕНИЕ ДАННЫХ ИЗ EEPROM ---
      // Читаем сохраненный режим работы (по адресу 0)
      char savedMode = EEPROM.read(0);
      if (savedMode == 'o') activeMode = "ON";
      if (savedMode == 'a') activeMode = "AUTO";
      if (savedMode == 'f') activeMode = "OFF";

      // --- ЧТЕНИЕ ЗНАЧЕНИЯ ТЕМПЕРАТУРЫ ИЗ EEPROM ПРИ СТАРТЕ ---
      targetTemp = EEPROM.read(1);
      // --- ЗАЩИТА ОТ "МУСОРА" ---
      // Если в памяти лежит 255 (пусто) или число больше нашего максимума (115)
      if (targetTemp == 255 || targetTemp > 115) {
      targetTemp = 10; // Устанавливаем безопасное значение по умолчанию
      }

      // Читаем сохраненный гистерезис (по адресу 2)
      targetHyst = EEPROM.read(2);
      if (targetHyst == 255 || targetHyst > 25) {
      targetHyst = 2; // Устанавливаем безопасное значение по умолчанию
      }

      // Читаем сохраненные таймеры (по адресам  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19)
      t1HourON1 = EEPROM.read(4);
      t2MinuteON1 = EEPROM.read(5);
      t3HourOFF1 = EEPROM.read(6);
      t4MinuteOFF1 = EEPROM.read(7);
      
      t5HourON2 = EEPROM.read(8);
      t6MinuteON2 = EEPROM.read(9);
      t7HourOFF2 = EEPROM.read(10);
      t8MinuteOFF2 = EEPROM.read(11);

      t9HourON3 = EEPROM.read(12);
      t10MinuteON3 = EEPROM.read(13);
      t11HourOFF3 = EEPROM.read(14);
      t12MinuteOFF3 = EEPROM.read(15);
      
      t13HourON4 = EEPROM.read(16);
      t14MinuteON4 = EEPROM.read(17);
      t15HourOFF4 = EEPROM.read(18);
      t16MinuteOFF4 = EEPROM.read(19);


      // --- ДИАГНОСТИЧЕСКИЙ ОТЧЕТ ПРИ СТАРТЕ ---
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

      // Получаем текущее время с часов и сохраняем его внутри объекта rtcClock
      DateTime now = rtc.now();

      // Выводим время на экран, используя данные из объекта
      // Добавляем проверку на "0" для красоты (чтобы было "09", а не "9")
      //if (rtc.now < 10) tft->print("0");
      tft->print(now.hour());
      tft->print(":");
      if (now.minute() < 10) {
      tft->print("0");
    }
      tft->print(now.minute());
  
  delay(1000);
  
      tft->setCursor(10, 60);
      tft->print("Temp Sensor: ");
      tft->print(shtSensor.readTemperature());tft->print((char)248);tft->print("C"); // Получаем температуру с датчика
  
  delay(1000);
  
      tft->setCursor(10, 80);
      tft->print("Set Temp (EEPROM): ");
      tft->print(targetTemp);tft->print((char)248);tft->print("C"); // Значение из памяти
  
  delay(1000);
  
      tft->setCursor(10, 100);
      tft->print("Hysteresis (EEPROM): ");
      tft->print(targetHyst); tft->print((char)248);tft->print("C"); // Значение из памяти
  
  delay(1000);
  
      tft->setCursor(10, 120);
      tft->print("Timers (EEPROM): ");

      // --- ПРОВЕРКА ЧАСОВ ВКЛЮЧЕНИЯ ТАЙМЕРА 1 ---
      tft->setCursor(10, 140);
tft->print("Timer 1-"); tft->print(" ON ");
if (t1HourON1 >= 0 && t1HourON1 <= 23) {
    tft->print(t1HourON1);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВКЛЮЧЕНИЯ ТАЙМЕРА 1 ---
if (t2MinuteON1 >= 0 && t2MinuteON1 <= 59) {
    tft->print(t2MinuteON1);
} else {
    tft->print("--");
}

// --- ПРОВЕРКА ЧАСОВ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 1 ---
tft->print(" - OFF ");
if (t3HourOFF1 >= 0 && t3HourOFF1 <= 23) {
    tft->print(t3HourOFF1);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 1 ---
if (t4MinuteOFF1 >= 0 && t4MinuteOFF1 <= 59) {
    tft->print(t4MinuteOFF1);
} else {
    tft->print("--");
}
//--- ПРОВЕРКА ТАЙМЕРА 1 ЗАВЕРШЕНА ---

// --- ПРОВЕРКА ЧАСОВ ВКЛЮЧЕНИЯ ТАЙМЕРА 2 ---
      tft->setCursor(10, 160);
tft->print("Timer 2-"); tft->print(" ON ");


if (t5HourON2 >= 0 && t5HourON2 <= 23) {
    tft->print(t5HourON2);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВКЛЮЧЕНИЯ ТАЙМЕРА 2 ---
if (t6MinuteON2 >= 0 && t6MinuteON2 <= 59) {
    tft->print(t6MinuteON2);
} else {
    tft->print("--");
}

// --- ПРОВЕРКА ЧАСОВ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 2 ---
tft->print(" - OFF ");
if (t7HourOFF2 >= 0 && t7HourOFF2 <= 23) {
    tft->print(t7HourOFF2);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 2 ---
if (t8MinuteOFF2 >= 0 && t8MinuteOFF2 <= 59) {
    tft->print(t8MinuteOFF2);
} else {
    tft->print("--");
}
      //--- ПРОВЕРКА ТАЙМЕРА 2 ЗАВЕРШЕНА ---

// --- ПРОВЕРКА ЧАСОВ ВКЛЮЧЕНИЯ ТАЙМЕРА 3 ---
      tft->setCursor(10, 180);
tft->print("Timer 3-"); tft->print(" ON ");


if (t9HourON3 >= 0 && t9HourON3 <= 23) {
    tft->print(t9HourON3);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВКЛЮЧЕНИЯ ТАЙМЕРА 3 ---
if (t10MinuteON3 >= 0 && t10MinuteON3 <= 59) {
    tft->print(t10MinuteON3);
} else {
    tft->print("--");
}

// --- ПРОВЕРКА ЧАСОВ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 3 ---
tft->print(" - OFF ");
if (t11HourOFF3 >= 0 && t11HourOFF3 <= 23) {
    tft->print(t11HourOFF3);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 3 ---
if (t12MinuteOFF3 >= 0 && t12MinuteOFF3 <= 59) {
    tft->print(t12MinuteOFF3);
} else {
    tft->print("--");
}
      //--- ПРОВЕРКА ТАЙМЕРА 3 ЗАВЕРШЕНА ---

// --- ПРОВЕРКА ЧАСОВ ВКЛЮЧЕНИЯ ТАЙМЕРА 4 ---
      tft->setCursor(10, 200);
tft->print("Timer 4-"); tft->print(" ON ");


if (t13HourON4 >= 0 && t13HourON4 <= 23) {
    tft->print(t13HourON4);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВКЛЮЧЕНИЯ ТАЙМЕРА 4 ---
if (t14MinuteON4 >= 0 && t14MinuteON4 <= 59) {
    tft->print(t14MinuteON4);
} else {
    tft->print("--");
}

// --- ПРОВЕРКА ЧАСОВ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 4 ---
tft->print(" - OFF ");
if (t15HourOFF4 >= 0 && t15HourOFF4 <= 23) {
    tft->print(t15HourOFF4);
} else {
    tft->print("--");
}
tft->print(":");

// --- ПРОВЕРКА МИНУТ ВЫКЛЮЧЕНИЯ ТАЙМЕРА 4 ---
if (t16MinuteOFF4 >= 0 && t16MinuteOFF4 <= 59) {
    tft->print(t16MinuteOFF4);
} else {
    tft->print("--");
}
//--- ПРОВЕРКА ТАЙМЕРА 4 ЗАВЕРШЕНА ---  
  // ---  ЗАВЕРШЕНИЕ SETUP ---
  // Ждем 5 секунд, чтобы можно было прочитать отчет
  delay(5000);
  
      // Полностью стираем экран перед переходом в основной цикл
      tft->fillScreen(COLOR_BACKGROUND);
}


//--- Блок 3 основной цикл ---

  void loop() {
  // --- ОБЯЗАТЕЛЬНЫЙ ОПРОС ЭНКОДЕРА ---
  enc1.tick(); 

  // --- ЛОГИКА ГЛАВНОЙ СТРАНИЦЫ (MAIN_PAGE) ---
  if (currentPage == "MAIN_PAGE") {
      if (enc1.isHolded()) {
          tft->fillScreen(COLOR_BACKGROUND);
          selectedMenuItem = MENU_ITEM_SET_TIME; // КУРСОР НА ПЕРВЫЙ ПУНКТ
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
      }

      // --- ОТРИСОВКА MAIN_PAGE ---
      if (!isStaticDrawn) { 
          drawBackground();
          isStaticDrawn = true; 
      }
      drawDinamointerface();
      //EEPROM.update(1, targetTemp);
      targetTemp = EEPROM.read(1);
      targetHyst = EEPROM.read(2);
      targetFrost = EEPROM.read(3); 

      // Неблокирующая задержка
      static unsigned long lastMainLoopTime = 0;
      if (millis() - lastMainLoopTime < 50) return;
      lastMainLoopTime = millis();
  }

  // --- ЛОГИКА СТРАНИЦЫ НАСТРОЕК (SET_PAGE) ---
  else if (currentPage == "SET_PAGE") {
      // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - setInactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "MAIN_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
      }
          // --- ОТРИСОВКА СТРАНИЦЫ SET_PAGE ---
              if (!isSetPageDrawn) {
              drawSetpage();
              isSetPageDrawn = true;
              }
              targetTemp = EEPROM.read(1);
              targetHyst = EEPROM.read(2);
              targetFrost = EEPROM.read(3);

      // --- ПРОВЕРКА ВРАЩЕНИЯ ЭНКОДЕРА ---
      byte prevItem = selectedMenuItem;
      if (enc1.isRight()) {
          setInactivityTimer = millis();
          selectedMenuItem++;
          if (selectedMenuItem > MENU_ITEM_EXIT) selectedMenuItem = MENU_ITEM_SET_TIME;
          updateSetPageItem(prevItem, false);
          updateSetPageItem(selectedMenuItem, true);
      }
      if (enc1.isLeft()) {
          setInactivityTimer = millis();
          selectedMenuItem--;
          if (selectedMenuItem < MENU_ITEM_SET_TIME) selectedMenuItem = MENU_ITEM_EXIT;
          updateSetPageItem(prevItem, false);
          updateSetPageItem(selectedMenuItem, true);
      }
          
      

      // --- ОБЪЕДИНЕННАЯ ЛОГИКА КЛИКА ПО КНОПКЕ ---
      if (enc1.isClick()) {
          setInactivityTimer = millis();

          // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
           if (selectedMenuItem == MENU_ITEM_EXIT) {
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "MAIN_PAGE";
              isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetFrostPageDrawn = false;
              isSetHystPageDrawn = false;
              isSetTimerPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();
              timerInactivityTimer = millis(); 
              return;
          }

          // --- ПЕРЕХОД В ОКНО УСТАНОВКИ ВРЕМЕНИ ---
          else if (selectedMenuItem == MENU_ITEM_SET_TIME) {
              selectedTimeItem = TIME_EXIT; // Сбрасываем позицию курсора
              timeMenuState = NAVIGATING;
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_TIME_PAGE";
             /* isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();*/
              return;
           }
           
           // --- ПЕРЕХОД В ОКНО УСТАНОВКИ ТЕМПЕРАТУРЫ ---
          else if (selectedMenuItem == MENU_ITEM_SET_TEMP) {
              // --- СБРОС КУРСОРА ПРИ ВХОДЕ НА СТРАНИЦУ ---
              selectedTempItem = TEMP_EXIT;
              tempMenuState = TEMP_NAVIGATING; 
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_TEMP_PAGE";
             /* isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis(); */
              return;
          }

              // --- ПЕРЕХОД В ОКНО УСТАНОВКИ ГИСТЕРЕЗИСА ---
          else if (selectedMenuItem == MENU_ITEM_SET_HYST) {
              // --- СБРОС КУРСОРА ПРИ ВХОДЕ НА СТРАНИЦУ ---
              selectedHystItem = HYST_EXIT;
              hystMenuState = HYST_NAVIGATING; 
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_HYST_PAGE";
              /*isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetHystPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();*/
              return;
          }   


              // --- ПЕРЕХОД В ОКНО УСТАНОВКИ РАЗМОРОЗКИ 
              //(ПОДДЕРЖАНИЕ ТЕМПЕРАТУРЫ ПРИ УСЛОВИИ ОТКЛЮЧЕНИЯ ПО ТАЙМЕРАМ) ---
         else if (selectedMenuItem == MENU_ITEM_SET_FROST) {
              selectedTimeItem = FROST_EXIT; // Сбрасываем позицию курсора
              frostMenuState = FROST_NAVIGATING;
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_FROST_PAGE";
              /*isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetHystPageDrawn = false;
              isSetFrostPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();*/
              return;
          }

               // --- ПЕРЕХОД В ОКНО УСТАНОВКИ ТАЙМЕР 1 --- 
              else if (selectedMenuItem == MENU_ITEM_SET_TIMER_1) {
              selectedTimerItem = TIMER_EXIT; // Сбрасываем позицию курсора
              activeTimerNumber = 1;
              timerMenuState = TIMER_NAVIGATING;
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_TIMER_1_PAGE";
            }
            
           //<<< Вставляем сюда следующие страницы логики вращения энкодера в меню set page          
              
              static unsigned long lastSetLoopTime = 0;
              if (millis() - lastSetLoopTime < 50) return;
              lastSetLoopTime = millis(); 
    }
    
           
  } ///скобка закрытия else if (currentPage == "SET_PAGE")

  
  // --- ЛОГИКА СТРАНИЦЫ УСТАНОВКИ ВРЕМЕНИ (SET_TIME_PAGE) ---
  else if (currentPage == "SET_TIME_PAGE") {
      // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - timeInactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetFrostPageDrawn = false;
              isSetTimerPageDrawn = false;
              isSetHystPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();
              timerInactivityTimer = millis();
          return;
      }

           // Получаем текущее время с RTC для отображения и сохранения
              DateTime now = rtc.now();
              int currentHour = now.hour();
              int currentMinute = now.minute();

          // При первом входе на страницу, загружаем текущее время в переменные для редактирования
          if (!isSetTimePageDrawn) {
              drawSetTimepage();     // ...тогда рисуем
              isSetTimePageDrawn = true; // И ставим флаг, что она нарисована
              targetHour = currentHour;
              targetMinute = currentMinute;
      }
          // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА (НАВИГАЦИЯ) ---
          // Эта часть работает только если мы НЕ в режиме редактирования
          if (timeMenuState == NAVIGATING) {
              byte prevTimeItem = selectedTimeItem; 

          if (enc1.isRight()) {
              timeInactivityTimer = millis();
              selectedTimeItem++;
              if (selectedTimeItem > TIME_EXIT) selectedTimeItem = TIME_HOURS;  
              updateSetTimeItem(prevTimeItem, false);
              updateSetTimeItem(selectedTimeItem, true);}
          
          if (enc1.isLeft()) {
              timeInactivityTimer = millis();
              selectedTimeItem--;
              if (selectedTimeItem < TIME_HOURS) selectedTimeItem = TIME_EXIT; 
              updateSetTimeItem(prevTimeItem, false);
              updateSetTimeItem(selectedTimeItem, true);}
           }

      // --- ЛОГИКА КЛИКА ПО КНОПКЕ (ВХОД/ВЫХОД ИЗ РЕДАКТИРОВАНИЯ) ---
      if (enc1.isClick()) {
          timeInactivityTimer = millis();

          // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
          if (selectedTimeItem == TIME_EXIT) {
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_PAGE";
              timeMenuState = NAVIGATING;
              isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetFrostPageDrawn = false;
              isSetHystPageDrawn = false;
              isSetTimerPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();
              timerInactivityTimer = millis();
              return;
          }

           // --- ВХОД/ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
           // Проверяем клик на часы или минуты
           if (selectedTimeItem == TIME_HOURS || selectedTimeItem == TIME_MINUTES) {
           // Если мы УЖЕ находимся в режиме редактирования этого же пункта, значит это клик на "Сохранить"
           if ((selectedTimeItem == TIME_HOURS && timeMenuState == EDITING_HOURS) || 
           (selectedTimeItem == TIME_MINUTES && timeMenuState == EDITING_MINUTES)) {
                   
           // Выходим из режима редактирования
           timeMenuState = NAVIGATING;}
                        
            else {
           // Входим в режим редактирования выбранного пункта
           timeMenuState = (selectedTimeItem == TIME_HOURS) ? EDITING_HOURS : EDITING_MINUTES;
          }
         }
       }
          // --- ЛОГИКА УСТАНОВКИ ЗНАЧЕНИЯ ПРИ ВРАЩЕНИИ В РЕЖИМЕ РЕДАКТИРОВАНИЯ ---
          // Эта часть работает ТОЛЬКО если мы в режиме редактирования
          // --- ЧИСТАЯ ЛОГИКА ВРАЩЕНИЯ (БЕЗ АНТИДРЕБЕЗГА) ---     
          // --- БЛОК ДЛЯ ЧАСОВ ---
          if (timeMenuState == EDITING_HOURS) {
          // Если энкодер повернулся вправо
          if (enc1.isRight()) {
              timeInactivityTimer = millis();
              targetHour++;
              if (targetHour > 23) targetHour = 0;

              // Сохраняем в RTC              
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), targetHour, targetMinute, 0));
          }
          // Если энкодер повернулся влево
          if (enc1.isLeft()) {
              timeInactivityTimer = millis();
              targetHour--;
              if (targetHour < 0) targetHour = 23;

              // Сохраняем в RTC              
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), targetHour, targetMinute, 0));
          }
      }

          // --- БЛОК ДЛЯ МИНУТ ---
          if (timeMenuState == EDITING_MINUTES) {
          if (enc1.isRight()) {
              timeInactivityTimer = millis();
              targetMinute++;
              if (targetMinute > 59) targetMinute = 0;

              // Сохраняем в RTC         
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), targetHour, targetMinute, 0));
          }
          if (enc1.isLeft()) {
              timeInactivityTimer = millis();
              targetMinute--;
              if (targetMinute < 0) targetMinute = 59;

              // Сохраняем в RTC
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), targetHour, targetMinute, 0));
          }
      }
                     
      // --- ОТРИСОВКА ВРЕМЕНИ НА ЭКРАНЕ ---
      // --- ЛОГИКА ДЛЯ ЧАСОВ ---
      tft->setTextSize(4);
      tft->setTextColor( (timeMenuState == EDITING_HOURS) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(102, 110);
          if (currentHour < 10) tft->print("0");
          tft->print(currentHour);
      if (currentHour != targetHour) {
          // 1. СТИРАЕМ СТАРЫЕ ЧАСЫ цветом фона
          tft->setTextSize(4);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(102, 110);
          if (targetHour < 10) tft->print("0");
          tft->print(targetHour);
          
          // РИСУЕМ НОВЫЕ ЧАСЫ. Цвет зависит от режима.
          tft->setTextSize(4);
          tft->setTextColor( (timeMenuState == EDITING_HOURS) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(102, 110);
       if (currentHour < 10) tft->print("0");
          tft->print(currentHour);
          targetHour = currentHour; // Обновляем для следующего сравнения
      }

          // --- ЛОГИКА ДЛЯ МИНУТ ---
          tft->setTextSize(4);
          tft->setTextColor( (timeMenuState == EDITING_MINUTES) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(180, 110);
          if (currentMinute < 10) tft->print("0");
          tft->print(currentMinute);
      if (currentMinute != targetMinute) {
          // СТИРАЕМ СТАРЫЕ МИНУТЫ
          tft->setTextSize(4);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(180, 110);
          if (targetMinute < 10) tft->print("0");
          tft->print(targetMinute);
          
          // РИСУЕМ НОВЫЕ МИНУТЫ. Цвет зависит от режима.
          tft->setTextSize(4);
          tft->setTextColor( (timeMenuState == EDITING_MINUTES) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(180, 110);
          if (currentMinute < 10) tft->print("0");
          tft->print(currentMinute);
          targetMinute = currentMinute;
      }
          static unsigned long lastSetTimeLoopTime = 0;
          if (millis() - lastSetTimeLoopTime < 50) return;
          lastSetTimeLoopTime = millis();
      }      

   
          // --- ЛОГИКА СТРАНИЦЫ УСТАНОВКИ ТЕМПЕРАТУРЫ (SET_TEMP_PAGE) ---
        if (currentPage == "SET_TEMP_PAGE") {
          // ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА ПРИ ВХОДЕ НА СТРАНИЦУ
          // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
        if (millis() - tempInactivityTimer > inactivityTime) {
        tft->fillScreen(COLOR_BACKGROUND);
        targetTemp = EEPROM.read(1);
        currentPage = "SET_PAGE";
              isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetFrostPageDrawn = false;
              isSetHystPageDrawn = false;
              isSetTimerPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();
              timerInactivityTimer = millis();
              return;
         }
  
          // --- ЗАГРУЖАЕМ СТРАНИЦУ SetTemppage ЕСЛИ ОНА НЕ ЗАГРУЖЕНА ---
          if (!isSetTempPageDrawn) {
              drawSetTemppage();
              isSetTempPageDrawn = true;
              targetTemp = EEPROM.read(1);
          }
              static int lastTargetTemp = -1;
              lastTargetTemp = targetTemp;
      
              // --- ЛОГИКА КЛИКА ПО КНОПКЕ ---
              if (enc1.isClick()) {
              tempInactivityTimer = millis();

            // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
          if (selectedTempItem == TEMP_EXIT) {
              tft->fillScreen(COLOR_BACKGROUND);
              currentPage = "SET_PAGE";
            //tempMenuState = TEMP_NAVIGATING;
           EEPROM.update(1, targetTemp); //Сохроняем в EEPROM если значения изменились
              isStaticDrawn = false;
              isSetPageDrawn = false;
              isSetTimePageDrawn = false;
              isSetTempPageDrawn = false;
              isSetFrostPageDrawn = false;
              isSetHystPageDrawn = false;
              isSetTimerPageDrawn = false;
              mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
              tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
              setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
              timeInactivityTimer = millis();
              hystInactivityTimer = millis();
              frostInactivityTimer = millis();
              timerInactivityTimer = millis();
      return;
    }
    // --- ВХОД/ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
    if (selectedTempItem == SET_TEMP) {
        // Если мы УЖЕ в режиме редактирования, значит это клик на "ВЫХОД ИЗ РЕЖИМА"
        if (tempMenuState == TEMP_EDITING) {
            tempMenuState = TEMP_NAVIGATING; // Выключаем режим редактирования
            // Цвет цифр изменится на белый автоматически благодаря нашей "умной" строчке
        } else {
            // Если мы НЕ в режиме редактирования, значит это клик на "ВХОД В РЕЖИМ"
            tempMenuState = TEMP_EDITING; }
        }
      }
      // --- ЕСЛИ МЫ В РЕЖИМЕ НАВИГАЦИИ (МЕНЯЕМ ВЫДЕЛЕНИЕ) ---
      if (tempMenuState == TEMP_NAVIGATING) {
    byte prevTempItem = selectedTempItem;
    if (enc1.isRight()) {
      tempInactivityTimer = millis();
      selectedTempItem++;
      if (selectedTempItem > TEMP_EXIT) selectedTempItem = SET_TEMP;
      updateSetTempItem(prevTempItem, false);
      updateSetTempItem(selectedTempItem, true);}
    
    if (enc1.isLeft()) {
      tempInactivityTimer = millis();
      selectedTempItem--;
      if (selectedTempItem < SET_TEMP) selectedTempItem = TEMP_EXIT;
      updateSetTempItem(prevTempItem, false);
      updateSetTempItem(selectedTempItem, true);}
    }
    // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА В РЕЖИМЕ РЕДАКТИРОВАНИЯ ---
  if (tempMenuState == TEMP_EDITING) {

      // --- ВРАЩЕНИЕ ВПРАВО (УВЕЛИЧЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isRight()) {
          tempInactivityTimer = millis();
          if (targetTemp < 115) {
              targetTemp++;}
      }

      // --- ВРАЩЕНИЕ ВЛЕВО (УМЕНЬШЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isLeft()) {
          tempInactivityTimer = millis();
          if (targetTemp > 0) {
              targetTemp--;}
      }
  }
      // --- ПРОВЕРКА: ИЗМЕНИЛОСЬ ЛИ ЗНАЧЕНИЕ? ---
          tft->setTextSize(5);
          tft->setTextColor((tempMenuState == TEMP_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(112, 110);
      if (targetTemp < 100) tft->print(" ");
      if (targetTemp < 10) tft->print(" ");
          tft->print(targetTemp);
      if (targetTemp != lastTargetTemp) {
          // 1. Стираем старое значение черным цветом
          tft->setTextSize(5);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(112, 110);
      if (lastTargetTemp < 100) tft->print(" ");
      if (lastTargetTemp < 10) tft->print(" ");
          tft->print(lastTargetTemp);

          // 2. Рисуем новое значение желтым цветом
          tft->setTextColor((tempMenuState == TEMP_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(112, 110);
          if (targetTemp < 100) tft->print(" ");
          if (targetTemp < 10) tft->print(" ");
          tft->print(targetTemp);

          // Обновляем старое значение
          lastTargetTemp = targetTemp;
      }

         // Небольшая задержка для стабильности работы энкодера
          static unsigned long lastSetTempLoopTime = 0;
      if (millis() - lastSetTempLoopTime < 50) return;
          lastSetTempLoopTime = millis();
 
    }     
        // <<< Копируем от сюда
        // --- ЛОГИКА СТРАНИЦЫ УСТАНОВКИ ГИСТЕРЕЗИСА (SET_HYST_PAGE) ---
      if (currentPage == "SET_HYST_PAGE") {
  
        // ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА ПРИ ВХОДЕ НА СТРАНИЦУ
        // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - hystInactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          targetHyst = EEPROM.read(2);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
       }

          // --- ЗАГРУЖАЕМ СТРАНИЦУ SetHystpage ЕСЛИ ОНА НЕ ЗАГРУЖЕНА ---
       if (!isSetHystPageDrawn) {
          drawSetHystpage();
          isSetHystPageDrawn = true;
          targetHyst = EEPROM.read(2);
      }
          static int lastTargetHyst = -1;
          lastTargetHyst = targetHyst;
      
          // --- ЛОГИКА КЛИКА ПО КНОПКЕ ---
       if (enc1.isClick()) {
          hystInactivityTimer = millis();
          // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
       if (selectedHystItem == HYST_EXIT) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "SET_PAGE";
          //tempMenuState = TEMP_NAVIGATING;
          EEPROM.update(2, targetHyst); //Сохроняем в EEPROM если значения изменились
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
       }
    // --- ВХОД/ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
    if (selectedHystItem == SET_HYST) {
        // Если мы УЖЕ в режиме редактирования, значит это клик на "ВЫХОД ИЗ РЕЖИМА"
        if (hystMenuState == HYST_EDITING) {
            hystMenuState = HYST_NAVIGATING; // Выключаем режим редактирования
            // Цвет цифр изменится на белый автоматически благодаря нашей "умной" строчке
        } else {
            // Если мы НЕ в режиме редактирования, значит это клик на "ВХОД В РЕЖИМ"
            hystMenuState = HYST_EDITING; }
        }
      }
      // --- ЕСЛИ МЫ В РЕЖИМЕ НАВИГАЦИИ (МЕНЯЕМ ВЫДЕЛЕНИЕ) ---
      if (hystMenuState == HYST_NAVIGATING) {
    byte prevHystItem = selectedHystItem;
    if (enc1.isRight()) {
      hystInactivityTimer = millis();
      selectedHystItem++;
      if (selectedHystItem > HYST_EXIT) selectedHystItem = SET_HYST;
      updateSetHystItem(prevHystItem, false);
      updateSetHystItem(selectedHystItem, true);}
    
    if (enc1.isLeft()) {
      hystInactivityTimer = millis();
      selectedHystItem--;
      if (selectedHystItem < SET_HYST) selectedHystItem = HYST_EXIT;
      updateSetHystItem(prevHystItem, false);
      updateSetHystItem(selectedHystItem, true);}
    }
    // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА В РЕЖИМЕ РЕДАКТИРОВАНИЯ ---
  if (hystMenuState == HYST_EDITING) {

      // --- ВРАЩЕНИЕ ВПРАВО (УВЕЛИЧЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isRight()) {
          hystInactivityTimer = millis();
          if (targetHyst < 25) {
              targetHyst++;}
      }

      // --- ВРАЩЕНИЕ ВЛЕВО (УМЕНЬШЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isLeft()) {
          hystInactivityTimer = millis();
          if (targetHyst > 0) {
              targetHyst--;}
      }
  }
      // --- ПРОВЕРКА: ИЗМЕНИЛОСЬ ЛИ ЗНАЧЕНИЕ? ---
          tft->setTextSize(5);
          tft->setTextColor((hystMenuState == HYST_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(126, 110);
      if (targetHyst < 10) tft->print(" ");
          tft->print(targetHyst);
      if (targetHyst != lastTargetHyst) {
          // Стираем старое значение черным цветом
          tft->setTextSize(5);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(126, 110);
      if (lastTargetHyst < 10) tft->print(" ");
          tft->print(lastTargetHyst);

          // Рисуем новое значение желтым цветом
          tft->setTextColor((hystMenuState == HYST_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(126, 110);
          if (targetHyst < 10) tft->print(" ");
          tft->print(targetHyst);

          // Обновляем старое значение
          lastTargetHyst = targetHyst;
      }

          // Небольшая задержка для стабильности работы энкодера
          static unsigned long lastSetHystLoopTime = 0;
          if (millis() - lastSetHystLoopTime < 50) return;
          lastSetHystLoopTime = millis();
 
      } 

          // --- ЛОГИКА СТРАНИЦЫ УСТАНОВКИ ГИСТЕРЕЗИСА (SET_FROST_PAGE) ---
      if (currentPage == "SET_FROST_PAGE") {
          // ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА ПРИ ВХОДЕ НА СТРАНИЦУ
          // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - frostInactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          targetFrost = EEPROM.read(3);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
      }
  
          // --- ЗАГРУЖАЕМ СТРАНИЦУ SetFrostpage ЕСЛИ ОНА НЕ ЗАГРУЖЕНА ---
      if (!isSetFrostPageDrawn) {
          drawSetFrostpage();
          isSetFrostPageDrawn = true;
          targetFrost = EEPROM.read(3);
      }
          static int lastTargetFrost = -1;
          lastTargetFrost = targetFrost;
      
          // --- ЛОГИКА КЛИКА ПО КНОПКЕ ---
      if (enc1.isClick()) {
          frostInactivityTimer = millis();
    
          // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
      if (selectedFrostItem == FROST_EXIT) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "SET_PAGE";
          EEPROM.update(3, targetFrost); //Сохроняем в EEPROM если значения изменились
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
      return;
    }
    // --- ВХОД/ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
    if (selectedFrostItem == SET_FROST) {
        // Если мы УЖЕ в режиме редактирования, значит это клик на "ВЫХОД ИЗ РЕЖИМА"
        if (frostMenuState == FROST_EDITING) {
            frostMenuState = FROST_NAVIGATING; // Выключаем режим редактирования
            // Цвет цифр изменится на белый автоматически благодаря нашей "умной" строчке
        } else {
            // Если мы НЕ в режиме редактирования, значит это клик на "ВХОД В РЕЖИМ"
            frostMenuState = FROST_EDITING; }
        }
      }
      // --- ЕСЛИ МЫ В РЕЖИМЕ НАВИГАЦИИ (МЕНЯЕМ ВЫДЕЛЕНИЕ) ---
      if (frostMenuState == FROST_NAVIGATING) {
    byte prevFrostItem = selectedFrostItem;
    if (enc1.isRight()) {
      frostInactivityTimer = millis();
      selectedFrostItem++;
      if (selectedFrostItem > FROST_EXIT) selectedFrostItem = SET_FROST;
      updateSetFrostItem(prevFrostItem, false);
      updateSetFrostItem(selectedFrostItem, true);}
    
    if (enc1.isLeft()) {
      frostInactivityTimer = millis();
      selectedFrostItem--;
      if (selectedFrostItem < SET_FROST) selectedFrostItem = FROST_EXIT;
      updateSetFrostItem(prevFrostItem, false);
      updateSetFrostItem(selectedFrostItem, true);}
    }
    // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА В РЕЖИМЕ РЕДАКТИРОВАНИЯ ---
  if (frostMenuState == FROST_EDITING) {

      // --- ВРАЩЕНИЕ ВПРАВО (УВЕЛИЧЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isRight()) {
          frostInactivityTimer = millis();
          if (targetFrost < 25) {
              targetFrost++;}
      }

      // --- ВРАЩЕНИЕ ВЛЕВО (УМЕНЬШЕНИЕ ТЕМПЕРАТУРЫ) ---
      if (enc1.isLeft()) {
          frostInactivityTimer = millis();
          if (targetFrost > 0) {
              targetFrost--;}
      }
  }
      // --- ПРОВЕРКА: ИЗМЕНИЛОСЬ ЛИ ЗНАЧЕНИЕ? ---
          tft->setTextSize(5);
          tft->setTextColor((frostMenuState == FROST_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(126, 110);
      if (targetFrost < 10) tft->print(" ");
          tft->print(targetFrost);
      if (targetFrost != lastTargetFrost) {
          // Стираем старое значение черным цветом
          tft->setTextSize(5);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(126, 110);
      if (lastTargetFrost < 10) tft->print(" ");
          tft->print(lastTargetFrost);

          // Рисуем новое значение желтым цветом
          tft->setTextColor((frostMenuState == FROST_NAVIGATING) ? COLOR_WHITE : COLOR_YELLOW);
          tft->setCursor(126, 110);
          if (targetFrost < 10) tft->print(" ");
          tft->print(targetFrost);

          // Обновляем старое значение
          lastTargetFrost = targetFrost;
      }

          // Небольшая задержка для стабильности работы энкодера
          static unsigned long lastSetFrostLoopTime = 0;
          if (millis() - lastSetFrostLoopTime < 50) return;
          lastSetFrostLoopTime = millis();
 
      } 

          // --- ЛОГИКА СТРАНИЦЫ УСТАНОВКИ ТАЙМЕР 1 (SET_TIMER_1_PAGE) ---
      if (currentPage == "SET_TIMER_1_PAGE") {
          // ИНИЦИАЛИЗАЦИЯ ТАЙМЕРА ПРИ ВХОДЕ НА СТРАНИЦУ
          // --- ПРОВЕРКА БЕЗДЕЙСТВИЯ (10 сек) ---
      if (millis() - timerInactivityTimer > inactivityTime) {
          tft->fillScreen(COLOR_BACKGROUND);
          t1HourON1 = EEPROM.read(4);
          t2MinuteON1 = EEPROM.read(5);
          t3HourOFF1 = EEPROM.read(6);
          t4MinuteOFF1 = EEPROM.read(7);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;
      }
  
          // --- ЗАГРУЖАЕМ СТРАНИЦУ SetTimerpage ЕСЛИ ОНА НЕ ЗАГРУЖЕНА ---
      if (!isSetTimerPageDrawn) {
          drawSetTimerpage();
          t1HourON1 = EEPROM.read(4);
          t2MinuteON1 = EEPROM.read(5);
          t3HourOFF1 = EEPROM.read(6);
          t4MinuteOFF1 = EEPROM.read(7);
          tft->setTextSize(2);
          tft->setTextColor(COLOR_WHITE);
          tft->setCursor(200, 20);
          tft->print(" " );
          tft->print(activeTimerNumber);
          tft->print(" >");
          tft->setCursor(189, 90);
          if (t1HourON1 >= 0 && t1HourON1 <= 23) {
          tft->print(t1HourON1);
          } else {
          tft->print("--");}
          tft->setCursor(240, 90);
          if (t2MinuteON1 >= 0 && t2MinuteON1 <= 59) {
          tft->print(t2MinuteON1);
          } else  {
          tft->print("--");}
          /*tft->setCursor(195, 140);
          if (t3HourOFF1 >= 0 && t3HourOFF1 <= 23) {
          tft->print(t3HourOFF1);
          } else {
          tft->print("--");}
          tft->setCursor(232, 140);
          if (t4MinuteOFF1 >= 0 && t4MinuteOFF1 <= 59) {
          tft->print(t4MinuteOFF1);
          } else {
          tft->print("--");}*/
          isSetTimerPageDrawn = true;
      }
          // временные переменные для хранения установленного времени включения и выключения
          static byte lasteditHourON = -1;   // Временные часы включения
          static byte lasteditMinuteON = -1; // Временные минуты включения
          static byte lasteditHourOFF = -1;   // Временные часы выключения
          static byte lasteditMinuteOFF = -1; // Временные минуты выключения
          // приравниваем временные значения к установленным 
          /*lasteditHourON = t1HourON1;
          lasteditMinuteON = t2MinuteON1;
          lasteditHourOFF = t3HourOFF1;
          lasteditMinuteOFF = t4MinuteOFF1;*/

          // --- ЕСЛИ МЫ В РЕЖИМЕ НАВИГАЦИИ (МЕНЯЕМ ВЫДЕЛЕНИЕ) ---
      if (timerMenuState == TIMER_NAVIGATING) {
          byte prevItem = selectedTimerItem;
      if (enc1.isRight()) {
          timerInactivityTimer = millis();
          selectedTimerItem++;
      if (selectedTimerItem > TIMER_EXIT) selectedTimerItem = TIMER_H_ON;
          updateSetTimerItem(prevItem, false);
          updateSetTimerItem(selectedTimerItem, true);}
    
      if (enc1.isLeft()) {
          timerInactivityTimer = millis();
          selectedTimerItem--;
      if (selectedTimerItem < TIMER_H_ON) selectedTimerItem = TIMER_EXIT;
          updateSetTimerItem(prevItem, false);
          updateSetTimerItem(selectedTimerItem, true);}
    }
      
          // --- ЛОГИКА КЛИКА ПО КНОПКЕ ---
      if (enc1.isClick()) {
          timerInactivityTimer = millis();
    
          // --- ВЫХОД ПО ПУНКТУ "EXIT" ---
      if (selectedTimerItem == TIMER_EXIT) {
          tft->fillScreen(COLOR_BACKGROUND);
          currentPage = "SET_PAGE";
          isStaticDrawn = false;
          isSetPageDrawn = false;
          isSetTimePageDrawn = false;
          isSetTempPageDrawn = false;
          isSetFrostPageDrawn = false;
          isSetHystPageDrawn = false;
          isSetTimerPageDrawn = false;
          mainInactivityTimer = millis(); // Сбрасываем таймер MAIN_PAGE
          tempInactivityTimer = millis(); // Сбрасываем таймер для новой страницы
          setInactivityTimer = millis(); // Сбрасываем таймер SET_PAGE при возврате
          timeInactivityTimer = millis();
          hystInactivityTimer = millis();
          frostInactivityTimer = millis();
          timerInactivityTimer = millis();
          return;}

          // --- ВХОД/ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
          // Проверяем клик на часы или минуты (ВКЛ или ВЫКЛ)
      if (selectedTimerItem == TIMER_H_ON || selectedTimerItem == TIMER_M_ON ||
          selectedTimerItem == TIMER_H_OFF || selectedTimerItem == TIMER_M_OFF) {

          // --- ВХОД В РЕЖИМ РЕДАКТИРОВАНИЯ ---
          // Если мы сейчас в режиме навигации, то входим в редактирование
      if (timerMenuState == TIMER_NAVIGATING) {
        // Входим в режим редактирования того пункта, на который кликнули
        if (selectedTimerItem == TIMER_H_ON) {
            timerMenuState = TIMER_ON_EDITING_H;
            t1HourON1=0;
        } else if (selectedTimerItem == TIMER_M_ON) {
            timerMenuState = TIMER_ON_EDITING_M;
            t2MinuteON1=0;
        } else if (selectedTimerItem == TIMER_H_OFF) {
            timerMenuState = TIMER_OFF_EDITING_H;
        } else if (selectedTimerItem == TIMER_M_OFF) {
            timerMenuState = TIMER_OFF_EDITING_M;
        }
    }
            // --- ВЫХОД ИЗ РЕЖИМА РЕДАКТИРОВАНИЯ ---
            // Если мы УЖЕ находимся в режиме редактирования этого же пункта, выходим
          else if ((selectedTimerItem == TIMER_H_ON && timerMenuState == TIMER_ON_EDITING_H) ||
             (selectedTimerItem == TIMER_M_ON && timerMenuState == TIMER_ON_EDITING_M) ||
             (selectedTimerItem == TIMER_H_OFF && timerMenuState == TIMER_OFF_EDITING_H) ||
             (selectedTimerItem == TIMER_M_OFF && timerMenuState == TIMER_OFF_EDITING_M)) {
            // Выходим из режима редактирования
            timerMenuState = TIMER_NAVIGATING; }
          }
         }
     
            // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА В РЕЖИМЕ РЕДАКТИРОВАНИЯ ЧАСОВ ВКЛЮЧЕНИЯ ---
        else if (timerMenuState == TIMER_ON_EDITING_H) {
            // Проверяем вращение ВПРАВО
            if (enc1.isRight()) {
            timerInactivityTimer = millis(); // Сбрасываем таймер
            // Увеличиваем значение, но не более 23
            if (t1HourON1 < 23) {
            t1HourON1++; }
          }
            // Проверяем вращение ВЛЕВО
            if (enc1.isLeft()) {
            timerInactivityTimer = millis(); // Сбрасываем таймер
            // Уменьшаем значение, но не менее 0
            if (t1HourON1 > 0) {
            t1HourON1--;}
          } 
        }
            // --- ЛОГИКА ВРАЩЕНИЯ ЭНКОДЕРА В РЕЖИМЕ РЕДАКТИРОВАНИЯ МИНУТ ВКЛЮЧЕНИЯ ---
        else if (timerMenuState == TIMER_ON_EDITING_M) {
            // Проверяем вращение ВПРАВО
            if (enc1.isRight()) {
            timerInactivityTimer = millis(); // Сбрасываем таймер
            // Увеличиваем значение, но не более 59
            if (t2MinuteON1 < 59) {
            t2MinuteON1++;}
           }
            // Проверяем вращение ВЛЕВО
            if (enc1.isLeft()) {
            timerInactivityTimer = millis(); // Сбрасываем таймер
            // Уменьшаем значение, но не менее 0
            if (t2MinuteON1 > 0) {
            t2MinuteON1--;}
           } 
         }
          //--- Логика отрисовки установки часов таймера включения
         tft->setTextSize(2);
          tft->setTextColor( (timerMenuState == TIMER_ON_EDITING_H) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(189, 90);
          if (t1HourON1 >= 0 && t1HourON1 <= 23) {
          if (t1HourON1 < 10) tft->print("0");
          tft->print(t1HourON1);
          } else {
          tft->setCursor(189, 90);
          tft->print("--");}
          //tft->print(t1HourON1);
          if (t1HourON1 != lasteditHourON) {
          //  СТИРАЕМ СТАРЫЕ значения цветом фона
          tft->setTextSize(2);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(189, 90);
          tft->print("--");
          tft->setCursor(189, 90);
          if (lasteditHourON < 10) tft->print("0");
          tft->print(lasteditHourON);
          
          // РИСУЕМ НОВЫЕ ЧАСЫ. Цвет зависит от режима.
          tft->setTextSize(2);
          tft->setTextColor( (timerMenuState == TIMER_ON_EDITING_H) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(189, 90);
          if (t1HourON1 >= 0 && t1HourON1 <= 23) {
          if (t1HourON1 < 10) tft->print("0");
          tft->print(t1HourON1);
          } else {
          tft->setCursor(189, 90);
          tft->print("--");}
          lasteditHourON = t1HourON1; // Обновляем для следующего сравнения
          }
           // --- ЛОГИКА  ДЛЯ установки МИНУТ таймера включения ---
          tft->setTextSize(2);
          tft->setTextColor( (timerMenuState == TIMER_ON_EDITING_M) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(240, 90);         
          if (t2MinuteON1 >= 0 && t2MinuteON1 <= 59) {
          if (t2MinuteON1 < 10) tft->print("0");
          tft->print(t2MinuteON1);
          } else {
          tft->setCursor(240, 90);
          tft->print("--");}
          
      if (t2MinuteON1 != lasteditMinuteON) {
          // СТИРАЕМ СТАРЫЕ МИНУТЫ
          tft->setTextSize(2);
          tft->setTextColor(COLOR_BACKGROUND);
          tft->setCursor(240, 90);
          tft->print("--");
          tft->setCursor(240, 90);
          if (lasteditMinuteON < 10) tft->print("0");
          tft->print(lasteditMinuteON);
          
          // РИСУЕМ НОВЫЕ МИНУТЫ. Цвет зависит от режима.
          tft->setTextSize(2);
          tft->setTextColor( (timerMenuState == TIMER_ON_EDITING_M) ? COLOR_YELLOW : COLOR_WHITE );
          tft->setCursor(240, 90);         
          if (t2MinuteON1 >= 0 && t2MinuteON1 <= 59) {
          if (t2MinuteON1 < 10) tft->print("0");
          tft->print(t2MinuteON1);
          } else {
          tft->setCursor(240, 90);
          tft->print("--");}
          lasteditMinuteON = t2MinuteON1;
         }


          
          static unsigned long lastSetTimerLoopTime = 0;
          if (millis() - lastSetTimerLoopTime < 50) return;
          lastSetTimerLoopTime = millis();
            
            
    // <<< вставляем сюда
  }  // Скобка закрытия  if (currentPage == "SET_TIMER_1_PAGE")    
} //<<< вставляем до этой скобки эта скобка закрытие LOOP


// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА МЕНЮ В SET_PAGE ---
void updateSetPageItem(byte itemIndex, bool isSelected) {

      // Печатаем часы реального времени
      DateTime now = rtc.now(); // Запускаем RTC
      int currentHour = now.hour();
      int currentMinute = now.minute();
      tft->setTextColor(COLOR_WHITE);     // устанавливаем цвет текста
      tft->setCursor(175, 51);
      if (currentHour < 10) {             // проверяем если время <10
      tft->print("0"); }                  // печатаем 0 перед часами
      tft->print(currentHour);            // печатаем часы с RTC
      if (currentHour != lastHour) {      // проверяем если новые часы не равны старым
      tft->setTextColor(COLOR_BACKGROUND);// устанавливаем черный цвет
      tft->setCursor(175, 51);
      if (lastHour < 10) {                // проверяем если время <10
      tft->print("0");}                   // печатаем 0 перед часами
      tft->print(lastHour);               // печатаем старые часы черным цветом
      tft->setTextColor(COLOR_WHITE);     // устанавливаем белый цвет
      tft->setCursor(175, 51);
      if (currentHour < 10) {             // проверяем если время <10
      tft->print("0");}                   // печатаем 0 перед минутами
      tft->print(currentHour);            // печатаем обновленное время белым цветом
      lastHour = currentHour;}            // приравниваем новыепоказания часов к старым
      tft->setTextColor(COLOR_WHITE);     // ставим белый цвет
      tft->setCursor(192, 51);
      tft->print(":");                    // печатаем разделение между часами и минутами
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(202, 51);
      if (currentMinute < 10) {           // проверяем если минуты <10
      tft->print("0");}                   // печатаем 0 перед минутами
      tft->print(currentMinute);          // печатаем минуты с RTC
      if (currentMinute != lastMinute) {  // проверяем если новые минуты не равны старым
      tft->setTextColor(COLOR_BACKGROUND);// устанавливаем черный цвет  
      tft->setCursor(202, 51); 
      if (lastMinute < 10) {              // проверяем если минуты <10
      tft->print("0");}                   // печатаем 0 перед минутами
      tft->print(lastMinute);             // печатаем старые минуты черным цветом
      tft->setTextColor(COLOR_WHITE);     // ставим белый цвет
      tft->setCursor(202, 51);   
      if (currentMinute < 10) {           // проверяем если минуты <10
      tft->print("0");}                   // печатаем 0 перед минутами
      tft->print(currentMinute);          // печатаем минуты с RTC
      lastMinute = currentMinute;}        // приравниваем новыепоказания минут к старым
      
      // Печатаем значение установленной температуры из EEPROM 
      tft->setTextColor(COLOR_WHITE);     // ставим белый цвет
      tft->setCursor(185, 71);
      if (targetTemp < 100) tft->print(" ");
      if (targetTemp < 10) tft->print(" ");
      tft->print(targetTemp);tft->print((char)248);tft->print("C");

      // Печатаем значение установленного гистерезиса из EEPROM 
      tft->setTextColor(COLOR_WHITE);     // ставим белый цвет
      tft->setCursor(190, 91);
      if (targetHyst < 10) tft->print(" ");
      tft->print(targetHyst);tft->print((char)248);tft->print("C");

      // Печатаем значение установленного разморозки из EEPROM 
      tft->setTextColor(COLOR_WHITE);     // ставим белый цвет
      tft->setCursor(192, 111);
      if (targetHyst < 10) tft->print(" ");
      tft->print(targetFrost);tft->print((char)248);tft->print("C");

      
      
      
  // --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА МЕНЮ В SET_PAGE (ТОЛЬКО ВЫДЕЛЕНИЕ) ---
  const int itemHeight = 20;
  const int startY = 30;
  int x = 40; // Начальная координата X для всех пунктов
  int y = startY + itemIndex * itemHeight; // Координата Y для текущего пункта

  // --- СТИРАЕМ СТАРУЮ РАМКУ ---
  // Чтобы стереть рамку, рисуем ее цветом фона.
  // Координаты подобраны так, чтобы рамка была вокруг текста с небольшим отступом.
  tft->drawRect(x - 6, y - 6, 126, itemHeight, COLOR_BACKGROUND);

  // --- 2. РИСУЕМ ТЕКСТ ПУНКТА МЕНЮ (ВСЕГДА БЕЛЫМ) ---
  tft->setTextColor(COLOR_WHITE);
  tft->setCursor(x, y);

  // Выводим надпись в зависимости от номера пункта
  switch (itemIndex) {
    case MENU_ITEM_SET_TIME:
      tft->print("Set Time: ......... ");
      break;
    case MENU_ITEM_SET_TEMP:
      tft->print("Set Temperature: .. ");
      break;
    case MENU_ITEM_SET_HYST:
      tft->print("Set Hysteresis: ... ");
      break;
    case MENU_ITEM_SET_FROST:
      tft->print("Set Frosting: ..... ");
      break;
    case MENU_ITEM_SET_TIMER_1:
      tft->print("Set Timer 1:");
      break;
    case MENU_ITEM_SET_TIMER_2:
      tft->print("Set Timer 2:");
      break;
    case MENU_ITEM_SET_TIMER_3:
      tft->print("Set Timer 3:");
      break;
    case MENU_ITEM_SET_TIMER_4:
      tft->print("Set Timer 4:");
      break;
    case MENU_ITEM_EXIT:
      tft->print("EXIT");
      break;
  }

   // --- 3. РИСУЕМ НОВУЮ РАМКУ ЕСЛИ ПУНКТ ВЫБРАН ---
   // Рисуем рамку ПОВЕРХ текста.
   if (isSelected) {
       // Рисуем НЕЗАКРАШЕННЫЙ (только контур) белый прямоугольник.
       // Координаты x-2, y-2 создают отступ в 2 пикселя вокруг текста.
       // Ширина и height подобраны для стандартного пункта меню.
       tft->drawRect(x - 6, y - 6, 126, itemHeight, COLOR_YELLOW); 
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
      tft->setCursor(54, 1); 
      // Выводим надпись <Sauna Burovichok>
      tft->print("<Sauna Burovichok>");
      // Устанавливаем курсор для вывода надписи время в координаты x=10, y=20
      tft->setCursor(10, 25); 
      // Выводим надпись <TIME>
      tft->print("<TIME>");
      
      // --- Отрисовка шкалы влажности
      // --- ОТРИСОВКА СЕРОЙ ОКАНТОВКИ ---
      // Углы для влажности: от 180° до 270°
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
      
      // Сбрасываем "память" о старых значениях температуры и влажности перед отрисовкой динамики
      lastTemperature = -1; 
      lastHumidity = -1;
     }

// Рисуем динамические части интерфейса, стрелки digital indicator,clock, меню выбора работы   
void drawDinamointerface() {
      
      targetTemp = EEPROM.read(1);
      DateTime now = rtc.now(); // Запускаем RTC
      int currentHour = now.hour();
      int currentMinute = now.minute();
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(15, 50);
      if (currentHour < 10) {
      tft->print("0");
    }
      tft->print(currentHour);
  
      // --- ВЫВОД ВРЕМЕНИ  ---
     // --- ЛОГИКА ДЛЯ ЧАСОВ (ПЕРВАЯ) ---
     // Проверяем: ЧАСЫ ИЗМЕНИЛИСЬ?
      if (currentHour != lastHour) {
     //  СТИРАЕМ СТАРЫЕ ЧАСЫ
      tft->setTextColor(COLOR_BACKGROUND);
      tft->setCursor(15, 50); // Координаты часов
      if (lastHour < 10) {
      tft->print("0"); // Стираем ведущий ноль, если он был
    }
      tft->print(lastHour);

     //  ПЕЧАТАЕМ НОВЫЕ ЧАСЫ
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(15, 50);
      if (currentHour < 10) {
      tft->print("0");
    }
      tft->print(currentHour);
    
    // Сохраняем новое значение часов
      lastHour = currentHour;
  }
 
    // --- ВЫВОД ДВОЕТОЧИЯ ---
    // Двоеточие просто рисуем всегда на своем месте
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(38, 50); // Координаты двоеточия (между часами и минутами)
    tft->print(":");

    // --- ЛОГИКА ДЛЯ МИНУТ (ВТОРАЯ) ---
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(50, 50);
    if (currentMinute < 10) {
        tft->print("0");
    }
    tft->print(currentMinute);
         
    // Проверяем: МИНУТЫ ИЗМЕНИЛИСЬ?
    if (currentMinute != lastMinute) {
    // 1. СТИРАЕМ СТАРЫЕ МИНУТЫ
    tft->setTextColor(COLOR_BACKGROUND);
    tft->setCursor(50, 50); // Координаты минут
    if (lastMinute < 10) {
        tft->print("0"); // Стираем ведущий ноль, если он был
    }
    tft->print(lastMinute);

    //  ПЕЧАТАЕМ НОВЫЕ МИНУТЫ
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(50, 50);
    if (currentMinute < 10) {
        tft->print("0");
    }
    tft->print(currentMinute);
    
    // Сохраняем новое значение минут
    lastMinute = currentMinute;
  }
        //Цифровые и аналоговые индикаторы
        // --- ВЛАЖНОСТЬ ---
       int currentHumidity = shtSensor.readHumidity(); // устанавливаем отображение влажности целым числом
       tft->setTextColor(COLOR_WHITE);
       tft->setCursor(40, 173);
       tft->print(currentHumidity);
       
       
       // СЧИТЫВАЕМ СВЕЖИЕ ДАННЫЕ ТОЛЬКО ЕСЛИ НУЖНО ОБНОВЛЕНИЕ! 
       
         // Координаты цифровых индикаторов влажности
      if (currentHumidity != lastHumidity) {
        //  СТИРАЕМ СТАРЫЙ ЦИФРОВОЙ указатель влажности
      tft->setTextColor(COLOR_BACKGROUND);
      tft->setCursor(40, 173); // Координаты цифрового указателя влажности
      tft->print(lastHumidity);
         //  ПЕЧАТАЕМ НОВЫЙ ЦИФРОВОЙ указатель влажности
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(40, 173);
      tft->print(currentHumidity);

      // РИСУЕМ НОВУЮ СТРЕЛКУ ВЛАЖНОСТИ
      // Линия из центра (HUM_CENTER_X, HUM_CENTER_Y)
      // в точку (HUM_CENTER_X - длина, HUM_CENTER_Y) 
      // СТИРАЕМ СТАРУЮ СТРЕЛКУ ВЛАЖНОСТИ
      // Используем lastHumidity для расчета угла
      float oldAngleHum = (lastHumidity * 0.9) + 270;
      int16_t oldXEndHum = HUM_CENTER_X + (int16_t)(sin(oldAngleHum * (PI / 180.0)) * HUM_NEEDLE_LENGTH);
      int16_t oldYEndHum = HUM_CENTER_Y - (int16_t)(cos(oldAngleHum * (PI / 180.0)) * HUM_NEEDLE_LENGTH);
      //tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, oldXEndHum, oldYEndHum, COLOR_YELLOW);
      tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, oldXEndHum, oldYEndHum, COLOR_BACKGROUND);

      // РИСУЕМ НОВУЮ СТРЕЛКУ ВЛАЖНОСТИ
      // Используем currentHumidity для нового угла
      float newAngleHum = (currentHumidity * 0.9) + 270;
      int16_t newXEndHum = HUM_CENTER_X + (int16_t)(sin(newAngleHum * (PI / 180.0)) * HUM_NEEDLE_LENGTH);
      int16_t newYEndHum = HUM_CENTER_Y - (int16_t)(cos(newAngleHum * (PI / 180.0)) * HUM_NEEDLE_LENGTH);
      tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, newXEndHum, newYEndHum, COLOR_YELLOW);
      //tft->drawLine(HUM_CENTER_X, HUM_CENTER_Y, newXEndHum, newYEndHum, COLOR_BACKGROUND);
    
        // Сохраняем новое значение влажности 
        lastHumidity = currentHumidity;
      } 
   
      
      // --- ТЕМПЕРАТУРА ---
      int currentTemperature = shtSensor.readTemperature(); // Устанавливаем отображение температурыцелым числом
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(185, 173);
      tft->print(currentTemperature);
      
      // СЧИТЫВАЕМ СВЕЖИЕ ДАННЫЕ ТОЛЬКО ЕСЛИ НУЖНО ОБНОВЛЕНИЕ!
      
      //координаты цифровых индикаторов температуры
      if (currentTemperature != lastTemperature) {
        
        //  СТИРАЕМ СТАРЫЙ ЦИФРОВОЙ указатель температуры
      tft->setTextColor(COLOR_BACKGROUND);
      tft->setCursor(185, 173); // Координаты цифрового указателя температуры
      tft->print(lastTemperature);
         //  ПЕЧАТАЕМ НОВЫЙ ЦИФРОВОЙ указатель температуры
      tft->setTextColor(COLOR_WHITE);
      tft->setCursor(185, 173);
      tft->print(currentTemperature);

      // РИСУЕМ СТРЕЛКУ ТЕМПЕРАТУРЫ
      // Линия из центра (TEMP_CENTER_X, TEMP_CENTER_Y)
      // в точку (TEMP_CENTER_X - длина, TEMP_CENTER_Y)
      // Устанавливаем "прошлые" значения равными текущим
      // СТИРАЕМ СТАРУЮ СТРЕЛКУ
      // Используем lastTemperature для расчета угла
      float oldAngle = (lastTemperature * 1.5) + 270;
      int16_t oldXEnd = TEMP_CENTER_X + (int16_t)(sin(oldAngle * (PI / 180.0)) * TEMP_NEEDLE_LENGTH);
      int16_t oldYEnd = TEMP_CENTER_Y - (int16_t)(cos(oldAngle * (PI / 180.0)) * TEMP_NEEDLE_LENGTH);
      //tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, oldXEnd, oldYEnd, COLOR_YELLOW);
      tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, oldXEnd, oldYEnd, COLOR_BACKGROUND);

      // РИСУЕМ НОВУЮ СТРЕЛКУ
      // Используем currentTemperature для нового угла
      float newAngle = (currentTemperature * 1.5) + 270;
      int16_t newXEnd = TEMP_CENTER_X + (int16_t)(sin(newAngle * (PI / 180.0)) * TEMP_NEEDLE_LENGTH);
      int16_t newYEnd = TEMP_CENTER_Y - (int16_t)(cos(newAngle * (PI / 180.0)) * TEMP_NEEDLE_LENGTH);
      tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, newXEnd, newYEnd, COLOR_YELLOW);
      //tft->drawLine(TEMP_CENTER_X, TEMP_CENTER_Y, newXEnd, newYEnd, COLOR_BACKGROUND);
       
      // Сохраняем новое значение температуры
      lastTemperature = currentTemperature;
    }
    
   
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
      tft->setCursor(138, 210); 
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

  // Заголовок страницы
  tft->setCursor(120, 10);
  tft->setTextColor(COLOR_WHITE);
  tft->print("SETTING");

  tft->setTextSize(1);

  // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = MENU_ITEM_SET_TIME; i <= MENU_ITEM_EXIT; i++) {
    updateSetPageItem(i, (i == selectedMenuItem));
  }
}

// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА В МЕНЮ SET_TIME ---
void updateSetTimeItem(byte itemIndex, bool isSelected) {
 
  // --- РИСУЕМ ЭЛЕМЕНТ В ЗАВИСИМОСТИ ОТ itemIndex ---
  switch (itemIndex) {
    case TIME_HOURS:
      tft->drawRect(95, 100, 58, 50, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

    case TIME_MINUTES:
      tft->drawRect(171, 100, 58, 50, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

    case TIME_EXIT:
      // Рисуем кнопку текущим цветом
      tft->drawRect(112, 190, 85, 40, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;
  }
}


// --- ОТРИСОВКА СТРАНИЦЫ УСТАНОВКИ ВРЕМЕНИ ---
void drawSetTimepage() {
    // Очищаем экран перед отрисовкой
    tft->fillScreen(COLOR_BACKGROUND); 

    // --- ЗАГОЛОВОК  ---
    tft->setTextSize(3);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(90, 30); // Координаты по центру сверху
    tft->print("SET TIME");
    // --- РИСУЕМ ДВОЕТОЧИЕ ---
    tft->setTextSize(4);
    tft->setCursor(152, 110);
    tft->setTextColor(COLOR_WHITE);
    tft->print(":");
    tft->setTextSize(3);
    tft->setCursor(122, 200);
    tft->print("EXIT");
    
    // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = TIME_HOURS; i <= TIME_EXIT; i++) {
    updateSetTimeItem(i, (i == selectedTimeItem));
  }
}

// --- ОТРИСОВКА СТРАНИЦЫ УСТАНОВКИ ТЕМПЕРАТУРЫ ---
void drawSetTemppage() {
    // Очищаем экран перед отрисовкой
    tft->fillScreen(COLOR_BACKGROUND); 

    // ---  ЗАГОЛОВОК  ---
    tft->setTextSize(3);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(30, 30); // Координаты по центру сверху
    tft->print("SET TEMPERATURE");
    
    tft->setTextSize(3);
    tft->setCursor(122, 200);
    tft->print("EXIT");
    
    // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = SET_TEMP; i <= TEMP_EXIT; i++) {
    updateSetTempItem(i, (i == selectedTempItem));
  }
}

// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА В МЕНЮ SET_TEMP ---
  void updateSetTempItem(byte itemIndex, bool isSelected) {
 
  // --- РИСУЕМ ЭЛЕМЕНТ В ЗАВИСИМОСТИ ОТ itemIndex ---
  switch (itemIndex) {
    case SET_TEMP:
      tft->drawRect(103, 103, 105, 48, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

    case TEMP_EXIT:
      // Рисуем кнопку текущим цветом
      tft->drawRect(112, 190, 85, 40, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;
  }
}

// --- ОТРИСОВКА СТРАНИЦЫ УСТАНОВКИ ГИСТЕРЕЗИСА ---
void drawSetHystpage() {
    // Очищаем экран перед отрисовкой
    tft->fillScreen(COLOR_BACKGROUND); 

    // ---  ЗАГОЛОВОК  ---
    tft->setTextSize(3);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(35, 30); // Координаты по центру сверху
    tft->print("SET HYSTERESIS"); 
    
    tft->setTextSize(3);
    tft->setCursor(122, 200);
    tft->print("EXIT");
    
    // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = SET_HYST; i <= HYST_EXIT; i++) {
    updateSetHystItem(i, (i == selectedHystItem));
  }
}

// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА В МЕНЮ SET_HYST ---
   void updateSetHystItem(byte itemIndex, bool isSelected) {
 
  // --- РИСУЕМ ЭЛЕМЕНТ В ЗАВИСИМОСТИ ОТ itemIndex ---
  switch (itemIndex) {
    case SET_HYST:
      tft->drawRect(103, 103, 105, 48, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

    case HYST_EXIT:
      // Рисуем кнопку текущим цветом
      tft->drawRect(112, 190, 85, 40, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;
  }
}

// --- ОТРИСОВКА СТРАНИЦЫ УСТАНОВКИ ПОДДЕРЖАНИЯ ТЕМПЕРАТУРЫ ---
void drawSetFrostpage() {
    // Очищаем экран перед отрисовкой
    tft->fillScreen(COLOR_BACKGROUND); 

    // ---  ЗАГОЛОВОК  ---
    tft->setTextSize(3);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(35, 30); // Координаты по центру сверху
    tft->print("SET DEFROSTING"); 
    
    tft->setTextSize(3);
    tft->setCursor(122, 200);
    tft->print("EXIT");
    
    // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = SET_FROST; i <= FROST_EXIT; i++) {
    updateSetFrostItem(i, (i == selectedFrostItem));
  }
}

// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА В МЕНЮ SET_FROST ---
   void updateSetFrostItem(byte itemIndex, bool isSelected) {
 
  // --- РИСУЕМ ЭЛЕМЕНТ В ЗАВИСИМОСТИ ОТ itemIndex ---
  switch (itemIndex) {
    case SET_FROST:
      tft->drawRect(103, 103, 105, 48, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

    case FROST_EXIT:
      // Рисуем кнопку текущим цветом
      tft->drawRect(112, 190, 85, 40, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;
  }
}

// ----Страница для таймеров----
// --- ОТРИСОВКА СТРАНИЦЫ УСТАНОВКИ ТАЙМЕРОВ ---
void drawSetTimerpage() {
    // Очищаем экран перед отрисовкой
    tft->fillScreen(COLOR_BACKGROUND); 

    // ---  ЗАГОЛОВОК  ---
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(70, 20); // Координаты по центру сверху
    tft->print("< SET TIMER");
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(220, 90); 
    tft->print(":");
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(220, 140); 
    tft->print(":");
    
    // --- НАДПИСЬ ДЛЯ ТАЙМЕР ON ---- 
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(50, 90); 
    tft->print("TIMER ON");

    // --- НАДПИСЬ ДЛЯ ТАЙМЕР OFF ---- 
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(50, 140); 
    tft->print("TIMER OFF");

    // --- НАДПИСЬ SAVE ---
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(30, 203);
    tft->print("SAVE");

    // --- НАДПИСЬ CLEAR ---
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(128, 203);
    tft->print("CLEAR");

    // --- НАДПИСЬ EXIT ---
    tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(238, 203);
    tft->print("EXIT");
    
    // Отрисовываем все пункты меню, выделяя текущий
  for (byte i = TIMER_H_ON; i <= TIMER_EXIT; i++) {
    updateSetTimerItem(i, (i == selectedTimerItem));
  }
}

// --- ФУНКЦИЯ ДЛЯ ОБНОВЛЕНИЯ ПУНКТА В МЕНЮ SET_TIMER ---
   void updateSetTimerItem(byte itemIndex, bool isSelected) {
    /*tft->setTextSize(2);
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(200, 20);
    tft->print(" 1 >");*/
 
  // --- РИСУЕМ ЭЛЕМЕНТ В ЗАВИСИМОСТИ ОТ itemIndex ---
  switch (itemIndex) {
    case TIMER_H_ON:
      tft->drawRect(185, 86, 32, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_M_ON:
      tft->drawRect(236, 86, 32, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_H_OFF:
      tft->drawRect(185, 136, 32, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_M_OFF:
      tft->drawRect(236, 136, 32, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_SAVE:
      tft->drawRect(25, 200, 55, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_CLEAR:
      tft->drawRect(124, 200, 66, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;

      case TIMER_EXIT:
      // Рисуем кнопку текущим цветом
      tft->drawRect(233, 200, 55, 22, (isSelected ? COLOR_YELLOW : COLOR_BACKGROUND));
      break;
  }
}