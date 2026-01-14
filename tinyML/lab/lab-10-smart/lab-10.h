/***************************************************
 * Lab 9: TinyML-Driven Smart Control & Actuation (ESP32)
 * Full Arduino Code
 *
 * This sketch demonstrates:
 *  1) Streaming sensor sampling (analog sensor on GPIO34)
 *  2) Sliding-window feature extraction
 *  3) INT8 inference (example linear classifier)
 *  4) Post-ML control logic for CPS safety:
 *       - Decision smoothing (majority vote)
 *       - Time confirmation (must persist before ON)
 *       - Minimum ON/OFF hold times (anti-chatter)
 *       - Safe fallback (sensor out-of-range)
 *  5) Actuation using:
 *       - LED (GPIO2) as actuator
 *       - Optional RELAY_PIN (disabled by default)
 *  6) CSV logs to analyze behavior
 *
 * IMPORTANT:
 *  - Replace model parameters (Wi/bi) and X_SCALE/X_ZERO_POINT with your
 *    trained model values for real performance.
 ***************************************************/

#include <Arduino.h>
#include <math.h>
#include <stdint.h>

// ===================== Pins =====================
#define SENSOR_PIN 34
#define LED_PIN     2
// Optional relay pin (uncomment to use external actuator)
// #define RELAY_PIN  26

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS = 20;    // 50 Hz sensor sampling
const uint32_t INFER_PERIOD_MS  = 200;   // model inference every 200 ms

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Features / Model =====================
#define INPUT_SIZE  6
#define NUM_CLASSES 3
// Example class meanings (you can redefine):
// 0 = SAFE/OFF condition
// 1 = SAFE/ON condition
// 2 = ALERT condition (blink)

// ---------------- INT8 model parameters (EXAMPLE) ----------------
const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  { -2,  1,  1, -1,  2,  5 },  // class 0
  {  1, -1, -1,  1, -2, -2 },  // class 1
  {  2, -1, -2, -1,  1,  3 }   // class 2
};
const int32_t bi[NUM_CLASSES] = { 10, 0, -10 };

// Input quantization params (EXAMPLE)
const float X_SCALE = 50.0f;
const int   X_ZERO_POINT = 0;
// ---------------------------------------------------------------

// ===================== Decision Smoothing =====================
#define DECISION_WIN 7
int decisionBuf[DECISION_WIN];
int dHead = 0;
bool decisionFilled = false;

// ===================== CPS Safety / Control Logic =====================
// Time confirmation: require stable "ON-intent" before turning ON
const uint32_t CONFIRM_ON_MS = 1000;   // must request ON for 1 second

// Anti-chatter: once ON, keep ON for at least...
const uint32_t MIN_ON_HOLD_MS  = 1500;

// Anti-chatter: once OFF, keep OFF for at least...
const uint32_t MIN_OFF_HOLD_MS = 800;

// ===================== Control State =====================
bool actuatorState = false;          // OFF initially
uint32_t stateChangedAt = 0;         // last transition time
uint32_t onIntentStart  = 0;         // when smoothed wants ON started
int lastSmoothedClass   = 0;         // for LED patterns + logging

// ===================== Timing State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

// ===================== Helpers: Sliding Window =====================
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

// ===================== Quantization =====================
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

// ===================== Decision Smoothing =====================
void addDecision(int d) {
  decisionBuf[dHead] = d;
  dHead = (dHead + 1) % DECISION_WIN;
  if (dHead == 0) decisionFilled = true;
}

int smoothDecision() {
  int count = decisionFilled ? DECISION_WIN : dHead;
  if (count <= 0) return 0;

  int votes[NUM_CLASSES] = {0};
  for (int i = 0; i < count; i++) {
    int lab = decisionBuf[i];
    if (lab >= 0 && lab < NUM_CLASSES) votes[lab]++;
  }

  int best = 0;
  for (int c = 1; c < NUM_CLASSES; c++)
    if (votes[c] > votes[best]) best = c;

  return best;
}

// ===================== Safe Fallback Checks =====================
bool sensorOutOfRange() {
  // Simple sanity check: ADC should be 0..4095
  // If your sensor can saturate, tune these.
  int lastIdx = (head - 1 + WINDOW_SIZE) % WINDOW_SIZE;
  int rawLast = windowBuf[lastIdx];
  return (rawLast < 0 || rawLast > 4095);
}

// ===================== Post-ML Control Logic (Safe Actuation) =====================
void safeControlUpdate(int smoothedClass) {
  uint32_t now = millis();

  // Interpret classes into "intent"
  // intentOn: desire to turn actuator ON
  // intentAlert: special alert mode (blink) but still safe
  bool intentOn    = (smoothedClass == 1);
  bool intentAlert = (smoothedClass == 2);

  // Safety override: if sensor invalid, force safe OFF
  if (sensorOutOfRange()) {
    actuatorState = false;
    onIntentStart = 0;
    lastSmoothedClass = 0;
    return;
  }

  // Track ON intent persistence
  if (intentOn) {
    if (onIntentStart == 0) onIntentStart = now;
  } else {
    onIntentStart = 0;
  }

  // Anti-chatter holds
  uint32_t heldFor = now - stateChangedAt;

  if (!actuatorState) {
    // Currently OFF: can turn ON only if:
    //  1) intentOn persists for CONFIRM_ON_MS
    //  2) OFF has been held for MIN_OFF_HOLD_MS
    if (intentOn && onIntentStart > 0 &&
        (now - onIntentStart >= CONFIRM_ON_MS) &&
        (heldFor >= MIN_OFF_HOLD_MS)) {
      actuatorState = true;
      stateChangedAt = now;
    }
  } else {
    // Currently ON: turn OFF if:
    //  1) intentOn is not present (or alert wants OFF)
    //  2) ON has been held for MIN_ON_HOLD_MS
    if ((!intentOn) && (heldFor >= MIN_ON_HOLD_MS)) {
      actuatorState = false;
      stateChangedAt = now;
    }
  }

  // Save for output patterns
  lastSmoothedClass = intentAlert ? 2 : (actuatorState ? 1 : 0);
}

// ===================== Actuation Output =====================
void driveOutputs() {
  // LED behavior based on lastSmoothedClass (post-control):
  // 0: OFF, 1: ON, 2: BLINK (alert)
  static bool blinkState = false;
  static uint32_t lastBlink = 0;

  if (lastSmoothedClass == 0) {
    digitalWrite(LED_PIN, LOW);
    // #ifdef RELAY_PIN
    // digitalWrite(RELAY_PIN, LOW);
    // #endif
  } else if (lastSmoothedClass == 1) {
    digitalWrite(LED_PIN, HIGH);
    // #ifdef RELAY_PIN
    // digitalWrite(RELAY_PIN, HIGH);
    // #endif
  } else {
    uint32_t now = millis();
    if (now - lastBlink >= 100) {
      lastBlink = now;
      blinkState = !blinkState;
    }
    digitalWrite(LED_PIN, blinkState ? HIGH : LOW);
    // For a relay, you usually do NOT want rapid blinking.
    // If using RELAY_PIN, consider keeping it OFF during alert.
    // #ifdef RELAY_PIN
    // digitalWrite(RELAY_PIN, LOW);
    // #endif
  }
}

// ===================== Setup =====================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  // #ifdef RELAY_PIN
  // pinMode(RELAY_PIN, OUTPUT);
  // digitalWrite(RELAY_PIN, LOW);
  // #endif

  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;
  for (int i = 0; i < DECISION_WIN; i++) decisionBuf[i] = 0;

  actuatorState = false;
  stateChangedAt = millis();
  onIntentStart = 0;

  Serial.println("==================================================================================");
  Serial.println(" Lab 9: TinyML-Driven Smart Control & Safe Actuation (ESP32)");
  Serial.println(" CSV: time_ms,mean,rms,slope,pred,smoothed,post_class,act_state,infer_us");
  Serial.println("==================================================================================");
  Serial.println("time_ms,mean,rms,slope,pred,smoothed,post_class,act_state,infer_us");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // 1) Sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  // 2) Inference + control update
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    float features[INPUT_SIZE];
    extractFeatures(features);

    int32_t scores[NUM_CLASSES];
    uint32_t t0 = micros();
    int pred = predict_int8(features, scores);
    uint32_t t1 = micros();
    uint32_t infer_us = (uint32_t)(t1 - t0);

    addDecision(pred);
    int smoothed = smoothDecision();

    // Post-ML control logic
    safeControlUpdate(smoothed);

    // Log a compact set of features + decisions
    Serial.print(now);              Serial.print(",");
    Serial.print(features[0], 2);   Serial.print(","); // mean
    Serial.print(features[4], 2);   Serial.print(","); // rms
    Serial.print(features[5], 4);   Serial.print(","); // slope
    Serial.print(pred);             Serial.print(",");
    Serial.print(smoothed);         Serial.print(",");
    Serial.print(lastSmoothedClass);Serial.print(",");
    Serial.print(actuatorState ? 1 : 0); Serial.print(",");
    Serial.println(infer_us);
  }

  // 3) Drive outputs continuously (keeps blink responsive)
  driveOutputs();
}

/*********************** HOW TO ADAPT ************************
- If your model outputs only ON/OFF:
  * Set NUM_CLASSES=2 and simplify post logic:
      intentOn = (smoothed==1)
      no alert class

- For relays/motors:
  * Avoid fast blinking (mechanical wear).
  * Use alert mode to turn relay OFF and blink only an LED/buzzer.

- Tune safety parameters:
  * CONFIRM_ON_MS: longer -> safer (less false ON)
  * MIN_ON_HOLD_MS / MIN_OFF_HOLD_MS: prevents rapid toggling

- Add more safety checks:
  * watch-dog reset
  * sensor disconnect detection (stuck at 0 or 4095)
  * maximum runtime in ON state
***************************************************/

