// Include necessary libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <DHT.h>
#include <EEPROM.h>

// Define DHT sensor pin and type
#define DHTPIN 13
#define DHTTYPE DHT11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Variables for switching between time/date and temp/humidity on LCD
unsigned long lastSwitchTime = 0;
bool showTimeDate = true;

// Define pins for shift register and LEDs
int clockpin = 2;
int latchpin = 3;
int datapin = 4;
int led_red = 10;
int led_yellow = 11;
int led_green = 12;
int switchPin = 9; // Mode switch input

// 7-segment display patterns for digits 0-9
int segdisp[10] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111,
  0b01100110, 0b01101101, 0b01111101, 0b00100111,
  0b01111111, 0b01101111
};

// Initialize LCD and RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 rtc(SDA, SCL);

// Arrays to store timing and repeat data for 8 time frames
int green_times[8], yellow_times[8], red_times[8], repeat_counts[8], repeat_count_blink = 0;

// Write int array to EEPROM
void writeIntArrayToEEPROM(int startAddr, int* data, int length) {
  for (int i = 0; i < length; i++) {
    EEPROM.put(startAddr + i * sizeof(int), data[i]);
  }
}

// Read int array to EEPROM
void readIntArrayFromEEPROM(int startAddr, int* data, int length) {
  for (int i = 0; i < length; i++) {
    EEPROM.get(startAddr + i * sizeof(int), data[i]);
  }
}

void setup() {
  Serial.begin(9600); // Start serial communication

  // Set pin modes for shift register and LEDs
  pinMode(latchpin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(datapin, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(switchPin, INPUT);

  // Initialize I2C, LCD, DHT and RTC
  Wire.begin();
  lcd.init();
  lcd.backlight();
  dht.begin();
  rtc.begin();

  // Set time and date (manual setup)
  rtc.setDOW(WEDNESDAY);     // Set day of the week
  rtc.setTime(8, 58, 10);     // Set hours, minutes, seconds
  rtc.setDate(7, 6, 2025);   // Date

  // Read data from EEPROM
  int addr = 0;
  readIntArrayFromEEPROM(addr, green_times, 8); addr += 8 * sizeof(int);
  readIntArrayFromEEPROM(addr, yellow_times, 8); addr += 8 * sizeof(int);
  readIntArrayFromEEPROM(addr, red_times, 8); addr += 8 * sizeof(int);
  readIntArrayFromEEPROM(addr, repeat_counts, 8); addr += 8 * sizeof(int);
  EEPROM.get(addr, repeat_count_blink);

  // Display current time and date when system starts
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(rtc.getTimeStr());
  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(rtc.getDateStr());
}

// Countdown function for a specific time and LED
void countdown(int start, int led) {
  digitalWrite(led, HIGH);
  unsigned long lastUpdate = millis();

  for (int z = start; z >= 0; z--) {
    displayNumber(z); // Display the current number

   // Wait exactly 1000ms but not block
    while (millis() - lastUpdate < 1000) {
      if (digitalRead(switchPin) == HIGH) {
        turnOffAll();
        return;
      }
      updateLCD(); // Update LCD 
    }
    lastUpdate += 1000; // Update next time point
  }

  digitalWrite(led, LOW);
}


// Turn off all LEDs and clear 7-segment display
void turnOffAll() {
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, MSBFIRST, 0x00);
  shiftOut(datapin, clockpin, MSBFIRST, 0x00);
  digitalWrite(latchpin, HIGH);
}

// Display a two-digit number on the 7-segment display
void displayNumber(int num) {
  int e = num / 10;
  int c = num % 10;
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, MSBFIRST, segdisp[c]);
  shiftOut(datapin, clockpin, MSBFIRST, segdisp[e]);
  digitalWrite(latchpin, HIGH);
}

// Blink yellow LED mode, used at night or special modes
void blinkYellow(int times = 1) {
  turnOffAll();
  for (int i = 0; i < times; i++) {
    if (digitalRead(switchPin) == HIGH) {
      turnOffAll();
      return;
    }
    digitalWrite(led_yellow, HIGH);
    updateLCD();
    delay(1000);
    digitalWrite(led_yellow, LOW);
    updateLCD();
    delay(1000);
  }
}

// Alternate between displaying time/date and temperature/humidity every 7 seconds
void updateLCD() {
  static bool lastShowTimeDate = true;

  if (millis() - lastSwitchTime >= 7000) {
    showTimeDate = !showTimeDate;
    lastSwitchTime = millis();
  }
  if (showTimeDate != lastShowTimeDate) {
    lcd.clear();
    lastShowTimeDate = showTimeDate;
  }
  if (showTimeDate) {
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(rtc.getTimeStr());
    lcd.setCursor(0, 1);
    lcd.print("Date: ");
    lcd.print(rtc.getDateStr());
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(dht.readTemperature());
    lcd.print(" C  ");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(dht.readHumidity());
    lcd.print("%  ");
  }
}
void loop() {
  // Read timing data from serial input, e.g., from a Python program
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    sscanf(input.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
           &green_times[0], &yellow_times[0], &red_times[0], &repeat_counts[0],
           &green_times[1], &yellow_times[1], &red_times[1], &repeat_counts[1],
           &green_times[2], &yellow_times[2], &red_times[2], &repeat_counts[2],
           &green_times[3], &yellow_times[3], &red_times[3], &repeat_counts[3],
           &green_times[4], &yellow_times[4], &red_times[4], &repeat_counts[4],
           &green_times[5], &yellow_times[5], &red_times[5], &repeat_counts[5],
           &green_times[6], &yellow_times[6], &red_times[6], &repeat_counts[6],
           &green_times[7], &yellow_times[7], &red_times[7], &repeat_counts[7],
           &repeat_count_blink);

          // Save to EEPROM
    int addr = 0;
    writeIntArrayToEEPROM(addr, green_times, 8); addr += 8 * sizeof(int);
    writeIntArrayToEEPROM(addr, yellow_times, 8); addr += 8 * sizeof(int);
    writeIntArrayToEEPROM(addr, red_times, 8); addr += 8 * sizeof(int);
    writeIntArrayToEEPROM(addr, repeat_counts, 8); addr += 8 * sizeof(int);
    EEPROM.put(addr, repeat_count_blink);
  }

  // Run light sequences only when switch is LOW
  if (digitalRead(switchPin) == LOW) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < repeat_counts[i]; j++) {
        countdown(green_times[i], led_green);
        countdown(yellow_times[i], led_yellow);
        countdown(red_times[i], led_red);
      }
    }
    // Run blinking yellow light if set (once each)
    for (int k = 0; k < repeat_count_blink; k++) {
      blinkYellow(1);
    }
  } else {
    // Turn off all if switch is HIGH
    turnOffAll();
  }

  // Refresh LCD content
  updateLCD();
}
