// Smart Traffic TinyML (10s window) — Button mock sensors
// GPIO14 = NS vehicle (1 press = 1 vehicle)
// GPIO16 = EW vehicle (1 press = 1 vehicle)
//
// Publishes 10s counts:  city/intersection1/traffic_10s
// Publishes prediction:  city/intersection1/prediction   (LOW/MEDIUM/HIGH)
//
// Requirements:
// - WiFi + PubSubClient
// - TensorFlow Lite Micro (TFLM) libraries for Arduino/ESP32
// - Model array files: traffic_model_data.cpp + traffic_model_data.h

#include <WiFi.h>
#include <PubSubClient.h>

// -------------------- User Config --------------------
#define WIFI_SSID   "YOUR_WIFI"
#define WIFI_PASS   "YOUR_PASS"

#define MQTT_HOST   "broker.hivemq.com"   // or your broker IP
#define MQTT_PORT   1883

// Topics
static const char* TOPIC_TRAFFIC = "city/intersection1/traffic_10s";
static const char* TOPIC_PRED    = "city/intersection1/prediction";

// -------------------- Mock Sensor Buttons --------------------
static const int BTN_NS = 14;    // NS vehicle press
static const int BTN_EW = 16;    // EW vehicle press

// Debounce
static const uint32_t DEBOUNCE_MS = 120;
volatile uint32_t nsCount = 0;
volatile uint32_t ewCount = 0;
volatile uint32_t lastNsMs = 0;
volatile uint32_t lastEwMs = 0;

// Windowing
static const uint32_t WINDOW_MS = 10000;
uint32_t lastWindowMs = 0;

// History for features (current, prev, prev2)
uint16_t histNS[3] = {0,0,0};
uint16_t histEW[3] = {0,0,0};

// Normalization config (training must match!)
static const float MAX_COUNT_PER_WINDOW = 20.0f;

// -------------------- MQTT --------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);
}

static void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) {
    String cid = "esp32-traffic-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    mqtt.connect(cid.c_str());
    delay(300);
  }
}

// -------------------- Interrupts --------------------
void IRAM_ATTR isrNs() {
  uint32_t now = millis();
  if (now - lastNsMs >= DEBOUNCE_MS) {
    nsCount++;
    lastNsMs = now;
  }
}
void IRAM_ATTR isrEw() {
  uint32_t now = millis();
  if (now - lastEwMs >= DEBOUNCE_MS) {
    ewCount++;
    lastEwMs = now;
  }
}

// -------------------- TinyML (TFLM) --------------------
// If you don't have TFLM installed yet, set USE_TFLM 0 to compile without TinyML.
#define USE_TFLM 1

#if USE_TFLM
  #include "traffic_model_data.h"
  #include "tensorflow/lite/micro/all_ops_resolver.h"
  #include "tensorflow/lite/micro/micro_interpreter.h"
  #include "tensorflow/lite/schema/schema_generated.h"
  #include "tensorflow/lite/version.h"

  constexpr int kArenaSize = 20 * 1024;
  static uint8_t tensor_arena[kArenaSize];

  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  static inline int8_t clamp_int8(int x) {
    if (x > 127) return 127;
    if (x < -128) return -128;
    return (int8_t)x;
  }

  static int8_t quantize_float_to_int8(float x, float scale, int zero_point) {
    int q = (int)lroundf(x / scale) + zero_point;
    return clamp_int8(q);
  }

  static void setupTFLM() {
    model = tflite::GetModel(traffic_cls_int8_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
      Serial.println("TFLM: schema mismatch");
      while (1) delay(1000);
    }
    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kArenaSize);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
      Serial.println("TFLM: AllocateTensors failed");
      while (1) delay(1000);
    }
    input = interpreter->input(0);
    output = interpreter->output(0);
    Serial.println("TFLM ready");
  }

  static int argmax3(const int8_t* v) {
    int best = 0;
    for (int i=1;i<3;i++) if (v[i] > v[best]) best = i;
    return best;
  }

  // Returns class: 0/1/2
  static int predictCongestionClass(float x6[6]) {
    // quant params from model
    const float scale = input->params.scale;
    const int zp = input->params.zero_point;

    for (int i=0;i<6;i++) {
      input->data.int8[i] = quantize_float_to_int8(x6[i], scale, zp);
    }
    if (interpreter->Invoke() != kTfLiteOk) {
      Serial.println("TFLM: Invoke failed");
      return 1; // fallback MEDIUM
    }
    const int8_t* y = output->data.int8;
    return argmax3(y);
  }
#endif

// -------------------- Utility --------------------
static const char* clsToLabel(int cls) {
  if (cls == 0) return "LOW";
  if (cls == 1) return "MEDIUM";
  return "HIGH";
}

static float normCount(uint16_t c) {
  if (c > (uint16_t)MAX_COUNT_PER_WINDOW) c = (uint16_t)MAX_COUNT_PER_WINDOW;
  return (float)c / MAX_COUNT_PER_WINDOW;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_NS, INPUT_PULLUP);
  pinMode(BTN_EW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_NS), isrNs, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_EW), isrEw, FALLING);

  connectWiFi();
  connectMQTT();

#if USE_TFLM
  setupTFLM();
#else
  Serial.println("USE_TFLM=0 (no TinyML).");
#endif

  lastWindowMs = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  const uint32_t now = millis();
  if (now - lastWindowMs >= WINDOW_MS) {
    // capture and reset window counts
    noInterrupts();
    uint32_t ns = nsCount; nsCount = 0;
    uint32_t ew = ewCount; ewCount = 0;
    interrupts();

    // shift history: [0]=current, [1]=prev, [2]=prev2
    histNS[2] = histNS[1];
    histNS[1] = histNS[0];
    histNS[0] = (uint16_t)ns;

    histEW[2] = histEW[1];
    histEW[1] = histEW[0];
    histEW[0] = (uint16_t)ew;

    // publish traffic window + history
    char payload[220];
    snprintf(payload, sizeof(payload),
      "{\"ts_ms\":%lu,\"ns_10s\":%u,\"ew_10s\":%u,\"hist_ns\":[%u,%u,%u],\"hist_ew\":[%u,%u,%u],\"window_ms\":%u}",
      (unsigned long)now,
      (unsigned)histNS[0], (unsigned)histEW[0],
      (unsigned)histNS[0], (unsigned)histNS[1], (unsigned)histNS[2],
      (unsigned)histEW[0], (unsigned)histEW[1], (unsigned)histEW[2],
      (unsigned)WINDOW_MS
    );
    mqtt.publish(TOPIC_TRAFFIC, payload);
    Serial.println(payload);

    // build features (normalized)
    float x6[6] = {
      normCount(histNS[0]), normCount(histEW[0]),
      normCount(histNS[1]), normCount(histEW[1]),
      normCount(histNS[2]), normCount(histEW[2]),
    };

    int cls = 1;
#if USE_TFLM
    cls = predictCongestionClass(x6);
#else
    // simple heuristic fallback if TinyML not enabled
    const uint16_t total = histNS[0] + histEW[0];
    cls = (total <= 3) ? 0 : (total <= 8 ? 1 : 2);
#endif

    char predMsg[48];
    snprintf(predMsg, sizeof(predMsg), "{\"pred\":\"%s\",\"cls\":%d}", clsToLabel(cls), cls);
    mqtt.publish(TOPIC_PRED, predMsg);
    Serial.println(predMsg);

    lastWindowMs += WINDOW_MS; // keep stable cadence
  }
}
