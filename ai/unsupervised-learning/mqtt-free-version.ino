/*
  ESP32 LDR Classifier without MQTT
  - Reads LDR on GPIO34
  - Smooths signal with moving average
  - Normalizes ADC value
  - Classifies light level: DARK / NORMAL / BRIGHT
  - Controls LED on GPIO2
  - Uses Serial commands instead of MQTT:
      AUTO
      MANUAL_ON
      MANUAL_OFF
*/

#include <Arduino.h>

// ===============================
// Pins
// ===============================
const int LDR_PIN = 34;
const int LED_PIN = 2;

// ===============================
// Learned thresholds from Python
// ===============================
float THRESHOLD1 = 0.275f;
float THRESHOLD2 = 0.655f;

// ===============================
// Normalization
// ===============================
float ADC_MIN = 0.0f;
float ADC_MAX = 4095.0f;

// ===============================
// Smoothing
// ===============================
const int WINDOW_SIZE = 10;
int samples[WINDOW_SIZE];
int sampleIndex = 0;
bool bufferFilled = false;

// ===============================
// State
// ===============================
String currentMode = "AUTO";
String currentState = "UNKNOWN";
bool manualLedState = false;

// ===============================
// Helper functions
// ===============================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float getMovingAverage(int newValue) {
  samples[sampleIndex] = newValue;
  sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;

  if (sampleIndex == 0) bufferFilled = true;

  int count = bufferFilled ? WINDOW_SIZE : sampleIndex;
  if (count == 0) count = 1;

  long sum = 0;
  for (int i = 0; i < count; i++) {
    sum += samples[i];
  }
  return (float)sum / count;
}

String classifyLight(float normValue) {
  if (normValue < THRESHOLD1) return "DARK";
  else if (normValue < THRESHOLD2) return "NORMAL";
  else return "BRIGHT";
}

void applyAutoControl(const String& state) {
  if (state == "DARK") {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "AUTO") {
    currentMode = "AUTO";
    Serial.println("[CMD] Mode set to AUTO");
  }
  else if (cmd == "MANUAL_ON") {
    currentMode = "MANUAL";
    manualLedState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("[CMD] Mode set to MANUAL, LED ON");
  }
  else if (cmd == "MANUAL_OFF") {
    currentMode = "MANUAL";
    manualLedState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("[CMD] Mode set to MANUAL, LED OFF");
  }
  else {
    Serial.println("[CMD] Unknown command");
  }
}

void readSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }
}

void printTelemetry(int adcRaw, float adcSmooth, float normValue, const String& state) {
  Serial.print("{\"adc_raw\":");
  Serial.print(adcRaw);
  Serial.print(",\"adc_smooth\":");
  Serial.print(adcSmooth, 2);
  Serial.print(",\"norm\":");
  Serial.print(normValue, 4);
  Serial.print(",\"state\":\"");
  Serial.print(state);
  Serial.print("\",\"mode\":\"");
  Serial.print(currentMode);
  Serial.print("\",\"led\":");
  Serial.print(digitalRead(LED_PIN) ? 1 : 0);
  Serial.println("}");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  analogReadResolution(12);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    samples[i] = 0;
  }

  Serial.println("ESP32 LDR Classifier Ready");
  Serial.println("Type commands in Serial Monitor:");
  Serial.println("  AUTO");
  Serial.println("  MANUAL_ON");
  Serial.println("  MANUAL_OFF");
}

void loop() {
  readSerialCommand();

  int adcRaw = analogRead(LDR_PIN);
  float adcSmooth = getMovingAverage(adcRaw);
  float normValue = clamp01((adcSmooth - ADC_MIN) / (ADC_MAX - ADC_MIN));
  String state = classifyLight(normValue);
  currentState = state;

  if (currentMode == "AUTO") {
    applyAutoControl(state);
  }

  printTelemetry(adcRaw, adcSmooth, normValue, state);

  Serial.print("ADC Raw: ");
  Serial.print(adcRaw);
  Serial.print(" | Smooth: ");
  Serial.print(adcSmooth, 2);
  Serial.print(" | Norm: ");
  Serial.print(normValue, 4);
  Serial.print(" | State: ");
  Serial.print(state);
  Serial.print(" | Mode: ");
  Serial.println(currentMode);

  delay(1000);
}
