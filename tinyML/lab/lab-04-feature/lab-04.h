/***************************************************
 * Lab 3: Feature Engineering on ESP32 (TinyML Prep)
 * ESP32 - TinyML CPS Lab Series
 *
 * Pipeline:
 *  - Sample analog sensor periodically
 *  - Maintain sliding window (circular buffer)
 *  - Compute features:
 *      mean, min, max, variance, RMS, slope
 *  - Print feature vector in CSV format
 ***************************************************/

#include <Arduino.h>
#include <math.h>

// ====== Pin Configuration ======
#define SENSOR_PIN 34       // ADC input pin (GPIO34 recommended)
#define LED_PIN    2        // Onboard LED (optional)

// ====== Sampling / Inference Timing ======
const uint32_t SAMPLE_PERIOD_MS = 20;   // 50 Hz sampling
const uint32_t FEATURE_PERIOD_MS = 200; // feature output every 200 ms

// ====== ADC Reference (approx.) ======
const float ADC_MAX = 4095.0;
const float VREF    = 3.3;

// ====== Sliding Window ======
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;               // next write position
uint32_t sampleCount = 0;   // total samples seen

// ====== Timing State ======
uint32_t lastSampleTime = 0;
uint32_t lastFeatureTime = 0;

// ---------- Helpers: add sample to circular buffer ----------
void addSample(int sample) {
  windowBuf[head] = sample;
  head = (head + 1) % WINDOW_SIZE;
  sampleCount++;
}

// ---------- Helpers: copy circular buffer into ordered array (oldest->newest) ----------
void getWindowOrdered(int outArr[WINDOW_SIZE]) {
  // Oldest sample is at head (because head points to next write)
  int idx = head;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    outArr[i] = windowBuf[idx];
    idx = (idx + 1) % WINDOW_SIZE;
  }
}

// ---------- Feature Extraction ----------
void extractFeatures(float &mean, int &minV, int &maxV, float &var, float &rms, float &slope) {
  int w[WINDOW_SIZE];
  getWindowOrdered(w);

  // Mean, Min, Max
  long sum = 0;
  minV = w[0];
  maxV = w[0];
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += w[i];
    if (w[i] < minV) minV = w[i];
    if (w[i] > maxV) maxV = w[i];
  }
  mean = sum / (float)WINDOW_SIZE;

  // Variance
  float acc = 0.0f;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float d = w[i] - mean;
    acc += d * d;
  }
  var = acc / (float)WINDOW_SIZE;

  // RMS
  double sumSq = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumSq += (double)w[i] * (double)w[i];
  }
  rms = (float)sqrt(sumSq / (double)WINDOW_SIZE);

  // Slope (trend)
  // (newest - oldest) / WINDOW_SIZE
  slope = (w[WINDOW_SIZE - 1] - w[0]) / (float)WINDOW_SIZE;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  // Initialize buffer with zeros to avoid garbage
  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;

  Serial.println("==============================================================");
  Serial.println(" Lab 3: Sliding Window Feature Engineering (TinyML Features)");
  Serial.println(" CSV: time_ms,mean,min,max,variance,rms,slope,raw_last,voltage");
  Serial.println("==============================================================");
  Serial.println("time_ms,mean,min,max,variance,rms,slope,raw_last,voltage_last");
}

void loop() {
  uint32_t now = millis();

  // 1) Periodic sampling (non-blocking)
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;

    int raw = analogRead(SENSOR_PIN);
    addSample(raw);

    // Optional: LED indicator based on latest sample
    digitalWrite(LED_PIN, (raw > 2500) ? HIGH : LOW);
  }

  // 2) Feature output (only after buffer is "ready")
  if (sampleCount >= WINDOW_SIZE && (now - lastFeatureTime >= FEATURE_PERIOD_MS)) {
    lastFeatureTime = now;

    float mean, var, rms, slope;
    int minV, maxV;

    extractFeatures(mean, minV, maxV, var, rms, slope);

    // Most recent sample is at position (head - 1)
    int lastIdx = (head - 1 + WINDOW_SIZE) % WINDOW_SIZE;
    int rawLast = windowBuf[lastIdx];
    float voltageLast = (rawLast / ADC_MAX) * VREF;

    // 3) Print feature vector in CSV format
    Serial.print(now);
    Serial.print(",");
    Serial.print(mean, 2);
    Serial.print(",");
    Serial.print(minV);
    Serial.print(",");
    Serial.print(maxV);
    Serial.print(",");
    Serial.print(var, 2);
    Serial.print(",");
    Serial.print(rms, 2);
    Serial.print(",");
    Serial.print(slope, 4);
    Serial.print(",");
    Serial.print(rawLast);
    Serial.print(",");
    Serial.println(voltageLast, 3);
  }
}

