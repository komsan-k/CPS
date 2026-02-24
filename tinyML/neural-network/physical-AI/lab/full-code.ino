/*******************************************************
 * Physical AI × CPS 5C — ESP32 Full Code Template
 * Labs 0–5 (Arduino IDE / PlatformIO)
 *
 * Features:
 *  - Sensor read: LDR (ADC), Temperature (placeholder), IMU vibration (optional)
 *  - Preprocessing: normalization, moving average
 *  - Feature extraction: RMS, slope
 *  - TinyML inference hook: placeholder for TFLite Micro
 *  - Decision layer: hysteresis alerting, MQTT publish
 *  - Configuration layer: Q-learning adaptive control (LED/PWM)
 *
 * Notes:
 *  - Replace WIFI_SSID / WIFI_PASS / MQTT_HOST.
 *  - Choose your temp sensor library if using DS18B20 / LM73.
 *  - If using MPU6050, enable USE_MPU6050 and add its library.
 *******************************************************/

#include <WiFi.h>
#include <PubSubClient.h>

// -------------------- User Config --------------------
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASS     "YOUR_WIFI_PASSWORD"

#define MQTT_HOST     "192.168.1.10"   // Mosquitto broker IP
#define MQTT_PORT     1883

// MQTT Topics
#define TOPIC_LUX_RAW        "cps/lab0/lux_raw"
#define TOPIC_LUX_NORM       "cps/lab0/lux_norm"
#define TOPIC_FEATURES       "cps/lab1/features"
#define TOPIC_PREDICTION     "cps/lab2/prediction"
#define TOPIC_DECISION       "cps/lab3/decision"
#define TOPIC_ALERT          "cps/lab3/alert"
#define TOPIC_RL_STATUS      "cps/lab4/rl_status"
#define TOPIC_CONTROL        "cps/lab5/control"

// Pins
static const int PIN_LDR_ADC = 34;   // ADC1 pin (ESP32)
static const int PIN_LED_PWM = 16;   // PWM output pin (LED/driver)

// PWM Setup
static const int PWM_CH   = 0;
static const int PWM_FREQ = 5000;
static const int PWM_RES  = 8;       // 0..255

// Sampling
static const uint16_t SAMPLE_WINDOW = 32;   // for RMS, moving average
static const uint32_t LOOP_MS       = 500;  // publish period

// -------------------- Optional Sensors --------------------
// #define USE_DS18B20
// #define USE_MPU6050

// -------------------- CPS/Physical AI Parameters --------------------
// Normalization (Z-score) parameters (initialize from calibration)
float mu_lux = 2000.0f;      // mean
float sigma_lux = 800.0f;    // std dev (avoid 0)

// Decision thresholds (hysteresis)
float tau_high = 1.5f;  // anomaly ON if loss/score exceeds high
float tau_low  = 1.1f;  // anomaly OFF if below low

// -------------------- MQTT & WiFi --------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

// -------------------- Rolling Buffers --------------------
float lux_buf[SAMPLE_WINDOW];
uint16_t buf_idx = 0;
bool buf_full = false;

// For slope calculation
float prev_lux_norm = 0.0f;

// Decision state
bool alert_on = false;

// -------------------- RL Q-learning (Lab 4) --------------------
// Discrete states: 0=LOW light, 1=NORMAL, 2=HIGH
// Discrete actions: 0=LED_OFF, 1=LOW, 2=MED, 3=HIGH
static const int N_STATES  = 3;
static const int N_ACTIONS = 4;

float Q[N_STATES][N_ACTIONS]; // initialized to 0

// Q-learning hyperparameters
float alpha = 0.2f;   // learning rate
float gamma_ = 0.9f;  // discount
float eps = 0.3f;     // epsilon-greedy exploration
float eps_decay = 0.995f;
float eps_min = 0.05f;

// Comfort vs energy weighting (reward shaping)
float w_comfort = 1.0f;
float w_energy  = 0.05f;

// Current RL
int state_s = 1;
int action_a = 0;

// -------------------- Utility --------------------
static inline float clampf(float x, float a, float b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

static inline int argmaxQ(int s) {
  int best = 0;
  float bestv = Q[s][0];
  for (int a = 1; a < N_ACTIONS; a++) {
    if (Q[s][a] > bestv) { bestv = Q[s][a]; best = a; }
  }
  return best;
}

// -------------------- WiFi/MQTT Setup --------------------
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(300); }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) {
    String cid = "esp32-physicalai-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    mqtt.connect(cid.c_str());
    delay(500);
  }
}

// -------------------- Sensor Read (Lab 0) --------------------
float readLuxRaw() {
  // LDR on ADC: raw 0..4095
  int adc = analogRead(PIN_LDR_ADC);
  // Simple mapping to "lux-like" scale (placeholder)
  // Replace with calibration curve if desired.
  float lux = (adc / 4095.0f) * 5000.0f;
  return lux;
}

// -------------------- Preprocessing (Lab 0) --------------------
float normalizeZ(float x, float mu, float sigma) {
  if (sigma < 1e-6f) sigma = 1e-6f;
  return (x - mu) / sigma;
}

// Moving average over buffer
float movingAverage() {
  uint16_t n = buf_full ? SAMPLE_WINDOW : buf_idx;
  if (n == 0) return 0.0f;
  float s = 0.0f;
  for (uint16_t i = 0; i < n; i++) s += lux_buf[i];
  return s / (float)n;
}

// RMS over buffer
float rmsBuffer() {
  uint16_t n = buf_full ? SAMPLE_WINDOW : buf_idx;
  if (n == 0) return 0.0f;
  float s2 = 0.0f;
  for (uint16_t i = 0; i < n; i++) s2 += lux_buf[i] * lux_buf[i];
  return sqrtf(s2 / (float)n);
}

// -------------------- Feature Extraction (Lab 1) --------------------
struct Features {
  float lux_norm;
  float lux_ma;
  float lux_rms;
  float slope;
};

Features computeFeatures(float lux_norm) {
  // Update rolling buffer with normalized values
  lux_buf[buf_idx] = lux_norm;
  buf_idx++;
  if (buf_idx >= SAMPLE_WINDOW) { buf_idx = 0; buf_full = true; }

  Features f;
  f.lux_norm = lux_norm;
  f.lux_ma   = movingAverage();
  f.lux_rms  = rmsBuffer();

  // Discrete slope: delta of normalized lux
  f.slope = lux_norm - prev_lux_norm;
  prev_lux_norm = lux_norm;
  return f;
}

// -------------------- TinyML Inference Hook (Lab 2) --------------------
// Replace this stub with your TFLite Micro inference.
// Return: predicted class index (e.g., 0..2) or regression value.
int tinymlPredictClass(const Features& f) {
  // Example placeholder logic (students replace with real model):
  // Class 0: Dark, 1: Normal, 2: Bright
  if (f.lux_ma < -0.5f) return 0;
  if (f.lux_ma >  0.8f) return 2;
  return 1;
}

// Example "anomaly score" as reconstruction/forecast error proxy
float anomalyScore(const Features& f) {
  // Placeholder: use magnitude of slope + RMS as a crude score
  float score = fabsf(f.slope) + 0.3f * f.lux_rms;
  return score;
}

// -------------------- Decision Logic (Lab 3) --------------------
void updateAlertHysteresis(float score) {
  if (!alert_on && score >= tau_high) alert_on = true;
  else if (alert_on && score <= tau_low) alert_on = false;
}

// -------------------- RL (Lab 4) --------------------
int discretizeState(float lux_norm) {
  // Simple 3-bin discretization based on normalized value
  if (lux_norm < -0.5f) return 0;   // LOW light
  if (lux_norm >  0.8f) return 2;   // HIGH light
  return 1;                         // NORMAL
}

int epsilonGreedyAction(int s) {
  float r = (float)random(0, 10000) / 10000.0f;
  if (r < eps) return random(0, N_ACTIONS);
  return argmaxQ(s);
}

float computeReward(int s, int a) {
  // Comfort target: prefer NORMAL state (1)
  float comfort = (s == 1) ? 1.0f : 0.0f;

  // Energy cost increases with brightness level
  float energy = (float)a; // 0..3

  // Reward shaping
  return w_comfort * comfort - w_energy * energy;
}

int transitionModel(int s, int a) {
  // In real CPS, next state comes from sensors (environment).
  // Here we do no simulated transition—next state should be computed from new measurement.
  // This function kept for clarity; not used directly.
  return s;
}

void qLearnUpdate(int s, int a, float r, int s_next) {
  float best_next = Q[s_next][argmaxQ(s_next)];
  Q[s][a] = Q[s][a] + alpha * (r + gamma_ * best_next - Q[s][a]);
}

// Map RL action to PWM duty
uint8_t actionToPWM(int a) {
  switch (a) {
    case 0: return 0;    // OFF
    case 1: return 64;   // LOW
    case 2: return 140;  // MED
    case 3: return 255;  // HIGH
  }
  return 0;
}

// -------------------- MQTT Publish Helpers --------------------
void publishFloat(const char* topic, float v) {
  char msg[32];
  snprintf(msg, sizeof(msg), "%.4f", v);
  mqtt.publish(topic, msg, true);
}

void publishJSONFeatures(const Features& f) {
  char msg[160];
  snprintf(msg, sizeof(msg),
           "{\"lux_norm\":%.4f,\"lux_ma\":%.4f,\"lux_rms\":%.4f,\"slope\":%.4f}",
           f.lux_norm, f.lux_ma, f.lux_rms, f.slope);
  mqtt.publish(TOPIC_FEATURES, msg, true);
}

void publishDecision(int cls, float score, bool alert) {
  char msg[160];
  snprintf(msg, sizeof(msg),
           "{\"class\":%d,\"score\":%.4f,\"alert\":%s}",
           cls, score, alert ? "true" : "false");
  mqtt.publish(TOPIC_DECISION, msg, true);
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  // ADC setup
  analogReadResolution(12);

  // PWM setup
  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_LED_PWM, PWM_CH);
  ledcWrite(PWM_CH, 0);

  randomSeed((uint32_t)ESP.getEfuseMac());

  connectWiFi();
  connectMQTT();

  // Initialize buffer
  for (int i = 0; i < SAMPLE_WINDOW; i++) lux_buf[i] = 0.0f;

  // Initialize Q-table (optional: small optimistic init)
  for (int s = 0; s < N_STATES; s++)
    for (int a = 0; a < N_ACTIONS; a++)
      Q[s][a] = 0.0f;

  Serial.println("Physical AI CPS 5C Template Started.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  // ---------------- Lab 0: Sense ----------------
  float lux_raw = readLuxRaw();
  publishFloat(TOPIC_LUX_RAW, lux_raw);

  float lux_norm = normalizeZ(lux_raw, mu_lux, sigma_lux);
  publishFloat(TOPIC_LUX_NORM, lux_norm);

  // ---------------- Lab 1: Represent ----------------
  Features f = computeFeatures(lux_norm);
  publishJSONFeatures(f);

  // ---------------- Lab 2: Predict ----------------
  int pred_class = tinymlPredictClass(f);
  // publish prediction alone if needed:
  {
    char msg[32];
    snprintf(msg, sizeof(msg), "%d", pred_class);
    mqtt.publish(TOPIC_PREDICTION, msg, true);
  }

  // ---------------- Lab 3: Decide ----------------
  float score = anomalyScore(f);
  updateAlertHysteresis(score);
  publishDecision(pred_class, score, alert_on);

  // Dedicated alert topic (simple)
  mqtt.publish(TOPIC_ALERT, alert_on ? "1" : "0", true);

  // ---------------- Lab 4: Adapt (RL) ----------------
  // Current state from latest measurement
  int s_now = discretizeState(f.lux_norm);

  // Choose action
  int a_now = epsilonGreedyAction(s_now);

  // Apply control (Act)
  uint8_t duty = actionToPWM(a_now);
  ledcWrite(PWM_CH, duty);

  // Reward from current state/action (simple shaping)
  float r = computeReward(s_now, a_now);

  // Next state comes from next loop measurement (environment),
  // but we can do an online update using current reading as s_next
  // (in practice: use s_next from next time step).
  int s_next = s_now;

  // Update Q-table
  qLearnUpdate(s_now, a_now, r, s_next);

  // Decay exploration
  eps = (eps > eps_min) ? (eps * eps_decay) : eps_min;

  // Publish RL status
  {
    char msg[200];
    snprintf(msg, sizeof(msg),
             "{\"state\":%d,\"action\":%d,\"pwm\":%u,\"reward\":%.4f,\"eps\":%.4f}",
             s_now, a_now, duty, r, eps);
    mqtt.publish(TOPIC_RL_STATUS, msg, true);
    mqtt.publish(TOPIC_CONTROL, msg, true);
  }

  // Serial debug
  Serial.printf("lux_raw=%.1f lux_norm=%.3f ma=%.3f rms=%.3f slope=%.3f cls=%d score=%.3f alert=%d a=%d pwm=%u r=%.3f eps=%.3f\n",
                lux_raw, f.lux_norm, f.lux_ma, f.lux_rms, f.slope,
                pred_class, score, alert_on ? 1 : 0, a_now, duty, r, eps);

  delay(LOOP_MS);
}
