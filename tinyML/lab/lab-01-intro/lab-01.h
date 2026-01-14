/***************************************************
 * Lab 0: Introduction to TinyML on ESP32
 * Sensor ? Decision ? Actuator
 *
 * Author: TinyML Lab Series
 ***************************************************/

// ====== Configuration ======
#define LED_PIN 2          // Onboard LED (ESP32)
#define SENSOR_MIN 0
#define SENSOR_MAX 100
#define DECISION_THRESHOLD 50

// ====== Function Prototypes ======
int tinyml_decision(int inputValue);

// ====== Setup ======
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  // Wait for Serial Monitor
  delay(1000);

  Serial.println("=================================");
  Serial.println(" Lab 0: Hello TinyML (ESP32)");
  Serial.println(" Sensor -> Decision -> Actuator ");
  Serial.println("=================================");
}

// ====== Main Loop ======
void loop() {
  // 1. Simulated sensor input
  int sensorValue = random(SENSOR_MIN, SENSOR_MAX);

  // 2. TinyML-like decision (inference)
  int decision = tinyml_decision(sensorValue);

  // 3. Actuator control
  if (decision == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // 4. Serial output (monitoring)
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Decision: ");
  Serial.print(decision);
  Serial.print(" | LED: ");
  Serial.println(decision ? "ON" : "OFF");

  // 5. Delay (controls decision rate)
  delay(500);
}

// ====== TinyML Decision Function ======
/*
 * This function mimics a TinyML model.
 * Later labs will replace this with
 * real ML inference code.
 */
int tinyml_decision(int inputValue) {
  if (inputValue > DECISION_THRESHOLD) {
    return 1;   // Class 1: ON
  } else {
    return 0;   // Class 0: OFF
  }
}

