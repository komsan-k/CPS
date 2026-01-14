/***************************************************
 * Lab 5: Model Quantization (INT8) + Memory/Latency Compare
 * Full Arduino Code (ESP32)
 *
 * This sketch:
 *  1) Samples an analog sensor (e.g., LDR on GPIO34)
 *  2) Maintains a sliding window
 *  3) Extracts features: mean, min, max, variance, RMS, slope
 *  4) Runs BOTH:
 *       - Float32 linear classifier (baseline)
 *       - INT8 quantized linear classifier (optimized)
 *  5) Measures inference latency (microseconds)
 *  6) Prints CSV logs for comparison
 *
 * IMPORTANT:
 *  - The model parameters below are EXAMPLE values to compile/run.
 *  - Replace them with your trained parameters.
 *  - Quantization parameters (scale/zero_point) are also example values.
 *
 * Notes for best practice:
 *  - Quantize your trained weights and features using a consistent method.
 *  - In real TinyML, use TFLM INT8 with per-tensor/per-channel scales.
 ***************************************************/

#include <Arduino.h>
#include <math.h>
#include <stdint.h>

// ===================== Pin Configuration =====================
#define SENSOR_PIN 34
#define LED_PIN     2

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS = 20;   // 50 Hz
const uint32_t INFER_PERIOD_MS  = 200;  // infer every 200 ms

// ===================== ADC Reference (approx.) =====================
const float ADC_MAX = 4095.0f;
const float VREF    = 3.3f;

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Feature / Model Sizes =====================
#define INPUT_SIZE  6
#define NUM_CLASSES 3

// ===================== FLOAT MODEL (Baseline) =====================
// Example weights/bias (REPLACE with your trained float model)
const float Wf[NUM_CLASSES][INPUT_SIZE] = {
  { -0.0020f,  0.0005f,  0.0005f, -0.0001f,  0.0010f,  0.0500f },
  {  0.0010f, -0.0002f, -0.0001f,  0.0002f, -0.0005f, -0.0200f },
  {  0.0020f, -0.0003f, -0.0004f, -0.0001f,  0.0002f,  0.0300f }
};
const float bf[NUM_CLASSES] = { 0.10f, 0.00f, -0.10f };

// ===================== INT8 MODEL (Quantized) =====================
// Example int8 weights/bias (REPLACE with quantized parameters)
// Convention here (simple demo):
//   score_i_int32 = bias_i_int32 + sum_j (w_int8[i][j] * x_int8[j])
// Argmax on int32 scores.
const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  { -2,  1,  1, -1,  2,  5 },
  {  1, -1, -1,  1, -2, -2 },
  {  2, -1, -2, -1,  1,  3 }
};
// Bias often quantized to int32; here example values:
const int32_t bi[NUM_CLASSES] = { 10, 0, -10 };

// Quantization parameters for FEATURE vector (demo; replace properly!)
//
// We quantize features as:
//   x_int8 = clamp(round(x_float / X_SCALE) + X_ZERO_POINT, -128..127)
// Choose X_SCALE so typical feature range maps nicely into int8.
//
// NOTE: In real pipelines you typically normalize features first.
const float X_SCALE = 50.0f;    // example
const int   X_ZERO_POINT = 0;   // example

// ===================== State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;
int lastPredInt8 = 0; // for LED control

// ===================== Sliding Window Helpers =====================
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

// ===================== Float Inference =====================
int predict_float(const float x[INPUT_SIZE], float scoresOut[NUM_CLASSES]) {
  for (int i = 0; i < NUM_CLASSES; i++) {
    float s = bf[i];
    for (int j = 0; j < INPUT_SIZE; j++) s += Wf[i][j] * x[j];
    scoresOut[i] = s;
  }
  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scoresOut[i] > scoresOut[best]) best = i;
  return best;
}

// ===================== Quantization Helper =====================
static inline int8_t quantize_feature(float x) {
  // round(x / X_SCALE) + X_ZERO_POINT
  float q = (x / X_SCALE) + (float)X_ZERO_POINT;
  int qi = (int)lroundf(q);

  if (qi > 127) qi = 127;
  if (qi < -128) qi = -128;
  return (int8_t)qi;
}

// ===================== INT8 Inference =====================
int predict_int8(const float xFloat[INPUT_SIZE], int32_t scoresOut[NUM_CLASSES]) {
  // Quantize input features
  int8_t xq[INPUT_SIZE];
  for (int j = 0; j < INPUT_SIZE; j++) xq[j] = quantize_feature(xFloat[j]);

  // Integer accumulation
  for (int i = 0; i < NUM_CLASSES; i++) {
    int32_t acc = bi[i];
    for (int j = 0; j < INPUT_SIZE; j++) {
      acc += (int32_t)Wi[i][j] * (int32_t)xq[j];
    }
    scoresOut[i] = acc;
  }

  // Argmax
  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scoresOut[i] > scoresOut[best]) best = i;

  return best;
}

// ===================== Actuation (uses INT8 prediction) =====================
void updateActuator(int label) {
  // 0 = LED OFF
  // 1 = LED ON
  // 2 = LED BLINK
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

  Serial.println("======================================================================");
  Serial.println(" Lab 5: Float vs INT8 Quantized Inference (ESP32) + Latency Compare");
  Serial.println(" CSV: time_ms,mean,min,max,var,rms,slope,float_pred,int8_pred,float_us,int8_us");
  Serial.println("======================================================================");
  Serial.println("time_ms,mean,min,max,variance,rms,slope,float_pred,int8_pred,float_us,int8_us");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // Sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  // Inference
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    float features[INPUT_SIZE];
    extractFeatures(features);

    // Float inference timing
    float fScores[NUM_CLASSES];
    uint32_t t0 = micros();
    int predF = predict_float(features, fScores);
    uint32_t t1 = micros();
    uint32_t float_us = (uint32_t)(t1 - t0);

    // INT8 inference timing
    int32_t iScores[NUM_CLASSES];
    uint32_t t2 = micros();
    int predI = predict_int8(features, iScores);
    uint32_t t3 = micros();
    uint32_t int8_us = (uint32_t)(t3 - t2);

    lastPredInt8 = predI;
    updateActuator(lastPredInt8);

    // CSV log
    Serial.print(now);                Serial.print(",");
    Serial.print(features[0], 2);     Serial.print(",");
    Serial.print((int)features[1]);   Serial.print(",");
    Serial.print((int)features[2]);   Serial.print(",");
    Serial.print(features[3], 2);     Serial.print(",");
    Serial.print(features[4], 2);     Serial.print(",");
    Serial.print(features[5], 4);     Serial.print(",");
    Serial.print(predF);             Serial.print(",");
    Serial.print(predI);             Serial.print(",");
    Serial.print(float_us);          Serial.print(",");
    Serial.println(int8_us);
  }

  // Keep blinking responsive when label==2
  updateActuator(lastPredInt8);
}

/*********************** HOW TO USE WITH YOUR TRAINED MODEL ************************
A) Float model:
   - Replace Wf and bf with your trained float weights/bias.

B) INT8 model:
   - Quantize the SAME trained model weights to int8:
        Wi = round(Wf / W_SCALE)  (or per-channel)
     and biases to int32:
        bi = round(bf / (W_SCALE * X_SCALE))
     (exact formula depends on your quantization scheme)

C) Input quantization:
   - Choose X_SCALE and X_ZERO_POINT based on typical feature ranges.
   - Ideally normalize features first (e.g., z-score), then quantize.

D) Compare results:
   - Look at float_pred vs int8_pred consistency
   - Compare float_us vs int8_us for speed improvement
   - Check Arduino compile output for Flash/RAM usage
***********************************************************************************/

