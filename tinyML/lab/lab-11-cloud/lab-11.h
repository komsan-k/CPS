/***************************************************
 * Lab 10: Edge-to-Cloud Monitoring for TinyML Systems (ESP32)
 * Full Arduino Code (Wi-Fi + MQTT + TinyML Inference Results Publish)
 *
 * This sketch:
 *  1) Runs TinyML inference locally on ESP32 (feature-based, INT8 example)
 *  2) Publishes ONLY small semantic results to the cloud via MQTT:
 *        - prediction label
 *        - confidence proxy (margin between top-2 scores)
 *        - optional anomaly score placeholder
 *        - device status (uptime)
 *  3) Keeps the CPS working even if Wi-Fi/MQTT is down
 *
 * IMPORTANT:
 *  - Replace Wi-Fi credentials and MQTT broker settings.
 *  - Model parameters (Wi/bi and quantization) are EXAMPLE values.
 *  - This code uses PubSubClient library:
 *      Arduino IDE -> Library Manager -> "PubSubClient" by Nick O'Leary
 *
 * Topics:
 *   Publish: tinyml/esp32/status
 *   Publish: tinyml/esp32/telemetry
 *
 * Payload format: JSON strings (compact)
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
// Use an MQTT broker IP on your LAN, or a broker hostname.
// Example: "broker.hivemq.com" (public) or local broker IP.
const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

// Client + topics
const char* MQTT_CLIENT_ID = "esp32-tinyml-lab10";
const char* TOPIC_STATUS   = "tinyml/esp32/status";
const char* TOPIC_TELEM    = "tinyml/esp32/telemetry";

// ===================== Pins =====================
#define SENSOR_PIN 34
#define LED_PIN     2

// ===================== Timing =====================
const uint32_t SAMPLE_PERIOD_MS = 20;   // 50 Hz
const uint32_t INFER_PERIOD_MS  = 200;  // inference every 200 ms
const uint32_t MQTT_PUB_MS      = 500;  // publish every 500 ms (2 Hz)
const uint32_t WIFI_RETRY_MS    = 5000; // retry Wi-Fi every 5s
const uint32_t MQTT_RETRY_MS    = 5000; // retry MQTT every 5s

// ===================== Sliding Window =====================
#define WINDOW_SIZE 20
int windowBuf[WINDOW_SIZE];
int head = 0;
uint32_t sampleCount = 0;

// ===================== Features / Model =====================
#define INPUT_SIZE  6
#define NUM_CLASSES 3

// INT8 linear model (EXAMPLE)
const int8_t Wi[NUM_CLASSES][INPUT_SIZE] = {
  { -2,  1,  1, -1,  2,  5 },
  {  1, -1, -1,  1, -2, -2 },
  {  2, -1, -2, -1,  1,  3 }
};
const int32_t bi[NUM_CLASSES] = { 10, 0, -10 };

// Quantization params (EXAMPLE)
const float X_SCALE = 50.0f;
const int   X_ZERO_POINT = 0;

// ===================== MQTT / WiFi =====================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

uint32_t lastWiFiAttempt = 0;
uint32_t lastMQTTAttempt = 0;
uint32_t lastMQTTPublish = 0;

// ===================== Local State =====================
uint32_t lastSampleTime = 0;
uint32_t lastInferTime  = 0;

int lastPred = 0;
int32_t lastScores[NUM_CLASSES] = {0};
uint32_t lastInferUs = 0;

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

// ===================== Confidence Proxy =====================
// For linear int score, a simple "margin" between best and 2nd best
float confidence_margin(const int32_t scores[NUM_CLASSES], int bestIdx) {
  int32_t best = scores[bestIdx];
  int32_t second = INT32_MIN;
  for (int i = 0; i < NUM_CLASSES; i++) {
    if (i == bestIdx) continue;
    if (scores[i] > second) second = scores[i];
  }
  // Map margin to a 0..1-ish value (heuristic)
  float margin = (float)(best - second);
  float conf = margin / (fabsf((float)best) + 50.0f); // avoid divide by 0
  if (conf < 0) conf = 0;
  if (conf > 1) conf = 1;
  return conf;
}

// ===================== Wi-Fi / MQTT Connect =====================
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  uint32_t now = millis();
  if (now - lastWiFiAttempt < WIFI_RETRY_MS) return;
  lastWiFiAttempt = now;

  Serial.print("Connecting Wi-Fi: ");
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

  Serial.print("Connecting MQTT: ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.println(MQTT_PORT);

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  // Connect with clean session
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("MQTT connected.");
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.println(mqttClient.state());
  }
}

// ===================== Publish MQTT =====================
void publishTelemetry() {
  if (!mqttClient.connected()) return;

  uint32_t now = millis();
  if (now - lastMQTTPublish < MQTT_PUB_MS) return;
  lastMQTTPublish = now;

  float conf = confidence_margin(lastScores, lastPred);

  // Lightweight JSON payload (no raw sensor data)
  // Example:
  // {"ts":12345,"pred":1,"conf":0.62,"infer_us":420,"uptime_s":12}
  char payload[196];
  snprintf(payload, sizeof(payload),
           "{\"ts\":%lu,\"pred\":%d,\"conf\":%.3f,\"infer_us\":%lu,\"uptime_s\":%lu}",
           (unsigned long)now,
           lastPred,
           conf,
           (unsigned long)lastInferUs,
           (unsigned long)(now / 1000));

  mqttClient.publish(TOPIC_TELEM, payload);

  // Also publish a simple status string occasionally (optional)
  char statusMsg[96];
  snprintf(statusMsg, sizeof(statusMsg),
           "{\"ip\":\"%s\",\"rssi\":%d,\"wifi\":%d,\"mqtt\":%d}",
           WiFi.localIP().toString().c_str(),
           WiFi.RSSI(),
           (WiFi.status() == WL_CONNECTED) ? 1 : 0,
           mqttClient.connected() ? 1 : 0);

  mqttClient.publish(TOPIC_STATUS, statusMsg);
}

// ===================== Actuation (local only) =====================
void updateLED(int label) {
  // Example mapping:
  // 0 -> OFF
  // 1 -> ON
  // 2 -> BLINK
  static bool blink = false;
  static uint32_t lastBlink = 0;

  if (label == 0) {
    digitalWrite(LED_PIN, LOW);
  } else if (label == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    uint32_t now = millis();
    if (now - lastBlink >= 100) {
      lastBlink = now;
      blink = !blink;
    }
    digitalWrite(LED_PIN, blink ? HIGH : LOW);
  }
}

// ===================== Setup =====================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WINDOW_SIZE; i++) windowBuf[i] = 0;

  WiFi.mode(WIFI_STA);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println("===================================================================================");
  Serial.println(" Lab 10: Edge-to-Cloud Monitoring (ESP32 TinyML local inference + MQTT publish)");
  Serial.println(" Publishes: tinyml/esp32/telemetry and tinyml/esp32/status");
  Serial.println(" CSV(local): time_ms,pred,conf,infer_us,wifi,mqtt");
  Serial.println("===================================================================================");
  Serial.println("time_ms,pred,conf,infer_us,wifi,mqtt");
}

// ===================== Loop =====================
void loop() {
  uint32_t now = millis();

  // 0) Keep connectivity attempts non-blocking
  ensureWiFi();
  ensureMQTT();

  // mqtt loop must be called frequently when connected
  if (mqttClient.connected()) mqttClient.loop();

  // 1) Sampling
  if (now - lastSampleTime >= SAMPLE_PERIOD_MS) {
    lastSampleTime = now;
    int raw = analogRead(SENSOR_PIN);
    addSample(raw);
  }

  // 2) Local inference (always local, regardless of Wi-Fi/MQTT)
  if (sampleCount >= WINDOW_SIZE && (now - lastInferTime >= INFER_PERIOD_MS)) {
    lastInferTime = now;

    float features[INPUT_SIZE];
    extractFeatures(features);

    uint32_t t0 = micros();
    lastPred = predict_int8(features, lastScores);
    uint32_t t1 = micros();
    lastInferUs = (uint32_t)(t1 - t0);

    // Local actuation (CPS must keep working even offline)
    updateLED(lastPred);

    // Print local status line
    float conf = confidence_margin(lastScores, lastPred);
    Serial.print(now); Serial.print(",");
    Serial.print(lastPred); Serial.print(",");
    Serial.print(conf, 3); Serial.print(",");
    Serial.print(lastInferUs); Serial.print(",");
    Serial.print((WiFi.status() == WL_CONNECTED) ? 1 : 0); Serial.print(",");
    Serial.println(mqttClient.connected() ? 1 : 0);
  }

  // 3) Publish to cloud if connected (low-bandwidth semantic data only)
  publishTelemetry();

  // Keep LED blink responsive
  updateLED(lastPred);
}

/*********************** QUICK SETUP GUIDE ************************
1) Install PubSubClient:
   Arduino IDE -> Tools -> Manage Libraries -> search "PubSubClient"

2) Put your Wi-Fi credentials:
   WIFI_SSID, WIFI_PASSWORD

3) Choose a broker:
   - Local Mosquitto (recommended for class)
   - Public broker (for quick demo): broker.hivemq.com

4) Node-RED dashboard (recommended):
   - Subscribe to: tinyml/esp32/telemetry
   - Parse JSON and show pred/conf/infer_us in gauges/charts

5) Principle:
   - Edge runs inference + control ALWAYS
   - Cloud gets only small semantic updates
*******************************************************************/

