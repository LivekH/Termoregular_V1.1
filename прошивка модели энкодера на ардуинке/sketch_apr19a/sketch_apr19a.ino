// --- 1. Определение пинов ---
// Объявляем константы для номеров пинов, чтобы код был читаемым.
// Это входы от наших кнопок.
const int BTN_RIGHT = 2; // Пин PD2 (цифровой пин 2 на Arduino)
const int BTN_LEFT  = 3; // Пин PD3 (цифровой пин 3 на Arduino)

// Это выходы, которые будут имитировать сигналы энкодера.
const int CH_A = 5; // Пин PB0 (цифровой пин 8 на Arduino)
const int CH_B = 6; // Пин PB1 (цифровой пин 9 на Arduino)

// --- 2. Переменные для отслеживания состояния кнопок ---
// Булевы переменные (флаги), чтобы отследить момент нажатия и избежать повторных срабатываний.
bool btnRightPressed = false;
bool btnLeftPressed  = false;

// --- 3. Блок настройки (setup) ---
void setup() {
  // Настраиваем пины кнопок как ВХОДЫ.
  // INPUT_PULLUP включает внутренний резистор, который "подтягивает" пин к питанию (+5В).
  // Это значит, что когда кнопка НЕ нажата, digitalRead() вернет HIGH (1).
  // А когда кнопка нажата и соединяет пин с землей, вернется LOW (0).
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);

  // Настраиваем выходы энкодера как ВЫХОДЫ.
  pinMode(CH_A, OUTPUT);
  pinMode(CH_B, OUTPUT);

  // Устанавливаем начальное состояние выходов в НИЗКИЙ уровень (0 Вольт).
  // Это состояние покоя энкодера.
  digitalWrite(CH_A, LOW);
  digitalWrite(CH_B, LOW);
}

// --- 4. Вспомогательная функция для генерации шага ---
// Эта функция делает всю "грязную" работу по переключению уровней напряжения.
void generateStep(bool directionRight) {

  // Если направление "вправо" (true)
  if(directionRight) {
    // Последовательность для вращения ВПРАВО: сначала A, потом B.
    digitalWrite(CH_A, HIGH); // Поднимаем сигнал A вверх (+5В)
    delayMicroseconds(200);   // Ждем немного, чтобы импульс был заметен
    digitalWrite(CH_B, HIGH); // Поднимаем сигнал B вверх
    delayMicroseconds(200);
    digitalWrite(CH_A, LOW);  // Опускаем сигнал A вниз (0В)
    delayMicroseconds(200);
    digitalWrite(CH_B, LOW);  // Опускаем сигнал B вниз
    delayMicroseconds(200);

  } else {
    // Если направление "влево" (false)
    // Последовательность для вращения ВЛЕВО: сначала B, потом A.
    digitalWrite(CH_B, HIGH); // Поднимаем сигнал B вверх
    delayMicroseconds(200);
    digitalWrite(CH_A, HIGH); // Поднимаем сигнал A вверх
    delayMicroseconds(200);
    digitalWrite(CH_B, LOW);  // Опускаем сигнал B вниз
    delayMicroseconds(200);
    digitalWrite(CH_A, LOW);  // Опускаем сигнал A вниз
    delayMicroseconds(200);
  }
}

// --- 5. Основной цикл программы (loop) ---
void loop() {

  // --- Блок проверки кнопки "Вправо" ---

  // Проверяем, нажата ли кнопка RIGHT.
  // Из-за INPUT_PULLUP при нажатии будет читаться LOW (0).
  if(digitalRead(BTN_RIGHT) == LOW) {

    // Проверяем флаг. Если кнопка уже была нажата и мы еще не вышли из цикла,
    // это условие не даст функции generateStep() выполниться еще раз.
    if(!btnRightPressed) {
      btnRightPressed = true; // Устанавливаем флаг в "истина"
      generateStep(true);     // Вызываем функцию генерации импульса для движения ВПРАВО

      // Цикл задержки. Программа будет стоять здесь, пока кнопка нажата.
      // Это нужно, чтобы не генерировать сотни импульсов, пока кнопка зажата.
      while(digitalRead(BTN_RIGHT) == LOW);
    }

    // Если кнопка отпущена (на входе HIGH), сбрасываем флаг в "ложь".
    // Это позволит нам нажать кнопку снова в будущем.
  } else {
    btnRightPressed = false;
  }


  // --- Блок проверки кнопки "Влево" ---

  // Полностью аналогичная логика для второй кнопки.
  if(digitalRead(BTN_LEFT) == LOW) {
    if(!btnLeftPressed) {
      btnLeftPressed = true;
      generateStep(false); // Вызываем функцию для движения ВЛЕВО

      while(digitalRead(BTN_LEFT) == LOW);
    }

   } else {
     btnLeftPressed = false;
   }
}
