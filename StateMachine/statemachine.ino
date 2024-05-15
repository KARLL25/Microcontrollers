#define DIR_RIGHT 7
#define SPEED_RIGHT 6
#define DIR_LEFT 4
#define SPEED_LEFT 5

#define PIN_TRIG_FRONT 3
#define PIN_ECHO_FRONT 2
#define PIN_TRIG_RIGHT 7 
#define PIN_ECHO_RIGHT 6 

void setup() {
  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(SPEED_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(SPEED_LEFT, OUTPUT);
  pinMode(PIN_TRIG_FRONT, OUTPUT);
  pinMode(PIN_ECHO_FRONT, INPUT);
  pinMode(PIN_TRIG_RIGHT, OUTPUT);
  pinMode(PIN_ECHO_RIGHT, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  long duration_front, distance_front;
  long duration_right, distance_right;
  
  digitalWrite(PIN_TRIG_FRONT, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG_FRONT, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG_FRONT, LOW);
  duration_front = pulseIn(PIN_ECHO_FRONT, HIGH);
  distance_front = (duration_front / 2) / 29.1; 
  
  digitalWrite(PIN_TRIG_RIGHT, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG_RIGHT, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG_RIGHT, LOW);
  duration_right = pulseIn(PIN_ECHO_RIGHT, HIGH);
  distance_right = (duration_right / 2) / 29.1; 
  
  int speed_left = 200; // базовая скорость
  int speed_right = 200; 
  
  if (distance_front < 3 && distance_right > 3) { // если передний дальномер обнаружил препятствие, а правый - нет
    digitalWrite(DIR_RIGHT, HIGH);
    digitalWrite(DIR_LEFT, LOW);
    speed_left = 150; 
    speed_right = 200; 
  } else if (distance_right < 3) { // если правый дальномер обнаружил стену
    digitalWrite(DIR_RIGHT, HIGH); // поворот налево
    digitalWrite(DIR_LEFT, HIGH);
    speed_left = 150; 
    speed_right = 200; 
  } else if (distance_front > 3 && distance_right < 3){ // если передний дальномер не обнаружил препятствие, а правый - да
    digitalWrite(DIR_RIGHT, HIGH);
    digitalWrite(DIR_LEFT, HIGH);
    speed_left = 200; 
    speed_right = 200;
  } else if (distance_front < 3 && distance_right < 3){ // если правый и передний дальномер обнаружили препятствие
    digitalWrite(DIR_RIGHT, HIGH); // поворот налево
    digitalWrite(DIR_LEFT, HIGH);
    speed_left = 150; 
    speed_right = 200;
  }
   else {
    // Движение вперед:
    digitalWrite(DIR_RIGHT, HIGH);
    digitalWrite(DIR_LEFT, HIGH);
    speed_left = 200; 
    speed_right = 200;
  }
  
  // установка скорости 
  analogWrite(SPEED_RIGHT, speed_right);
  analogWrite(SPEED_LEFT, speed_left);
}