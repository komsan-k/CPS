/***************************************************
 * Lab 7: Activity Recognition (Motion/Vibration) on ESP32
 * Full Arduino Code (MPU6050 via I2C + Streaming Features + INT8 Inference)
 *
 * What this sketch does:
 *  1) Reads MPU6050 accelerometer (ax, ay, az) over I2C
 *  2) Maintains sliding windows for x/y/z (circular buffers)
 *  3) Extracts compact features (RMS_x, RMS_y, RMS_z, MagRMS)
 *  4) Runs an INT8 linear classifier (example parameters included)
 *  5) Applies decision smoothing (majority vote)
 *  6) Controls an actuator (LED) based on activity:
 *       0=Still (OFF), 1=Move (ON), 2=Shake (BLINK)
 *  7) Prints CSV logs
 *
 * IMPORTANT:
 *  - This uses a minimal MPU6050 driver (no external libraries required).
 *  - You MUST replace model parameters (Wi/bi) and quantization params
 *    (X_SCALE/X_ZERO_POINT) with your trained model for real accuracy.
 *
 * Wiring (typical):
 *  - MPU6050 VCC  -> 3.3V (or 5V if your module supports it; prefer 3.3V)
 *  - MPU6050 GND  -> GND
 *  - MPU6050 SDA  -> ESP32 GPIO21
 *  - MPU6050 SCL  -> ESP32 GPIO22
 ***************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <stdint.h>

// ===================== I2C / MPU6050 =====================
#define MPU6050_ADDR 0x68

// MPU6050 registers
#define REG_PWR_MGMT_1   0x6B
#define REG_ACCEL_XOUT_H 0x3B

// ===================== Pins =====================
#define LED_PIN 2

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS = 20;   // 50 Hz accel sampling
const uint32_t INFER_PERIOD_MS  = 200;  // inference every 200 ms

// ===================== Sliding Window =====================
#define WINDOW_SIZE 25
int16_t axBuf[WINDOW_SIZE];
int16_t ayBuf[WINDOW_SIZE];
int16_t azBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Features / Model =====================
#define INPUT_SIZE  4   // RMSx, RMSy, RMSz, MagRMS
#define NUM_CLASSES 3   // 0=Still, 1=Move, 2=Shake

// ---------------- INT8 model parameters (EXAMPLE) ----------------
// score_i = bi[i] + sum_j( Wi[i][j] * xq[j] )
const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  // RMSx  RMSy  RMSz  MagRMS
  {  -2,  -2,  -2,   -3 },  // class 0: Still
  {   1,   1,   1,    2 },  // class 1: Move
  {   3,   3,   3,    4 }   // class 2: Shake
};
const int32_t bi[NUM_CLASSES] = { 30, 0, -20 };

// Input quantization params (EXAMPLE; replace properly)
const float X_SCALE = 2000.0f;   // maps RMS range to int8-ish range
const int   X_ZERO_POINT = 0;
// ---------------------------------------------------------------

// ===================== Decision Smoothing =====================
#define DECISION_WIN 7
int decisionBuf[DECISION_WIN];
int dHead = 0;
bool decisionFilled = false;

// ===================== Timing State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

// ===================== LED State =====================
int lastSmoothedPred = 0;

// -------------------- I2C helpers --------------------
void i2cWriteByte(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission(true);
}

bool i2cReadBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false; // repeated start
  uint8_t read = Wire.requestFrom(MPU6050_ADDR, len, (uint8_t)true);
  if (read != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

// -------------------- MPU init + read --------------------
bool mpuInit() {
  Wire.begin(); // default SDA=21, SCL=22 on ESP32
  Wire.setClock(400000);

  // Wake up MPU6050 (clear sleep bit)
  i2cWriteByte(REG_PWR_MGMT_1, 0x00);
  delay(50);

  // Simple check: read PWR_MGMT_1 back
  uint8_t v = 0xFF;
  if (!i2cReadBytes(REG_PWR_MGMT_1, &v, 1)) return false;
  return true;
}

bool readAccelRaw(int16_t &ax, int16_t &ay, int16_t &az) {
  uint8_t data[6];
  if (!i2cReadBytes(REG_ACCEL_XOUT_H, data, 6)) return false;

  ax = (int16_t)((data[0] << 8) | data[1]);
  ay = (int16_t)((data[2] << 8) | data[3]);
  az = (int16_t)((data[4] << 8) | data[5]);
  return true;
}

// ===================== Sliding Window =====================
void addAccelSample(int16_t ax, int16_t ay, int16_t az) {
  axBuf[head] = ax;
  ayBuf[head] = ay;
  azBuf[head] = az;
  head = (head + 1) % WINDOW_SIZE;
  sampleCount++;
}

// Copy circular buffer into ordered array (oldest->newest)
void getOrdered(const int16_t *src, int16_t *dst) {
  int idx = head; // oldest
  for (int i = 0; i < WINDOW_SIZE; i++) {
    dst[i] = src[idx];
    idx = (idx + 1) % WINDOW_SIZE;
  }
}

// ===================== Feature Extraction =====================
float rmsAxis(const int16_t *buf) {
  int16_t w[WINDOW_SIZE];
  getOrdered(buf, w);

  double sumSq = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    double v = (double)w[i];
    sumSq += v * v;
  }
  return (float)sqrt(sumSq / (double)WINDOW_SIZE);
}

void extractFeatures(float features[INPUT_SIZE]) {
  float rmsX = rmsAxis(axBuf);
  float rmsY = rmsAxis(ayBuf);
  float rmsZ = rmsAxis(azBuf);
  float magRMS = sqrtf(rmsX * rmsX + rmsY * rmsY + rmsZ * rmsZ);

  features[0] = rmsX;
  features[1] = rmsY;
  features[2] = rmsZ;
  features[3] = magRMS;
}

// ===================== Quantization + INT8 Inference =====================
static inline int8_t quantize_feature(float x) {
  float q = (x / X_SCALE) + (float)X_ZERO_POINT;
  int qi = (int)lroundf(q);
  if (qi > 127) qi = 127;
  if (qi < -128) qi = -128;
  return (int8_t)qi;
}

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

// ===================== Actuation =====================
void updateActuator(int label) {
  // 0 = Still  -> LED OFF
  // 1 = Move   -> LED ON
  // 2 = Shake  -> LED BLINK
  static bool blinkState = false;
  static uint32_t lastBlink = 0;

  if (label == 0) {
    digitalWrite(LED_PIN, LOW);
  } else if (label == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    uint32_t now = millis();
    if (now - lastBlink >= 100) { // 5 Hz blink
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

  for (int i = 0; i < WINDOW_SIZE; i++) {
    axBuf[i] = ayBuf[i] = azBuf[i] = 0;
  }
  for (int i = 0; i < DECISION_WIN; i++) decisionBuf[i] = 0;

  Serial.println("=====================================================================");
  Serial.println(" Lab 7: Activity Recognition (MPU6050) + Streaming Features + INT8 ML");
  Serial.println(" CSV: time_ms,rmsx,rmsy,rmsz,magrms,pred,smoothed,infer_us");
  Serial.println("=====================================================================");
  Serial.println("time_ms,rmsx,rmsy,rmsz,magrMS,pred,smoothed,infer_us");

  if (!mpuInit()) {
    Serial.println("ERROR: MPU6050 not detected. Check wiring (SDA=21, SCL=22) and power.");
    while (1) { delay(1000); }
  }
  Serial.println("MPU6050 OK");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // 1) Sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;

    int16_t ax, ay, az;
    if (readAccelRaw(ax, ay, az)) {
      addAccelSample(ax, ay, az);
    } else {
      // If read fails, keep last samples; could also add zeros or skip.
      // Keep system alive.
    }
  }

  // 2) Inference (after window filled)
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
    lastSmoothedPred = smoothed;

    updateActuator(smoothed);

    // CSV log
    Serial.print(now);              Serial.print(",");
    Serial.print(features[0], 2);   Serial.print(",");
    Serial.print(features[1], 2);   Serial.print(",");
    Serial.print(features[2], 2);   Serial.print(",");
    Serial.print(features[3], 2);   Serial.print(",");
    Serial.print(pred);             Serial.print(",");
    Serial.print(smoothed);         Serial.print(",");
    Serial.println(infer_us);
  }

  // Keep blink responsive
  updateActuator(lastSmoothedPred);
}

/*********************** NOTES FOR REAL TRAINING ************************
1) Dataset collection:
   - Log rmsx,rmsy,rmsz,magrms and add a label:
       0=still, 1=move, 2=shake
   - Collect multiple sessions per class.

2) Training:
   - Train a small classifier (linear/logistic regression or tiny MLP)
   - Export weights/bias.
   - If you use an MLP, update inference code accordingly.

3) Quantization:
   - Normalize/standardize features on PC side.
   - Choose X_SCALE / X_ZERO_POINT for the normalized feature range.
   - Quantize weights + bias consistently with your method.
***********************************************************************/

