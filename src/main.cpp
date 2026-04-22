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
   
 // Прототипы функций для отрисовки
void drawBackground();
void drawDinamointerface();
void drawSetpage();
void drawTimerPage();

// Загружаем главную страницу с отрисовкой надписей и шкал приборов
void drawBackground(); {
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
    tft->setCursor(286, 152);
    tft->print(120);  
     
      // --- ПОДПИСЬ ДЛЯ ТЕМПЕРАТУРЫ ---
  // Давай временно поставим подпись для температуры рядом с её шкалой
  // Координаты (205, 190) - под центром шкалы температуры
     tft->setTextColor(COLOR_WHITE);
     tft->setTextSize(2); // Размер 2 для подписей
     tft->setCursor(221, 173);// координаты установки х=235, у=173
     tft->print((char)248);   // символ градусов!
     tft->setCursor(233, 173);// координаты установки х=230, у=173
     tft->print("C");
     
}
   
  void drawDinamointerface(); {

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
tft->setCursor(40, 220); 
tft->print("ON");
 
//Координаты надписи AUTO
tft->setTextColor(COLOR_WHITE);
tft->setCursor(140, 220); 
tft->print("AUTO");

//Координаты надписи OFF
tft->setTextColor(COLOR_WHITE);
tft->setCursor(240, 220); 
tft->print("OFF");
  } 
delay(5000);

  // Рисуем страницу настроек
 void drawSetpage(); {
  tft->fillScreen(COLOR_BACKGROUND);
  tft->setTextColor(COLOR_WHITE); 
  tft->setTextSize(2);

  // --- ЗАГОЛОВОК СТРАНИЦЫ ---
  // Координаты (80, 10) ставят текст примерно по центру сверху
  tft->setCursor(100, 10); 
  tft->print("SETTING");

  // --- ПУНКТЫ МЕНю ---
  // Установка значения времени
  // Координаты (20, Y) создают отступ слева
  tft->setTextSize(1)
  tft->setCursor(20, 50); 
  tft->print("Set Time:"); // Это тоже динамическая надпись должна будет менять цвет с белой на жёлтую при наведении курсора

// Время (00:00)
  tft->setCursor(180, 50);
  tft->print("00"); // Устанавливаем часы вращением энкодера, после установки по нажатию кнопки энкодера сохроняем в RTC
  tft->print(":");
  tft->print("00"); // Устанавливаем минуты вращением энкодера, после установки по нажатию кнопки энкодера сохроняем в RTC



}
  
 delay(5000);

void drawSetpage(); {
  tft->fillScreen(COLOR_BACKGROUND);
  tft->setTextColor(COLOR_WHITE); 
  tft->setTextSize(1);

  // --- ЗАГОЛОВОК СТРАНИЦЫ ---
  // Координаты (80, 10) ставят текст примерно по центру сверху
  tft->setCursor(80, 10); 
  tft->print("SETTING");

  // Устанавливаем температуру
  tft->setCursor(20, 40);
  tft->print("Set Temperature:");
//Устанавливаем температуру вращением энкодера, по нажатию кнопки сохроняем в EPROM 
  tft->print("000");

// Устанавливаем гистерезис
  tft->setCursor(20, 80);
  tft->print("Set Hysteresis:");

// Устанавливаем температуру вращением энкодера по нажатию на кнопку заносим значение в EPROM 
  // Гистерезис (00)
  tft->print("00");

// Устанавливаем режим работы поддержания температуры в режиме при отключению по таймеру
  tft->setCursor(20, 120);
// Устанавливаем температуру
  tft->print("Set Frosting:");
   вращением энкодера по нажатию на кнопку заносим значение в EPROM 
  // поддержание температуры максимальное значение установленное программой должно быть не более 50° (00)
  tft->print("00"); //максимально разрешённое значение должно быть 50°

// Пункт меню таймеры
  tft->setCursor(20, 160);
// Устанавливаем температуру
  tft->print("Set Timer:"); // при нажатии на кнопку энкодера переходим в меню Set_timer

} 

delay(5000);

  void drawTimerPage() ;{
     tft->fillScreen(COLOR_BACKGROUND);
     tft->setTextColor(COLOR_WHITE); 
     tft->setTextSize(1);

     // --- ЗАГОЛОВОК СТРАНИЦЫ ---
     tft->setCursor(70, 10); 
     tft->print("TIMER SETTINGS");


     // --- ТАЙМЕР 1 ---
     
     tft->setCursor(20, 20);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer 1");

     // Меню таймера
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->setCursor(20, 40);
     tft->print("Timer ON:");
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 

tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

// Отключение по таймеру
     tft->setCursor(40, 60);
     tft->print("Timer OFF:");
     tft->setCursor(20, 40);
     tft->setTextColor(COLOR_WHITE); //при установке часов  меняем цвет на жёлтый
     tft->print("00"); // Установка часов включения после подтверждения заносим в EPROM 

tft->setTextColor(COLOR_WHITE);// Цвет для ":"
     tft->print(":");
tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("00"); // установка минут для включения после подтверждения заносим в EPROM 

// запись в EPROM/ удаление из EPROM 
     tft->setCursor(40, 80);
     tft->setTextColor(COLOR_WHITE); //при наведении на пункт меню надпись меняем на жёлтую 
     tft->print("Timer Memory/Clear");


     // --- ТАЙМЕР 2 ---
     tft->setCursor(160, 50);
     tft->print("Timer 2");
  
     tft->setCursor(180, 80);
     tft->print("Timer ON:");
  
     tft->setCursor(180, 110);
     tft->print("Timer OFF:");
  
     tft->setCursor(180, 140);
     tft->print("Timer Memory/Clear");


      // --- ТАЙМЕРЫ З И 4 (ниже по вертикали) ---
      // Таймеры З и 4 мы размещаем под первыми двумя

      // --- ТАЙМЕР З ---
      tft->setCursor(20, 170);
      tft->print("Timer З");

      tft->setCursor(40, 200);
      tft->print("Timer ON:");

      // --- ТАЙМЕР 4 ---
      tft->setCursor(160, 170);
      tft->print("Timer 4");
  
      tft->setCursor(180, 200);
      tft->print("Timer ON:");


     // --- ДИНАМИЧЕСКИЕ ЗНАЧЕНИЯ (Нули) ДЛЯ ВСЕХ ТАЙМЕРОВ ---
    
     // Timer ON (для всех)
      tft->setCursor(180, 80); // Timer1 ON
      tft->print("00:00");
    
      tft->setCursor(320, 80); // Timer2 ON (вторая колонка)
      tft->print("00:00");
    
      tft->setCursor(180, 200); // Timer3 ON (третья строка)
      tft->print("00:00");
    
     // Timer OFF (для всех)
      tft->setCursor(180, 110); // Timer1 OFF
      tft->print("00:00");
    
      tft->setCursor(320, 110); // Timer2 OFF
      tft->print("00:00");
    
      tft->setCursor(180, 230); // Timer3 OFF
      // Оставим координату Y=234 для последнего элемента, чтобы не налезало на "Clear"
  }
  delay (5000);
  tft->fillScreen(COLOR_BACKGROUND);


}   