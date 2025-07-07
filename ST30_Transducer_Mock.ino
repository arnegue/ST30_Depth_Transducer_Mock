#include <LiquidCrystal.h>

// Analog values for LCD buttons
#define RIGHT_BUTTON_VAL 60
#define UP_BUTTON_VAL 200
#define DOWN_BUTTON_VAL 400
#define LEFT_BUTTON_VAL 600
#define SELECT_BUTTON_VAL 800

// LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
LiquidCrystal lcd(pin_RS, pin_EN, pin_d4, pin_d5, pin_d6, pin_d7);

// Depth selection and display
uint8_t currentDepth = 0;
int8_t selectedRelDepth = 0;
unsigned int calculatedDepthDelay = 0;

const unsigned int METER_TO_DELAY = 1333;              // Rough estimate
const unsigned int CORRECTION = 0.7 * METER_TO_DELAY;  // Some calibration (maybe to to some electronics delay and/or slow arduino[libraries])
// Pins to ST30
const int pin_pulseIn = 2;  // InPin, triggers an interrupt when ST30 generates a pulse to transducer
const int pin_echoOut = 3;  // OutPin, Toggles high to emulate echo of pulse

// Print depth to LCD
void printDepth() {
  lcd.clear();
  lcd.print("Depth: ");
  lcd.print(currentDepth);
  lcd.print(" m");
}


// Emulates echo to ST30
void echoISR() {
  delayMicroseconds(calculatedDepthDelay);
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
    Serial.println("R");
    selectedRelDepth = 1;
  } else if (x < UP_BUTTON_VAL) {
    Serial.println("U");
    selectedRelDepth = 10;
  } else if (x < DOWN_BUTTON_VAL) {
    Serial.println("D");
    selectedRelDepth = -10;
  } else if (x < LEFT_BUTTON_VAL) {
    Serial.println("L");
    selectedRelDepth = -1;
  } else if (x < SELECT_BUTTON_VAL) {
    Serial.println("S");
    lcd.noBlink();
    // GO
  } else {  // Button release
    if (selectedRelDepth != 0) {
      lcd.blink();
      Serial.println("Rel");

      // Ignore Over-/Underflow for now (maybe it's event convenient)
      currentDepth += selectedRelDepth;
      calculatedDepthDelay = (currentDepth * METER_TO_DELAY) + CORRECTION;
      Serial.print("New Delay for ");
      Serial.print(currentDepth);
      Serial.print(" Meters: ");
      Serial.println(calculatedDepthDelay);
      printDepth();
    }
    selectedRelDepth = 0;
  }
}
