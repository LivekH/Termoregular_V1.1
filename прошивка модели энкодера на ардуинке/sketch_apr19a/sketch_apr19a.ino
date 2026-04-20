// --- 1. Определение пинов ---
const int BTN_RIGHT = 2; // Пин для кнопки "Вправо"
const int BTN_LEFT  = 3; // Пин для кнопки "Влево"

const int CH_A = 5; // Выход A энкодера
const int CH_B = 6; // Выход B энкодера

// --- 2. Переменные для отслеживания состояния ---
bool btnRightPressed = false;
bool btnLeftPressed  = false;

// --- НОВОЕ: Переменные для таймера ---
// unsigned long хранит время в миллисекундах. Тип long нужен, чтобы переменная не переполнилась быстро.
unsigned long lastStepTimeRight = 0; // Время последнего шага для кнопки "Вправо"
unsigned long lastStepTimeLeft = 0;  // Время последнего шага для кнопки "Влево"

// Константа, определяющая скорость "вращения" (частоту импульсов)
// Чем меньше число, тем быстрее вращение.
const long stepDelay = 50; // Задержка между шагами в миллисекундах

// --- 3. Блок настройки (setup) ---
void setup() {
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  
  pinMode(CH_A, OUTPUT);
  pinMode(CH_B, OUTPUT);
  
  digitalWrite(CH_A, LOW);
  digitalWrite(CH_B, LOW);
}

// --- 4. Функция генерации шага (без изменений) ---
void generateStep(bool directionRight) {
  if(directionRight) {
    // Вращение ВПРАВО: A -> B
    digitalWrite(CH_A, HIGH);
    delayMicroseconds(200);
    digitalWrite(CH_B, HIGH);
    delayMicroseconds(200);
    digitalWrite(CH_A, LOW);
    delayMicroseconds(200);
    digitalWrite(CH_B, LOW);
  } else {
    // Вращение ВЛЕВО: B -> A
    digitalWrite(CH_B, HIGH);
    delayMicroseconds(200);
    digitalWrite(CH_A, HIGH);
    delayMicroseconds(200);
    digitalWrite(CH_B, LOW);
    delayMicroseconds(200);
    digitalWrite(CH_A, LOW);
  }
}

// --- 5. Основной цикл программы (loop) ---
void loop() {

  // --- Блок проверки кнопки "Вправо" ---
  if(digitalRead(BTN_RIGHT) == LOW) {
    
    // Если это самое начало нажатия
    if(!btnRightPressed) {
      btnRightPressed = true;
      // Сразу делаем первый шаг при нажатии
      generateStep(true); 
      lastStepTimeRight = millis(); // Запоминаем время этого шага
    }
    // Если кнопка УЖЕ была нажата, проверяем время для следующего шага
    else {
      // Проверяем, прошло ли достаточно времени с последнего шага
      if(millis() - lastStepTimeRight >= stepDelay) {
        generateStep(true); // Делаем следующий шаг
        lastStepTimeRight = millis(); // Обновляем время последнего шага
      }
    }
    
  } else {
    // Если кнопка отпущена, сбрасываем флаг и готовность к таймингу
    btnRightPressed = false;
  }


  // --- Блок проверки кнопки "Влево" ---
  if(digitalRead(BTN_LEFT) == LOW) {
    
    if(!btnLeftPressed) {
      btnLeftPressed = true;
      generateStep(false); // Первый шаг при нажатии влево
      lastStepTimeLeft = millis();
    }
    else {
      if(millis() - lastStepTimeLeft >= stepDelay) {
        generateStep(false); // Следующий шаг влево
        lastStepTimeLeft = millis();
      }
    }
    
   } else {
     btnLeftPressed = false;
   }
}
