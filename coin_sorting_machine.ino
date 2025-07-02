#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int coinPin = 2;         // Coin acceptor pulse pin
const int buttonPin = 8;       // Mode toggle button
const int led1 = 9;            // 100 UGX LED
const int led2 = 10;           // 200 UGX LED
const int led3 = 11;           // 500 UGX LED

volatile int pulse = 0;
unsigned long lastPulseTime = 0;
bool coinReady = false;

// Coin value constants
const int coinValue1 = 100;
const int coinValue2 = 200;
const int coinValue3 = 500;

// Coin counters
int c1 = 0, c2 = 0, c3 = 0;

// Button state tracking
int buttonState = 0;
int lastButtonState = 0;
bool showingTotals = false;

// LED blink timing
unsigned long led1Time = 0;
unsigned long led2Time = 0;
unsigned long led3Time = 0;
const unsigned long blinkDuration = 1000;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("  ARDUINO BASED ");
  lcd.setCursor(0, 1); lcd.print("  COIN SORTING   ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("UGX1  UGX2  UGX5");

  pinMode(coinPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(coinPin), coinInterrupt, RISING);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
}

void loop() {
  // Debounced button press
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    showingTotals = !showingTotals;
    delay(50); // debounce

    if (showingTotals) {
      digitalWrite(led1, HIGH); digitalWrite(led2, HIGH); digitalWrite(led3, HIGH);
      displayTotals();
    } else {
      // ✅ Clear screen and reset counts when leaving total view
      c1 = 0;
      c2 = 0;
      c3 = 0;
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("UGX1  UGX2  UGX5");
      digitalWrite(led1, LOW); digitalWrite(led2, LOW); digitalWrite(led3, LOW);
    }
  }

  lastButtonState = buttonState;

  if (showingTotals) return;

  // Check for settled pulse count (no pulse for 300 ms)
  if (pulse > 0 && (millis() - lastPulseTime > 300)) {
    handleCoin(pulse);
    pulse = 0;
  }

  // Handle LED timing
  if (led1Time > 0 && millis() - led1Time >= blinkDuration) {
    digitalWrite(led1, LOW); led1Time = 0;
  }
  if (led2Time > 0 && millis() - led2Time >= blinkDuration) {
    digitalWrite(led2, LOW); led2Time = 0;
  }
  if (led3Time > 0 && millis() - led3Time >= blinkDuration) {
    digitalWrite(led3, LOW); led3Time = 0;
  }

  // Update live counts on LCD
  lcd.setCursor(1, 1); lcd.print(c1);
  lcd.setCursor(7, 1); lcd.print(c2);
  lcd.setCursor(14, 1); lcd.print(c3);
}

// Interrupt service routine
void coinInterrupt() {
  pulse++;
  lastPulseTime = millis();
}

// Coin identification & LED response
void handleCoin(int p) {
  if (p == 4) {
    c1++;
    digitalWrite(led1, HIGH);
    led1Time = millis();
  } else if (p == 2) {
    c2++;
    digitalWrite(led2, HIGH);
    led2Time = millis();
  } else if (p == 6) {
    c3++;
    digitalWrite(led3, HIGH);
    led3Time = millis();
  } else {
    // Unknown coin
    lcd.setCursor(0, 1); lcd.print(" Invalid Coin    ");
    delay(1000);
    lcd.setCursor(0, 1); lcd.print("                 ");
  }
}

// Display totals on button press
void displayTotals() {
  lcd.clear();

  long total1 = c1 * (long)coinValue1;
  long total2 = c2 * (long)coinValue2;
  long total3 = c3 * (long)coinValue3;
  long grandTotal = total1 + total2 + total3;

  lcd.setCursor(0, 0); lcd.print("S1:"); lcd.print(total1);
  lcd.setCursor(8, 0); lcd.print("S2:"); lcd.print(total2);
  lcd.setCursor(0, 1); lcd.print("S5:"); lcd.print(total3);
  lcd.setCursor(8, 1); lcd.print("T:"); lcd.print(grandTotal);
}
