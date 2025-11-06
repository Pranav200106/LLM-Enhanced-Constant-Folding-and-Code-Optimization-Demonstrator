#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Keypad setup ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Globals ---
String incomingText = "";
bool newMessage = false;
bool isScrolling = false;
bool interruptScroll = false;

const int SCROLL_DELAY = 600;
const int PAUSE_TIME = 1000;

void scrollText(String msg, int speedMs);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Ready...");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == '1' || key == '2' || key == '3') {
      if (isScrolling) {
        interruptScroll = true; // signal scroll to stop
      }

      lcd.clear();
      lcd.print("Requesting...");
      Serial.println(key);
      delay(300);
    }
  }

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      newMessage = true;
    } else {
      incomingText += c;
    }
  }

  if (newMessage) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Received!");
    delay(500);

    scrollText(incomingText, SCROLL_DELAY);

    incomingText = "";
    newMessage = false;
    lcd.clear();
    lcd.print("Ready...");
  }
}

void scrollText(String msg, int speedMs) {
  msg.trim();
  msg += " ";
  int len = msg.length();

  isScrolling = true;
  interruptScroll = false;

  if (len <= 16) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg);
    delay(PAUSE_TIME);
    isScrolling = false;
    return;
  }

  delay(PAUSE_TIME);

  for (int i = 0; i <= len - 16; i++) {
    // 🔹 Check if a new key is pressed mid-scroll
    char k = keypad.getKey();
    if (k == '1' || k == '2' || k == '3') {
      interruptScroll = true;
      Serial.println(k); // tell Python to send new text
    }

    // 🔹 Break out if interrupted
    if (interruptScroll) {
      isScrolling = false;
      return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg.substring(i, i + 16));
    delay(speedMs);
  }

  delay(PAUSE_TIME);
  isScrolling = false;
}
