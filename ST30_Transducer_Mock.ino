#include <LiquidCrystal.h>

// Analog values for LCD buttons
#define RIGHT_BUTTON_VAL 60
#define UP_BUTTON_VAL 200
#define DOWN_BUTTON_VAL 400
#define LEFT_BUTTON_VAL 600
#define SELECT_BUTTON_VAL 800

#define ARDUINO_MICROSECONDS_DELAY_MAX (0x4000 - 1)  // 14 bit

// LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
LiquidCrystal lcd(pin_RS, pin_EN, pin_d4, pin_d5, pin_d6, pin_d7);

// Pins to ST30
const int pin_pulseIn = 2;  // InPin, triggers an interrupt when ST30 generates a pulse to transducer
const int pin_echoOut = 3;  // OutPin, Toggles high to emulate echo of pulse

// Depth selection and display
uint8_t currentDepth = 0;     // Selected depth from user
int8_t selectedRelDepth = 0;  // Previously selected depth (needs to be outside of loop scope)

// Delay calculation
uint32_t calculatedDepthDelay = 0;                 // Wait time in microseconds
const uint32_t METER_TO_DELAY = 1333;              // Rough estimate in microseconds
const uint32_t CORRECTION = 0.7 * METER_TO_DELAY;  // Some calibration (maybe to to some electronics delay and/or slow arduino[libraries])

// Print depth to LCD
void printDepth() {
  lcd.clear();
  lcd.print("Depth: ");
  lcd.print(currentDepth);
  lcd.print(" m");
}

// Wraps delayMicroseconds for higher delays
// Arduino's microseconds delay is only 14 bit length. milliseconds delay-function doesn't seem to work properly in ISR
void us_delay(uint32_t waitTime) {
  while (waitTime > ARDUINO_MICROSECONDS_DELAY_MAX) {
    delayMicroseconds(ARDUINO_MICROSECONDS_DELAY_MAX);
    waitTime -= ARDUINO_MICROSECONDS_DELAY_MAX;
  }
  if (waitTime > 0) {
    delayMicroseconds(waitTime);
  }
}

// Emulates echo to ST30
void echoISR() {
  // Wait calculated time
  us_delay(calculatedDepthDelay);

  // Toggle pin
  digitalWrite(pin_echoOut, HIGH);
  digitalWrite(pin_echoOut, LOW);
}

// Setup Serial, LCD and Pins for Depth emulation
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(pin_pulseIn, INPUT);
  pinMode(pin_echoOut, OUTPUT);
  digitalWrite(pin_echoOut, LOW);
  attachInterrupt(digitalPinToInterrupt(pin_pulseIn), echoISR, RISING);

  lcd.blink();
  printDepth();
}


// Reads button pins for depth selection
void loop() {
  int x = analogRead(0);
  if (x < RIGHT_BUTTON_VAL) {
    selectedRelDepth = 1;
  } else if (x < UP_BUTTON_VAL) {
    selectedRelDepth = 10;
  } else if (x < DOWN_BUTTON_VAL) {
    selectedRelDepth = -10;
  } else if (x < LEFT_BUTTON_VAL) {
    selectedRelDepth = -1;
  } else if (x < SELECT_BUTTON_VAL) {
    // Nothing to do
    // GO
  } else {  // Button release
    if (selectedRelDepth != 0) {
      // Ignore Over-/Underflow for now (maybe it's event convenient)
      currentDepth += selectedRelDepth;
      calculatedDepthDelay = ((uint32_t)currentDepth * METER_TO_DELAY) + CORRECTION;
      Serial.print("New Delay for ");
      Serial.print(currentDepth);
      Serial.print(" Meters: ");
      Serial.print(calculatedDepthDelay);
      Serial.println(" us");
      printDepth();
    }
    selectedRelDepth = 0;
  }
}
