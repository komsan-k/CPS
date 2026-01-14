/***************************************************
 * Lab 6: Streaming Inference with Sliding Windows (ESP32)
 * Full Arduino Code
 *
 * This sketch:
 *  1) Samples an analog sensor continuously (non-blocking)
 *  2) Maintains a sliding window (circular buffer)
 *  3) Runs feature extraction + INT8 inference periodically
 *  4) Implements simple decision smoothing (majority vote)
 *  5) Controls an actuator (LED) safely and stably
 *  6) Prints CSV logs: time_ms,mean,var,rms,slope,int8_pred,smoothed_pred,infer_us
 *
 * IMPORTANT:
 *  - INT8 model parameters below are EXAMPLE values so it compiles.
 *  - Replace Wi/bi and quantization X_SCALE/X_ZERO_POINT with your trained model.
 ***************************************************/

#include <Arduino.h>
#include <math.h>
#include <stdint.h>

// ===================== Pins =====================
#define SENSOR_PIN 34
#define LED_PIN     2

// ===================== Timing =====================
// Sampling runs faster than inference (typical streaming design)
const uint32_t SAMPLE_PERIOD_MS = 20;   // 50 Hz sampling
const uint32_t INFER_PERIOD_MS  = 200;  // inference every 200 ms

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;                // next write position
uint32_t sampleCount = 0;

// ===================== Features / Model =====================
#define INPUT_SIZE  6
#define NUM_CLASSES 3

// ---------------- INT8 model parameters (EXAMPLE) ----------------
// score_i = bi[i] + sum_j( Wi[i][j] * xq[j] )
const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  { -2,  1,  1, -1,  2,  5 },
  {  1, -1, -1,  1, -2, -2 },
  {  2, -1, -2, -1,  1,  3 }
};
const int32_t bi[NUM_CLASSES] = { 10, 0, -10 };

// Input quantization params (EXAMPLE — replace properly)
const float X_SCALE = 50.0f;
const int   X_ZERO_POINT = 0;
// ---------------------------------------------------------------

// ===================== Decision Smoothing =====================
#define DECISION_WIN 5
int decisionBuf[DECISION_WIN];
int dHead = 0;
bool decisionFilled = false;

// ===================== Timing State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

// ===================== LED Control State =====================
int lastSmoothedPred = 0; // used for continuous blink behavior

// ===================== Buffer Helpers =====================
void addSample(int sample) {
  windowBuf[head] = sample;
  head = (head + 1) % WINDOW_SIZE;
  sampleCount++;
}

void getWindowOrdered(int outArr[WINDOW_SIZE]) {
  // oldest -> newest, oldest is at head
  int idx = head;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    outArr[i] = windowBuf[idx];
    idx = (idx + 1) % WINDOW_SIZE;
  }
}

// ===================== Feature Extraction =====================
void extractFeatures(float features[INPUT_SIZE]) {
  int w[WINDOW_SIZE];
  getWindowOrdered(w);

  long sum = 0;
  int minV = w[0], maxV = w[0];
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += w[i];
    if (w[i] < minV) minV = w[i];
    if (w[i] > maxV) maxV = w[i];
  }
  float mean = sum / (float)WINDOW_SIZE;

  float var = 0.0f;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float d = w[i] - mean;
    var += d * d;
  }
  var /= (float)WINDOW_SIZE;

  double sumSq = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) sumSq += (double)w[i] * (double)w[i];
  float rms = (float)sqrt(sumSq / (double)WINDOW_SIZE);

  float slope = (w[WINDOW_SIZE - 1] - w[0]) / (float)WINDOW_SIZE;

  features[0] = mean;
  features[1] = (float)minV;
  features[2] = (float)maxV;
  features[3] = var;
  features[4] = rms;
  features[5] = slope;
}

// ===================== Quantization Helper =====================
static inline int8_t quantize_feature(float x) {
  float q = (x / X_SCALE) + (float)X_ZERO_POINT;
  int qi = (int)lroundf(q);
  if (qi > 127) qi = 127;
  if (qi < -128) qi = -128;
  return (int8_t)qi;
}

// ===================== INT8 Inference =====================
int predict_int8(const float xFloat[INPUT_SIZE], int32_t scoresOut[NUM_CLASSES]) {
  int8_t xq[INPUT_SIZE];
  for (int j = 0; j < INPUT_SIZE; j++) xq[j] = quantize_feature(xFloat[j]);

  for (int i = 0; i < NUM_CLASSES; i++) {
    int32_t acc = bi[i];
    for (int j = 0; j < INPUT_SIZE; j++) {
      acc += (int32_t)Wi[i][j] * (int32_t)xq[j];
    }
    scoresOut[i] = acc;
  }

  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scoresOut[i] > scoresOut[best]) best = i;

  return best;
}

// ===================== Decision Smoothing Helpers =====================
void addDecision(int d) {
  decisionBuf[dHead] = d;
  dHead = (dHead + 1) % DECISION_WIN;
  if (dHead == 0) decisionFilled = true;
}

// Majority vote on labels (works best for binary/low-class tasks)
// For multi-class, this votes the most frequent label in the buffer.
int smoothDecision() {
  int count = decisionFilled ? DECISION_WIN : dHead;
  if (count <= 0) return 0;

  int votes[NUM_CLASSES] = {0};
  for (int i = 0; i < count; i++) {
    int lab = decisionBuf[i];
    if (lab >= 0 && lab < NUM_CLASSES) votes[lab]++;
  }

  int best = 0;
  for (int c = 1; c < NUM_CLASSES; c++) {
    if (votes[c] > votes[best]) best = c;
  }
  return best;
}

// ===================== Actuation =====================
void updateActuator(int label) {
  // 0: LED OFF
  // 1: LED ON
  // 2: LED BLINK (fast)
  static bool blinkState = false;
  static uint32_t lastBlink = 0;

  if (label == 0) {
    digitalWrite(LED_PIN, LOW);
  } else if (label == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    uint32_t now = millis();
    if (now - lastBlink >= 100) {
      lastBlink = now;
      blinkState = !blinkState;
      digitalWrite(LED_PIN, blinkState ? HIGH : LOW);
    }
  }
}

// ===================== Setup =====================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;
  for (int i = 0; i < DECISION_WIN; i++) decisionBuf[i] = 0;

  Serial.println("==========================================================================");
  Serial.println(" Lab 6: Streaming TinyML (Sliding Window + Periodic INT8 Inference) ESP32");
  Serial.println(" CSV: time_ms,mean,variance,rms,slope,pred,smoothed,infer_us");
  Serial.println("==========================================================================");
  Serial.println("time_ms,mean,variance,rms,slope,pred,smoothed,infer_us");
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

  // 2) Periodic inference after window is filled
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    float features[INPUT_SIZE];
    extractFeatures(features);

    int32_t scores[NUM_CLASSES];
    uint32_t t0 = micros();
    int pred = predict_int8(features, scores);
    uint32_t t1 = micros();
    uint32_t infer_us = (uint32_t)(t1 - t0);

    // Decision smoothing
    addDecision(pred);
    int smoothed = smoothDecision();
    lastSmoothedPred = smoothed;

    // Actuation (use smoothed decision for stability)
    updateActuator(smoothed);

    // CSV log (keep it lightweight)
    Serial.print(now);            Serial.print(",");
    Serial.print(features[0], 2); Serial.print(","); // mean
    Serial.print(features[3], 2); Serial.print(","); // variance
    Serial.print(features[4], 2); Serial.print(","); // rms
    Serial.print(features[5], 4); Serial.print(","); // slope
    Serial.print(pred);           Serial.print(",");
    Serial.print(smoothed);       Serial.print(",");
    Serial.println(infer_us);
  }

  // Keep blinking responsive if smoothed label==2
  updateActuator(lastSmoothedPred);
}

/*********************** HOW TO ADAPT FOR YOUR CLASS ************************
- If your task is binary (0/1):
  * Set NUM_CLASSES = 2
  * Provide Wi[2][6], bi[2]
  * smoothDecision() becomes very robust.

- If you want faster response:
  * Reduce WINDOW_SIZE (e.g., 10)
  * Reduce INFER_PERIOD_MS (e.g., 100)
  * Observe increased noise/jitter.

- If you want more stability:
  * Increase WINDOW_SIZE (e.g., 30)
  * Increase DECISION_WIN (e.g., 7)
  * Observe slower response but fewer false toggles.
****************************************************************************/

