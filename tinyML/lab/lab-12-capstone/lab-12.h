/***************************************************
 * Lab 11: TinyML CPS Capstone (ESP32)
 * Full Arduino Code (End-to-End System)
 *
 * CAPSTONE FEATURES (all-in-one):
 *  1) Continuous analog sensing (ADC) + sliding window
 *  2) Feature extraction (mean, min, max, variance, RMS, slope)
 *  3) INT8 TinyML inference (example linear model)
 *  4) Safe control logic:
 *       - Decision smoothing (majority vote)
 *       - Time confirmation + minimum hold times (anti-chatter)
 *       - Sensor sanity checks + safe fallback
 *  5) Edge-to-cloud monitoring via MQTT (optional but included)
 *       - Sends ONLY semantic data (pred, confidence proxy, anomaly score proxy)
 *       - CPS continues working even offline
 *  6) Optional "Normal-only" anomaly score (z-score) using confidence proxy
 *
 * REQUIRED LIBRARIES:
 *  - PubSubClient by Nick O'Leary (Library Manager)
 *  - WiFi.h is built-in on ESP32 core
 *
 * IMPORTANT:
 *  - Replace Wi-Fi/MQTT settings.
 *  - Replace model parameters (Wi/bi) and quantization params (X_SCALE/ZP)
 *    with your trained model for real accuracy.
 *
 * HARDWARE:
 *  - ESP32 Dev board
 *  - Analog sensor (LDR divider / pot / vibration sensor) on GPIO34
 *  - LED on GPIO2 (or external)
 *
 * OPTIONAL:
 *  - Relay/Motor driver pin: ACT_PIN (commented)
 ***************************************************/

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>
#include <stdint.h>

// ===================== USER CONFIG: Wi-Fi =====================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ===================== USER CONFIG: MQTT =====================
const char* MQTT_HOST = "broker.hivemq.com"; // or your local broker IP
const uint16_t MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "esp32-tinyml-capstone-lab11";
const char* TOPIC_TELEM    = "tinyml/esp32/lab11/telemetry";
const char* TOPIC_STATUS   = "tinyml/esp32/lab11/status";

// ===================== Pins =====================
#define SENSOR_PIN 34
#define LED_PIN     2
// Optional actuator (relay/motor driver) - enable carefully
// #define ACT_PIN   26

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS = 20;    // 50 Hz sensing
const uint32_t INFER_PERIOD_MS  = 200;   // 5 Hz inference
const uint32_t MQTT_PUB_MS      = 500;   // 2 Hz publish
const uint32_t WIFI_RETRY_MS    = 5000;
const uint32_t MQTT_RETRY_MS    = 5000;

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Feature / Model =====================
#define INPUT_SIZE  6
#define NUM_CLASSES 3
// Example class meanings:
// 0 = OFF / Normal
// 1 = ON  / Active
// 2 = ALERT (blink)

const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  { -2,  1,  1, -1,  2,  5 },  // example only
  {  1, -1, -1,  1, -2, -2 },
  {  2, -1, -2, -1,  1,  3 }
};
const int32_t bi[NUM_CLASSES] = { 10, 0, -10 };

// Quantization (EXAMPLE)
const float X_SCALE = 50.0f;
const int   X_ZERO_POINT = 0;

// ===================== Decision Smoothing =====================
#define DECISION_WIN 7
int decisionBuf[DECISION_WIN];
int dHead = 0;
bool decisionFilled = false;

// ===================== Safe Control Parameters =====================
const uint32_t CONFIRM_ON_MS     = 800;   // stable ON intent
const uint32_t CONFIRM_OFF_MS    = 800;   // stable OFF intent
const uint32_t MIN_ON_HOLD_MS    = 1500;  // keep ON for at least
const uint32_t MIN_OFF_HOLD_MS   = 800;   // keep OFF for at least

// ===================== State =====================
bool actuatorState = false;
uint32_t stateChangedAt = 0;

uint32_t candidateStart = 0;
int candidateTarget = -1; // -1 none, 0 OFF, 1 ON

int lastPred = 0;
int lastStableLabel = 0;
int lastPostLabel = 0; // post-control label used for LED pattern

int32_t lastScores[NUM_CLASSES] = {0};
uint32_t lastInferUs = 0;

// Simple "anomaly-like" score based on confidence margin drift
// (Optional concept: normal-only drift indication)
double confMean = 0.0;
double confM2   = 0.0;
uint32_t confN  = 0;
bool confBaselineReady = false;
const uint32_t CONF_BASELINE_MS = 15000; // learn confidence baseline 15s
uint32_t confBaselineStart = 0;
const float CONF_ANOM_THRESH = 3.0f;     // z-score threshold
bool confAnomaly = false;

// ===================== Connectivity =====================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

uint32_t lastWiFiAttempt = 0;
uint32_t lastMQTTAttempt = 0;
uint32_t lastMQTTPublish = 0;

uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

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
    for (int j = 0; j < INPUT_SIZE; j++) acc += (int32_t)Wi[i][j] * (int32_t)xq[j];
    scoresOut[i] = acc;
  }

  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scoresOut[i] > scoresOut[best]) best = i;
  return best;
}

// ===================== Confidence Proxy =====================
float confidence_margin(const int32_t scores[NUM_CLASSES], int bestIdx) {
  int32_t best = scores[bestIdx];
  int32_t second = INT32_MIN;
  for (int i = 0; i < NUM_CLASSES; i++) {
    if (i == bestIdx) continue;
    if (scores[i] > second) second = scores[i];
  }
  float margin = (float)(best - second);
  float conf = margin / (fabsf((float)best) + 50.0f);
  if (conf < 0) conf = 0;
  if (conf > 1) conf = 1;
  return conf;
}

// ===================== Online Stats for Confidence Baseline =====================
void confUpdate(double x) {
  confN++;
  double delta = x - confMean;
  confMean += delta / (double)confN;
  double delta2 = x - confMean;
  confM2 += delta * delta2;
}
double confVar() {
  if (confN < 2) return 0.0;
  return confM2 / (double)(confN - 1);
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

// ===================== Sensor Sanity Check =====================
bool sensorInvalid() {
  int lastIdx = (head - 1 + WINDOW_SIZE) % WINDOW_SIZE;
  int rawLast = windowBuf[lastIdx];
  return (rawLast < 0 || rawLast > 4095);
}

// ===================== Safe Control Logic =====================
// Map stable label -> target ON/OFF
int labelToTarget(int stableLabel) {
  // Treat ALERT as ON-intent but with alert overlay
  return (stableLabel >= 1) ? 1 : 0;
}

void safeControlUpdate(int stableLabel) {
  uint32_t now = millis();

  // Safety override
  if (sensorInvalid()) {
    actuatorState = false;
    candidateTarget = -1;
    candidateStart = 0;
    lastPostLabel = 0;
    return;
  }

  int desired = labelToTarget(stableLabel);

  // Minimum hold times
  uint32_t heldFor = now - stateChangedAt;
  if (actuatorState && heldFor < MIN_ON_HOLD_MS) desired = 1;
  if (!actuatorState && heldFor < MIN_OFF_HOLD_MS) desired = 0;

  // If no change needed, clear candidate
  if (desired == (actuatorState ? 1 : 0)) {
    candidateTarget = -1;
    candidateStart = 0;
  } else {
    // Candidate state change
    if (candidateTarget != desired) {
      candidateTarget = desired;
      candidateStart = now;
    }
    uint32_t elapsed = now - candidateStart;

    if (candidateTarget == 1 && elapsed >= CONFIRM_ON_MS) {
      actuatorState = true;
      stateChangedAt = now;
      candidateTarget = -1;
      candidateStart = 0;
    } else if (candidateTarget == 0 && elapsed >= CONFIRM_OFF_MS) {
      actuatorState = false;
      stateChangedAt = now;
      candidateTarget = -1;
      candidateStart = 0;
    }
  }

  // Post label: keep alert overlay if stableLabel==2
  if (!actuatorState) lastPostLabel = 0;
  else lastPostLabel = (stableLabel == 2) ? 2 : 1;
}

// ===================== Actuation Outputs =====================
void driveOutputs() {
  static bool blink = false;
  static uint32_t lastBlink = 0;

  if (lastPostLabel == 0) {
    digitalWrite(LED_PIN, LOW);
    // digitalWrite(ACT_PIN, LOW);
    return;
  }

  if (lastPostLabel == 1) {
    digitalWrite(LED_PIN, HIGH);
    // digitalWrite(ACT_PIN, HIGH);
    return;
  }

  // ALERT blink (LED only; keep relay/motor OFF for safety if desired)
  uint32_t now = millis();
  if (now - lastBlink >= 100) {
    lastBlink = now;
    blink = !blink;
  }
  digitalWrite(LED_PIN, blink ? HIGH : LOW);
  // digitalWrite(ACT_PIN, LOW); // recommended for relay during alert
}

// ===================== Connectivity =====================
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  uint32_t now = millis();
  if (now - lastWiFiAttempt < WIFI_RETRY_MS) return;
  lastWiFiAttempt = now;

  Serial.print("Wi-Fi connect attempt: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void ensureMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqttClient.connected()) return;

  uint32_t now = millis();
  if (now - lastMQTTAttempt < MQTT_RETRY_MS) return;
  lastMQTTAttempt = now;

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("MQTT connected");
  } else {
    Serial.print("MQTT connect failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void publishTelemetry(float conf, float confScoreZ) {
  if (!mqttClient.connected()) return;

  uint32_t now = millis();
  if (now - lastMQTTPublish < MQTT_PUB_MS) return;
  lastMQTTPublish = now;

  // Send only semantic data (NO raw sensor stream)
  char payload[220];
  snprintf(payload, sizeof(payload),
           "{\"ts\":%lu,\"pred\":%d,\"stable\":%d,\"post\":%d,"
           "\"act\":%d,\"conf\":%.3f,\"conf_z\":%.3f,\"anom\":%d,"
           "\"infer_us\":%lu,\"uptime_s\":%lu}",
           (unsigned long)now,
           lastPred,
           lastStableLabel,
           lastPostLabel,
           actuatorState ? 1 : 0,
           conf,
           confScoreZ,
           confAnomaly ? 1 : 0,
           (unsigned long)lastInferUs,
           (unsigned long)(now / 1000));

  mqttClient.publish(TOPIC_TELEM, payload);

  char status[140];
  snprintf(status, sizeof(status),
           "{\"ip\":\"%s\",\"rssi\":%d,\"wifi\":%d,\"mqtt\":%d}",
           WiFi.localIP().toString().c_str(),
           WiFi.RSSI(),
           (WiFi.status() == WL_CONNECTED) ? 1 : 0,
           mqttClient.connected() ? 1 : 0);

  mqttClient.publish(TOPIC_STATUS, status);
}

// ===================== Setup =====================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  // pinMode(ACT_PIN, OUTPUT);
  // digitalWrite(ACT_PIN, LOW);

  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;
  for (int i = 0; i < DECISION_WIN; i++) decisionBuf[i] = 0;

  actuatorState = false;
  stateChangedAt = millis();
  candidateTarget = -1;
  candidateStart = 0;

  WiFi.mode(WIFI_STA);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  confBaselineStart = millis();

  Serial.println("=====================================================================================");
  Serial.println(" Lab 11 Capstone: Sensing + Features + INT8 TinyML + Safe Control + MQTT Monitoring");
  Serial.println(" Local CSV: time_ms,pred,stable,post,act,conf,conf_z,anom,infer_us,wifi,mqtt");
  Serial.println(" MQTT topic: tinyml/esp32/lab11/telemetry (JSON)");
  Serial.println("=====================================================================================");
  Serial.println("time_ms,pred,stable,post,act,conf,conf_z,anom,infer_us,wifi,mqtt");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // Connectivity (non-blocking)
  ensureWiFi();
  ensureMQTT();
  if (mqttClient.connected()) mqttClient.loop();

  // 1) Sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  float conf = 0.0f;
  float confZ = 0.0f;

  // 2) Inference + CPS control
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    float features[INPUT_SIZE];
    extractFeatures(features);

    uint32_t t0 = micros();
    lastPred = predict_int8(features, lastScores);
    uint32_t t1 = micros();
    lastInferUs = (uint32_t)(t1 - t0);

    // Stabilize decision
    addDecision(lastPred);
    lastStableLabel = smoothDecision();

    // Safety control
    safeControlUpdate(lastStableLabel);

    // Confidence + anomaly-like z-score (optional)
    conf = confidence_margin(lastScores, lastPred);

    // Learn baseline confidence for the first CONF_BASELINE_MS
    if (!confBaselineReady) {
      confUpdate((double)conf);
      if (now - confBaselineStart >= CONF_BASELINE_MS && confN >= 20) {
        confBaselineReady = true;
      }
      confAnomaly = false;
      confZ = 0.0f;
    } else {
      double v = confVar();
      double s = sqrt(v);
      confZ = (float)(fabs((double)conf - confMean) / (s + 1e-6));
      confAnomaly = (confZ > CONF_ANOM_THRESH);
    }

    // Local CSV log (semantic)
    Serial.print(now); Serial.print(",");
    Serial.print(lastPred); Serial.print(",");
    Serial.print(lastStableLabel); Serial.print(",");
    Serial.print(lastPostLabel); Serial.print(",");
    Serial.print(actuatorState ? 1 : 0); Serial.print(",");
    Serial.print(conf, 3); Serial.print(",");
    Serial.print(confZ, 3); Serial.print(",");
    Serial.print(confAnomaly ? 1 : 0); Serial.print(",");
    Serial.print(lastInferUs); Serial.print(",");
    Serial.print((WiFi.status() == WL_CONNECTED) ? 1 : 0); Serial.print(",");
    Serial.println(mqttClient.connected() ? 1 : 0);
  }

  // 3) Actuation always responsive
  driveOutputs();

  // 4) Publish telemetry (if connected)
  publishTelemetry(conf, confZ);
}

/*********************** CAPSTONE CHECKLIST ************************
1) Works offline:
   - Inference + control must continue with Wi-Fi off ?

2) Data minimization:
   - Publish ONLY pred/conf/status; do NOT publish raw ADC stream ?

3) Safety:
   - Use hold times + confirmations to avoid chatter ?
   - Default OFF on boot ?

4) Documentation:
   - State your class labels, thresholds, and timing parameters
   - Provide results: latency (infer_us), stability, MQTT dashboard screenshot
*******************************************************************/

