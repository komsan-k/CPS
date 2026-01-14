/***************************************************
 * Lab 4: Train & Deploy a TinyML Classifier on ESP32
 * Full Arduino Code (Feature-based + Embedded Model)
 *
 * What this sketch does:
 *  1) Samples an analog sensor (e.g., LDR on GPIO34)
 *  2) Maintains a sliding window (circular buffer)
 *  3) Extracts features: mean, min, max, variance, RMS, slope
 *  4) Runs an on-device classifier (embedded weights/bias)
 *  5) Controls LED behavior based on predicted class
 *  6) Prints CSV logs: time_ms,features...,scores...,pred
 *
 * IMPORTANT:
 *  - The weights/bias below are EXAMPLE values so the code compiles.
 *  - Replace them with your trained model parameters from the PC side.
 ***************************************************/

#include <Arduino.h>
#include <math.h>

// ===================== Pin Configuration =====================
#define SENSOR_PIN 34   // ADC input pin (GPIO34 recommended)
#define LED_PIN     2   // Onboard LED (most ESP32 dev boards)

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS  = 20;   // 50 Hz sampling
const uint32_t INFER_PERIOD_MS   = 200;  // infer every 200 ms

// ===================== ADC Reference (approx.) =====================
const float ADC_MAX = 4095.0f;
const float VREF    = 3.3f;

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;                 // next write position
uint32_t sampleCount = 0;

// ===================== Model Definition =====================
// Feature vector size:
#define INPUT_SIZE 6

// Number of classes:
#define NUM_CLASSES 3
// Suggested mapping (you can change it):
// 0 = Dark, 1 = Normal, 2 = Bright

// --------- IMPORTANT: Replace with trained parameters ---------
// Simple linear classifier: score_i = b_i + sum_j (W_i,j * x_j)
// These are example values so the sketch compiles and runs.
const float W[NUM_CLASSES][INPUT_SIZE] = {
  // mean,   min,    max,    var,     rms,    slope
  { -0.002,  0.0005, 0.0005, -0.0001, 0.0010, 0.050 }, // class 0
  {  0.001, -0.0002,-0.0001,  0.0002,-0.0005,-0.020 }, // class 1
  {  0.002, -0.0003,-0.0004, -0.0001, 0.0002, 0.030 }  // class 2
};

const float b[NUM_CLASSES] = { 0.10, 0.00, -0.10 };
// ------------------------------------------------------------

// ===================== State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

// ===================== Sliding Window Helpers =====================
void addSample(int sample) {
  windowBuf[head] = sample;
  head = (head + 1) % WINDOW_SIZE;
  sampleCount++;
}

// Copy circular buffer into ordered array: oldest -> newest
void getWindowOrdered(int outArr[WINDOW_SIZE]) {
  int idx = head; // head points to oldest sample
  for (int i = 0; i < WINDOW_SIZE; i++) {
    outArr[i] = windowBuf[idx];
    idx = (idx + 1) % WINDOW_SIZE;
  }
}

// ===================== Feature Extraction =====================
void extractFeatures(float features[INPUT_SIZE]) {
  int w[WINDOW_SIZE];
  getWindowOrdered(w);

  // Mean, min, max
  long sum = 0;
  int minV = w[0], maxV = w[0];
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += w[i];
    if (w[i] < minV) minV = w[i];
    if (w[i] > maxV) maxV = w[i];
  }
  float mean = sum / (float)WINDOW_SIZE;

  // Variance
  float var = 0.0f;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float d = w[i] - mean;
    var += d * d;
  }
  var /= (float)WINDOW_SIZE;

  // RMS
  double sumSq = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumSq += (double)w[i] * (double)w[i];
  }
  float rms = (float)sqrt(sumSq / (double)WINDOW_SIZE);

  // Slope (trend)
  float slope = (w[WINDOW_SIZE - 1] - w[0]) / (float)WINDOW_SIZE;

  // Pack into feature vector
  features[0] = mean;
  features[1] = (float)minV;
  features[2] = (float)maxV;
  features[3] = var;
  features[4] = rms;
  features[5] = slope;
}

// ===================== TinyML Inference (Linear Classifier) =====================
int tinyml_predict(const float features[INPUT_SIZE], float scoresOut[NUM_CLASSES]) {
  // Compute class scores
  for (int i = 0; i < NUM_CLASSES; i++) {
    float s = b[i];
    for (int j = 0; j < INPUT_SIZE; j++) {
      s += W[i][j] * features[j];
    }
    scoresOut[i] = s;
  }

  // Argmax
  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++) {
    if (scoresOut[i] > scoresOut[best]) best = i;
  }
  return best;
}

// ===================== Actuation =====================
void updateActuator(int label) {
  // Simple, visible behavior using one LED:
  // 0 (Dark)   -> LED OFF
  // 1 (Normal) -> LED ON (steady)
  // 2 (Bright) -> LED BLINK pattern (fast)
  static bool blinkState = false;
  static uint32_t lastBlink = 0;

  if (label == 0) {
    digitalWrite(LED_PIN, LOW);
  } else if (label == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    // blink at 5 Hz
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

  // Initialize buffer
  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;

  Serial.println("===============================================================");
  Serial.println(" Lab 4: Train & Deploy TinyML Classifier (Feature-based) ESP32 ");
  Serial.println(" CSV: time_ms,mean,min,max,var,rms,slope,score0,score1,score2,pred");
  Serial.println("===============================================================");
  Serial.println("time_ms,mean,min,max,variance,rms,slope,score0,score1,score2,pred");
}

// ===================== Main Loop =====================
void loop() {
  uint32_t now = millis();

  // 1) Sample sensor periodically (non-blocking)
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  // 2) Inference periodically (after window filled)
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    // Extract features
    float features[INPUT_SIZE];
    extractFeatures(features);

    // Run inference and measure latency
    float scores[NUM_CLASSES];
    uint32_t t0 = micros();
    int pred = tinyml_predict(features, scores);
    uint32_t t1 = micros();

    // Actuate safely (LED behavior)
    updateActuator(pred);

    // Log CSV
    Serial.print(now);
    Serial.print(",");
    Serial.print(features[0], 2);  Serial.print(","); // mean
    Serial.print(features[1], 0);  Serial.print(","); // min
    Serial.print(features[2], 0);  Serial.print(","); // max
    Serial.print(features[3], 2);  Serial.print(","); // variance
    Serial.print(features[4], 2);  Serial.print(","); // rms
    Serial.print(features[5], 4);  Serial.print(","); // slope
    Serial.print(scores[0], 4);    Serial.print(",");
    Serial.print(scores[1], 4);    Serial.print(",");
    Serial.print(scores[2], 4);    Serial.print(",");
    Serial.print(pred);
    Serial.print("  | infer_us=");
    Serial.println((uint32_t)(t1 - t0));
  }

  // Note: updateActuator() for class 2 blinking runs in main loop via periodic calls.
  // If you want continuous blinking even without inference calls, call updateActuator()
  // every loop with last predicted label (store it in a global).
}

/*********************** HOW TO USE WITH A TRAINED MODEL ************************
1) Collect feature vectors from Lab 3 and label them:
   - Dark (0), Normal (1), Bright (2)

2) Train a simple linear classifier (e.g., logistic regression / linear SVM / small MLP).
   - If you use a different model type, you must change tinyml_predict() accordingly.

3) Export weights (W) and biases (b) and replace the arrays above.
   - W shape: [NUM_CLASSES][INPUT_SIZE]
   - b shape: [NUM_CLASSES]
*******************************************************************************/

