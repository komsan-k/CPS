/***************************************************
 * Lab 2: Analog Sensor Interfacing & Data Logging
 * ESP32 - TinyML CPS Lab Series
 *
 * Features:
 *  - ADC reading (0..4095)
 *  - Voltage conversion (approx.)
 *  - Moving average filtering
 *  - CSV logging: time_ms,raw,filtered,voltage
 ***************************************************/

#include <Arduino.h>

// ====== Pin Configuration ======
#define SENSOR_PIN 34          // ADC input pin (GPIO34 recommended)
#define LED_PIN    2           // Onboard LED (optional indicator)

// ====== Sampling Configuration ======
const uint32_t SAMPLE_PERIOD_MS = 50; // 20 Hz sampling

// ====== ADC Reference ======
// ESP32 ADC is not perfectly linear; this is a practical approximation.
const float ADC_MAX = 4095.0;
const float VREF    = 3.3;

// ====== Moving Average Filter ======
#define MA_WINDOW 10
int maBuffer[MA_WINDOW];
int maIndex = 0;
bool maFilled = false;

// ====== State ======
uint32_t lastSampleTime = 0;

// -------- Moving Average Update --------
float movingAverage(int newValue) {
  maBuffer[maIndex++] = newValue;
  if (maIndex >= MA_WINDOW) {
    maIndex = 0;
    maFilled = true;
  }

  int count = maFilled ? MA_WINDOW : maIndex;
  if (count == 0) count = 1;

  long sum = 0;
  for (int i = 0; i < count; i++) sum += maBuffer[i];
  return sum / (float)count;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  // Initialize buffer
  for (int i = 0; i < MA_WINDOW; i++) maBuffer[i] = 0;

  Serial.println("===============================================");
  Serial.println(" Lab 2: Analog Sensor + ADC + Filtering + Log ");
  Serial.println(" Output CSV: time_ms,raw,filtered,voltage(V)  ");
  Serial.println("===============================================");

  // CSV header (useful for copy/paste to file)
  Serial.println("time_ms,raw_adc,filtered_adc,voltage_v");
}

void loop() {
  uint32_t now = millis();

  // Non-blocking periodic sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;

    // 1) Read raw ADC
    int raw = analogRead(SENSOR_PIN);

    // 2) Filter (moving average)
    float filtered = movingAverage(raw);

    // 3) Convert to voltage (approx.)
    float voltage = (filtered / ADC_MAX) * VREF;

    // 4) Optional indicator: LED ON if bright/high (adjust threshold)
    if (filtered > 2500) digitalWrite(LED_PIN, HIGH);
    else digitalWrite(LED_PIN, LOW);

    // 5) CSV logging
    Serial.print(now);
    Serial.print(",");
    Serial.print(raw);
    Serial.print(",");
    Serial.print(filtered, 2);
    Serial.print(",");
    Serial.println(voltage, 3);
  }
}

