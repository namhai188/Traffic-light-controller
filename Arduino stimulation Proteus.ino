#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <DHT.h>

// LCD
LiquidCrystal_I2C lcd(0x20, 16, 2); // I2C address, LCD 16x2

// RTC
DS3231 rtc(SDA, SCL);

// DHT11
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD display switching time variable
unsigned long lastSwitchTime = 0;
bool showTime = true;
unsigned long lastLcdUpdate = 0;

// 74HC595
int clockpin = 2;
int latchpin = 3;
int datapin = 4;
int speed = 1000;

int led_red = 10;
int led_yellow = 11;
int led_green = 12;
int switchPin = 9;

// 7-segment code
int segdisp[10] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
  0b01101101, 0b01111101, 0b00100111, 0b01111111, 0b01101111
};

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

void turnOffAll() {
  clearDisplay();
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
}

// Countdown with switch test and LCD update
void countdown(int start, int led) {
  digitalWrite(led, HIGH);
  for (int z = start; z >= 0; z--) {
    if (digitalRead(switchPin) == HIGH) {
      turnOffAll();
      return;
    }

    displayNumber(z);
    updateLCD();  // Update LCD in each loop
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
    updateLCD(); delay(1000);
    digitalWrite(led_yellow, LOW);
    updateLCD(); delay(1000);
  }
}

// Function to update LCD screen (alternates between time/date and temp/humidity)
void updateLCD() {
  unsigned long now = millis();

  if (now - lastSwitchTime >= 7000) {
    showTime = !showTime;
    lcd.clear();
    lastSwitchTime = now;
  }

  if (now - lastLcdUpdate < 500) return;
  lastLcdUpdate = now;

  if (showTime) {
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(rtc.getTimeStr());

    lcd.setCursor(0, 1);
    lcd.print("Date: ");
    lcd.print(rtc.getDateStr());

  } else {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    if (isnan(temp)) lcd.print("Err");
    else {
      lcd.print(temp); lcd.print(" C");
    }

    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    if (isnan(hum)) lcd.print("Err");
    else {
      lcd.print(hum); lcd.print("%");
    }
  }
}

void setup() {
  // LCD, RTC, DHT
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  rtc.begin();
  dht.begin();

  // RTC time set - only need 1 time
  rtc.setDOW(WEDNESDAY);
  rtc.setTime(21, 15, 10);
  rtc.setDate(4, 6, 2025);

  // 74HC595 and LED
  pinMode(latchpin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(datapin, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(switchPin, INPUT);
}

void loop() {
  if (digitalRead(switchPin) == LOW) {
    for (int i = 0; i < 1; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    for (int i = 0; i < 0; i++) { countdown(30, led_green); countdown(3, led_yellow); countdown(10, led_red); }
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    for (int i = 0; i < 0; i++) { countdown(15, led_green); countdown(3, led_yellow); countdown(20, led_red); }
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    for (int i = 0; i < 0; i++) { countdown(35, led_green); countdown(3, led_yellow); countdown(7, led_red); }
    for (int i = 0; i < 0; i++) { countdown(20, led_green); countdown(3, led_yellow); countdown(15, led_red); }
    // Blinking yellow light at night 
    for (int i = 0; i < 5; i++) { blinkYellow(1); }
  } else {
    turnOffAll();
    updateLCD();
    delay(500);
  }
}
