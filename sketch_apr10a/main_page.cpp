// Main_page.cpp

#include <Arduino_GFX_Library.h>
#include "Main_page.h"

extern Arduino_GFX *tft; // Это нужно, чтобы библиотека могла использовать tft

void drawStaticBackground() {
  // --- СТАТИЧНЫЕ НАДПИСИ (ЧАСТЬ ФОНА) ---
  tft->setTextSize(2);
  tft->setTextColor(tft->color565(255, 255, 255)); // Белый цвет без констант
  
  tft->setCursor(54, 3);
  tft->print("<Sauna Burovichok>");
  
  tft->setCursor(10, 25); 
  tft->print("TIME:");
  
  tft->setCursor(80, 173); // Hu%
  tft->print("Hu%");
  
  tft->setCursor(225, 173); // C°
  tft->print("C");

  // --- ОТРИСТКА ШКАЛ (ЧАСТЬ ФОНА) ---
  drawHumidityScale();
  drawTempScale();
}

void drawHumidityScale() {
  int centerX = 100; // Координаты прописываем прямо тут, чтобы не зависеть от других файлов
  int centerY = 157;

  tft->fillArc(centerX, centerY, 85, 75, 180, 270, tft->color565(128,128,128));
  
  // Цветные сектора БЕЗ цифровых меток!
  tft->fillArc(centerX, centerY, 75, 65, 180, 202.5, tft->color565(255, 0, 0));
  tft->fillArc(centerX, centerY, 75, 65, 202.5, 225, tft->color565(255, 255, 0));
  tft->fillArc(centerX, centerY, 75, 65, 225, 247.5, tft->color565(0, 255, 0));
  tft->fillArc(centerX, centerY, 75, 65, 247.5, 270, tft->color565(0, 0, 255)); 
}

void drawTempScale() {
   int centerX = 205;
   int centerY = 157;

   tft->fillArc(centerX, centerY, 99, 89, 180, 360, tft->color565(128,128,128));

   tft->fillArc(centerX, centerY, 89, 79, 180, 225, tft->color565(0, 0, 255));
   tft->fillArc(centerX, centerY, 89, 79, 225, 270, tft->color565(0, 255, 0));
   tft->fillArc(centerX, centerY, 89, 79, 270, 315, tft->color565(255, 255, 0));
   tft->fillArc(centerX, centerY, 89, 79, 315, 360, tft->color565(255, 0, 0));
}
