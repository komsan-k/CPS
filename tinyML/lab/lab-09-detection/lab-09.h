/***************************************************
 * Lab 8: TinyML Anomaly Detection (Normal-Only) on ESP32
 * Full Arduino Code (Streaming + Feature + Baseline Model + Threshold)
 *
 * This sketch:
 *  1) Samples an analog sensor (e.g., LDR on GPIO34) continuously
 *  2) Maintains a sliding window (circular buffer)
 *  3) Extracts features (RMS by default; you can extend)
 *  4) Learns NORMAL baseline statistics during a training phase:
 *       mean_rms, var_rms  (computed online)
 *  5) Computes an anomaly score (z-score style):
 *       score = |rms - mean| / sqrt(var + eps)
 *  6) Triggers alarm (LED blink) when score > THRESHOLD
 *  7) Prints CSV logs for analysis
 *
 * No cloud. No labels required. Designed for CPS fault detection.
 *
 * Hardware:
 *  - ESP32 + analog sensor (LDR divider / potentiometer / vibration sensor)
 *
 * Notes:
 *  - ESP32 ADC is not perfectly linear; this is fine for anomaly detection.
 *  - You must run the device in NORMAL conditions during BASELINE_TRAIN_MS.
 ***************************************************/

#include <Arduino.h>
#include <math.h>

// ===================== Pins =====================
#define SENSOR_PIN 34
#define LED_PIN     2

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS  = 20;    // 50 Hz sampling
const uint32_t FEATURE_PERIOD_MS = 200;   // compute features + score every 200 ms

// ===================== Sliding Window =====================
#define WINDOW_SIZE 25
int windowBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Baseline Training =====================
// Duration to learn "normal" baseline (ms)
const uint32_t BASELINE_TRAIN_MS = 15000; // 15 seconds normal operation

// Online statistics (Welford) for RMS during training
bool baselineReady = false;
uint32_t trainStartMs = 0;
uint32_t baselineN = 0;
double baselineMean = 0.0;
double baselineM2 = 0.0;

// ===================== Anomaly Detection =====================
const float EPS = 1e-6f;
const float THRESHOLD = 3.0f;  // typical z-score threshold (tune this)

// ===================== Timing State =====================
uint32_t lastSampleTime  = 0;
uint32_t lastFeatureTime = 0;

// ===================== LED Alarm State =====================
bool alarmActive = false;

// -------------------- Buffer Helpers --------------------
void addSample(int sample) {
  windowBuf[head] = sample;
  head = (head + 1) % WINDOW_SIZE;
  sampleCount++;
}

void getWindowOrdered(int outArr[WINDOW_SIZE]) {
  int idx = head; // oldest
  for (int i = 0; i < WINDOW_SIZE; i++) {
    outArr[i] = windowBuf[idx];
    idx = (idx + 1) % WINDOW_SIZE;
  }
}

// -------------------- Feature: RMS --------------------
float computeRMS() {
  int w[WINDOW_SIZE];
  getWindowOrdered(w);

  double sumSq = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    double v = (double)w[i];
    sumSq += v * v;
  }
  return (float)sqrt(sumSq / (double)WINDOW_SIZE);
}

// -------------------- Online Baseline Update (Welford) --------------------
void baselineUpdate(double x) {
  baselineN++;
  double delta = x - baselineMean;
  baselineMean += delta / (double)baselineN;
  double delta2 = x - baselineMean;
  baselineM2 += delta * delta2;
}

double baselineVariance() {
  if (baselineN < 2) return 0.0;
  return baselineM2 / (double)(baselineN - 1); // sample variance
}

// -------------------- Alarm LED behavior --------------------
void updateAlarmLED() {
  static bool blinkState = false;
  static uint32_t lastBlink = 0;

  if (!alarmActive) {
    digitalWrite(LED_PIN, LOW);
    return;
  }

  // Blink at 5 Hz when alarm is active
  uint32_t now = millis();
  if (now - lastBlink >= 100) {
    lastBlink = now;
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState ? HIGH : LOW);
  }
}

// ===================== Setup =====================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;

  trainStartMs = millis();

  Serial.println("=======================================================================");
  Serial.println(" Lab 8: TinyML Anomaly Detection (Normal-Only) - ESP32");
  Serial.println(" Baseline training first, then anomaly scoring.");
  Serial.println(" CSV: time_ms,rms,baseline_mean,baseline_std,score,alarm,mode");
  Serial.println("=======================================================================");
  Serial.println("time_ms,rms,baseline_mean,baseline_std,score,alarm,mode");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // 1) Continuous sampling (non-blocking)
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  // 2) Periodic feature + detection
  if (sampleCount >= WINDOW_SIZE && (now - lastFeatureTime >= FEATURE_PERIOD_MS)) {
    lastFeatureTime = now;

    float rms = computeRMS();

    // Baseline training mode
    if (!baselineReady) {
      baselineUpdate((double)rms);

      // Decide if training complete
      if (now - trainStartMs >= BASELINE_TRAIN_MS && baselineN >= 10) {
        baselineReady = true;
      }

      // During training, alarm is off
      alarmActive = false;

      // Log (mode=TRAIN)
      double var = baselineVariance();
      double stdv = sqrt(var);
      Serial.print(now);               Serial.print(",");
      Serial.print(rms, 3);            Serial.print(",");
      Serial.print((float)baselineMean, 3); Serial.print(",");
      Serial.print((float)stdv, 3);    Serial.print(",");
      Serial.print(0.0f, 3);           Serial.print(",");
      Serial.print(alarmActive ? 1 : 0); Serial.print(",");
      Serial.println("TRAIN");
    }
    // Detection mode
    else {
      double var = baselineVariance();
      double stdv = sqrt(var);

      // z-score style anomaly score
      float score = (float)(fabs((double)rms - baselineMean) / (stdv + EPS));

      // Alarm decision
      alarmActive = (score > THRESHOLD);

      // Log (mode=DETECT)
      Serial.print(now);               Serial.print(",");
      Serial.print(rms, 3);            Serial.print(",");
      Serial.print((float)baselineMean, 3); Serial.print(",");
      Serial.print((float)stdv, 3);    Serial.print(",");
      Serial.print(score, 3);          Serial.print(",");
      Serial.print(alarmActive ? 1 : 0); Serial.print(",");
      Serial.println("DETECT");
    }
  }

  // 3) LED alarm behavior (kept responsive)
  updateAlarmLED();
}

/*********************** HOW TO USE / TUNING ************************
1) Power on and keep the system in NORMAL conditions for BASELINE_TRAIN_MS.
   - During TRAIN mode, the system learns baseline RMS statistics.

2) After training, introduce anomalies:
   - sudden changes (cover/uncover LDR rapidly)
   - unusual vibration bursts
   - sensor disconnection (might cause extreme readings)

3) Threshold tuning:
   - THRESHOLD=3 is a common starting point
   - Lower threshold -> more sensitive (more false alarms)
   - Higher threshold -> less sensitive (may miss anomalies)

4) Extensions (recommended):
   - Use multiple features: mean, variance, rms, slope
   - Compute a combined anomaly score (e.g., weighted sum)
   - Implement slow baseline adaptation for concept drift
********************************************************************/

