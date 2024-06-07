#define HC_ECHO 2       // Пин для Echo сигнала от ультразвукового дальномера
#define HC_TRIG 3       // Пин для Trig сигнала к ультразвуковому дальномеру

#define LED_MAX_MA 1500 // Ограничение максимального тока для светодиодной ленты в миллиамперах
#define LED_PIN 13      // Пин для подключения светодиодной ленты
#define LED_NUM 50      // Количество светодиодов в ленте

#define VB_DEB 0        // Отключение антидребезга (он уже реализован в фильтре)
#define VB_CLICK 900    // Таймаут для определения клика в миллисекундах
#include <VirtualButton.h>
VButton gest;           // Создание объекта для управления виртуальной кнопкой

#include <GRGB.h>
GRGB led;               // Создание объекта для управления RGB светодиодами

#include <FastLED.h>
CRGB leds[LED_NUM];     // Создание массива для хранения состояния каждого светодиода

// Структура для хранения настроек системы
struct Data {
  bool state = 1;     // Включено (1) или выключено (0)
  byte mode = 0;      // Режим работы: 0 - цвет, 1 - теплота, 2 - огонь
  byte bright[3] = {30, 30, 30};  // Яркость для каждого режима
  byte value[3] = {0, 0, 0};      // Параметры эффекта (например, цвет)
};

Data data;            // Создание объекта структуры для хранения текущих настроек

// Менеджер памяти для хранения и восстановления настроек
#include <EEManager.h>
EEManager mem(data);  // Инициализация менеджера памяти с переданными настройками

int prev_br;          // Переменная для хранения предыдущей яркости

void setup() {
  Serial.begin(115200);           // Инициализация последовательного соединения для отладки

  pinMode(HC_TRIG, OUTPUT);       // Установка пина Trig как выход
  pinMode(HC_ECHO, INPUT);        // Установка пина Echo как вход

  // Инициализация библиотеки FastLED для управления светодиодами
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_MA);
  FastLED.setBrightness(255);     // Установка максимальной яркости

  led.setBrightness(0);           // Начальная яркость светодиодов - 0
  led.attach(setLED);             // Привязка функции для обновления светодиодов
  led.setCRT(1);                  // Установка режима CRT для плавного изменения

  mem.begin(0, 'a');              // Инициализация менеджера памяти и чтение настроек
  applyMode();                    // Применение текущего режима работы
}

void loop() {
  mem.tick();  // Обновление менеджера памяти

  if (data.state && data.mode == 2) fireTick();   // Если включено и выбран режим огня, обновляем анимацию огня

  // Таймер с периодом 50 мс для опроса датчика и выполнения основной логики
  static uint32_t tmr;
  if (millis() - tmr >= 50) {
    tmr = millis();

    static uint32_t tout;   // Таймер для определения времени удержания
    static int offset_d;    // Смещение для настройки параметров
    static byte offset_v;

    int dist = getDist(HC_TRIG, HC_ECHO); // Получение расстояния с датчика
    dist = getFilterMedian(dist);         // Применение медианного фильтра
    dist = getFilterSkip(dist);           // Применение пропускающего фильтра
    int dist_f = getFilterExp(dist);      // Применение экспоненциального фильтра

    gest.poll(dist);                      // Обновление состояния виртуальной кнопки на основе расстояния

    // Обработка кликов и удержаний с таймаутом в 2 секунды
    if (gest.hasClicks() && millis() - tout > 2000) {
      switch (gest.clicks) {
        case 1:
          data.state = !data.state;  // Включение/выключение
          break;
        case 2:
          // Переключение режима при включенном состоянии
          if (data.state && ++data.mode >= 3) data.mode = 0;
          break;
      }
      applyMode();  // Применение нового режима
    }

    // Обработка одиночного клика
    if (gest.click() && data.state) {
      pulse();  // Мигнуть яркостью
    }

    // Обработка начала удержания
    if (gest.held() && data.state) {
      pulse();  // Мигнуть яркостью
      offset_d = dist_f;    // Сохранение текущего расстояния для последующей настройки
      switch (gest.clicks) {
        case 0: offset_v = data.bright[data.mode]; break;   // Сохранение текущей яркости
        case 1: offset_v = data.value[data.mode]; break;    // Сохранение текущего значения эффекта
      }
    }

    // Обработка удержания (выполняется пока кнопка удерживается)
    if (gest.hold() && data.state) {
      tout = millis();
      // Вычисление смещения настройки на основе начального и текущего расстояния
      int shift = constrain(offset_v + (dist_f - offset_d), 0, 255);
      
      // Применение новых значений настройки
      switch (gest.clicks) {
        case 0: data.bright[data.mode] = shift; break;
        case 1: data.value[data.mode] = shift; break;
      }
      applyMode();  // Применение изменений
    }
  }
}

// Функция для получения расстояния с ультразвукового дальномера
#define HC_MAX_LEN 1000L  // Максимальное измеряемое расстояние в мм
int getDist(byte trig, byte echo) {
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // Измерение времени отклика сигнала
  uint32_t us = pulseIn(echo, HIGH, (HC_MAX_LEN * 2 * 1000 / 343));

  // Вычисление и возврат расстояния в мм
  return (us * 343L / 2000);
}

// Медианный фильтр для устранения выбросов
int getFilterMedian(int newVal) {
  static int buf[3];
  static byte count = 0;
  buf[count] = newVal;
  if (++count >= 3) count = 0;
  return (max(buf[0], buf[1]) == max(buf[1], buf[2])) ? max(buf[0], buf[2]) : max(buf[1], min(buf[0], buf[2]));
}

// Пропускающий фильтр для исключения резких изменений
#define FS_WINDOW 7   // Количество измерений для сглаживания
#define FS_DIFF 80    // Допустимая разница между измерениями
int getFilterSkip(int val) {
  static int prev;
  static byte count;

  if (!prev && val) prev = val;   // Инициализация предыдущего значения
  if (abs(prev - val) > FS_DIFF || !val) {
    count++;
    if (count > FS_WINDOW) {
      prev = val;
      count = 0;
    } else val = prev;
  } else count = 0;   // Сброс счетчика
  prev = val;
  
  return val;
}

// Экспоненциальный фильтр для плавного изменения значений
#define ES_EXP 2L     // Коэффициент сглаживания (чем больше, тем плавнее)
#define ES_MULT 16L   // Множитель для повышения разрешения
int getFilterExp(int val) {
  static long filt;
  if (val) filt += (val * ES_MULT - filt) / ES_EXP;
  else filt = 0;  // Сброс фильтра до нуля при значении 0
  return filt / ES_MULT;
}

// Применение текущего режима и яркости
#define BR_STEP 4
void applyMode() {
  if (data.state) {
    switch (data.mode) {
      case 0: led.setWheel8(data.value[0]); break;
      case 1: led.setKelvin(data.value[1] * 28); break;
    }

    // Плавное изменение яркости при включении и смене режима
    if (prev_br != data.bright[data.mode]) {
      int shift = prev_br > data.bright[data.mode] ? -BR_STEP : BR_STEP;
      while (abs(prev_br - data.bright[data.mode]) > BR_STEP) {
        prev_br += shift;
        led.setBrightness(prev_br);
        delay(10);
      }
      prev_br = data.bright[data.mode];
    }
  } else {
    // Плавное уменьшение яркости при выключении
    while (prev_br > 0) {
      prev_br -= BR_STEP;
      if (prev_br < 0) prev_br = 0;
      led.setBrightness(prev_br);
      delay(10);
    }
  }
  
  mem.update(); // Обновление настроек в памяти
}

// Функция для обновления состояния светодиодов
void setLED() {
  FastLED.showColor(CRGB(led.R, led.G, led.B));
}

// Функция для анимации огня
void fireTick() {
  static uint32_t rnd_tmr, move_tmr;
  static int rnd_val, fil_val;
  
  // Таймер с периодом 100 мс для генерации случайных значений
  if (millis() - rnd_tmr > 100) {
    rnd_tmr = millis();
    rnd_val = random(0, 13);
  }
  
  // Таймер с периодом 20 мс для плавного изменения значений
  if (millis() - move_tmr > 20) {
    move_tmr = millis();
    fil_val += (rnd_val * 10 - fil_val) / 5;

    // Преобразование в яркость от 100 до 255
    int br = map(fil_val, 0, 120, 100, 255);

    // Преобразование в цветовой оттенок
    int hue = data.value[2] + fil_val / 5;
    led.setWheel8(hue, br);
  }
}

// Функция для кратковременного увеличения яркости (пульсация)
void pulse() {
  for (int i = prev_br; i < prev_br + 45; i += 3) {
    led.setBrightness(min(255, i));
    delay(10);
  }
  for (int i = prev_br + 45; i > prev_br; i -= 3) {
    led.setBrightness(min(255, i));
    delay(10);
  }
}
