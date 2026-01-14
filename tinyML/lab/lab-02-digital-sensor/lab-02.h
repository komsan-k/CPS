/***************************************************
 * Lab 1: Digital Sensor Interfacing with Debounce
 * ESP32 - TinyML CPS Lab Series
 *
 * Sensor ? Decision ? Actuator
 ***************************************************/

// ====== Pin Configuration ======
#define BUTTON_PIN 15      // Digital input (button / PIR)
#define LED_PIN     2      // Onboard LED (ESP32)

// ====== Debounce Parameters ======
#define DEBOUNCE_TIME 50   // milliseconds

// ====== Global Variables ======
int lastButtonState = HIGH;        // previous raw reading
int stableButtonState = HIGH;     // debounced state
unsigned long lastDebounceTime = 0;

// ====== Setup ======
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); // internal pull-up
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println(" Lab 1: Digital Sensor Interface ");
  Serial.println(" Reliable Sensor -> Actuator ");
  Serial.println("=================================");
}

// ====== Main Loop ======
void loop() {
  // 1. Read raw sensor value
  int reading = digitalRead(BUTTON_PIN);

  // 2. Check for state change (noise or real)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // reset debounce timer
  }

  // 3. Confirm stable state after debounce time
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // 4. Actuation logic (active LOW)
      if (stableButtonState == LOW) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Button PRESSED  -> LED ON");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Button RELEASED -> LED OFF");
      }
    }
  }

  // 5. Save state for next loop
  lastButtonState = reading;
}

