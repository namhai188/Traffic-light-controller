// Định nghĩa chân kết nối với 74HC595
int clockpin = 2;
int latchpin = 3;
int datapin = 4;
int speed = 1000;

// Định nghĩa chân LED giao thông
int led_red = 10;
int led_yellow = 11;
int led_green = 12;
int switchPin = 9;

// Định nghĩa bảng mã 7-segment
int segdisp[10] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
  0b01101101, 0b01111101, 0b00100111, 0b01111111, 0b01101111
};

void setup() {
  pinMode(latchpin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(datapin, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(switchPin, INPUT);
}

void displayNumber(int num) {
  int e = num / 10;
  int c = num % 10;
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, MSBFIRST, segdisp[c]);
  shiftOut(datapin, clockpin, MSBFIRST, segdisp[e]);
  digitalWrite(latchpin, HIGH);
}

void clearDisplay() {
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, MSBFIRST, 0x00);
  shiftOut(datapin, clockpin, MSBFIRST, 0x00);
  digitalWrite(latchpin, HIGH);
}

void countdown(int start, int led) {
  digitalWrite(led, HIGH);
  for (int z = start; z >= 0; z--) {
    if (digitalRead(switchPin) == HIGH) {
      turnOffAll();
      return;
    }
    displayNumber(z);
    delay(speed);
  }
  digitalWrite(led, LOW);
}

void blinkYellow(int times) {
  clearDisplay();
  for (int i = 0; i < times; i++) {
    if (digitalRead(switchPin) == HIGH) {
      turnOffAll();
      return;
    }
    digitalWrite(led_yellow, HIGH);
    delay(1000);
    digitalWrite(led_yellow, LOW);
    delay(1000);
  }
}

void turnOffAll() {
  clearDisplay();
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
}

void loop() {
  if (digitalRead(switchPin) == LOW) {
    // Chế độ Trung bình 1 (05:00 - 06:30)
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    // Chế độ Cao (06:30 - 09:00)
    for (int i = 0; i < 0; i++) { countdown(30, led_green); countdown(3, led_yellow); countdown(10, led_red); }
    // Chế độ Trung bình 2 (09:00 - 11:30)
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    // Chế độ Thấp (11:30 - 13:30)
    for (int i = 0; i < 0; i++) { countdown(15, led_green); countdown(3, led_yellow); countdown(20, led_red); }
    // Chế độ Trung bình 3 (13:30 - 16:00)
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    // Chế độ Rất Cao (16:30 - 19:30)
    for (int i = 0; i < 0; i++) { countdown(35, led_green); countdown(3, led_yellow); countdown(7, led_red); }
    // Chế độ Trung bình - Thấp (19:30 - 22:00)
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    // Chế độ Rất Thấp (22:00 - 05:00)
    for (int i = 0; i < 1; i++) { blinkYellow(1); }
  } else {
    turnOffAll();
    while (digitalRead(switchPin) == HIGH) {
      delay(1000);
    }
  }
}
