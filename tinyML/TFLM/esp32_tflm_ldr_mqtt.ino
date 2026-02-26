#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------- TFLM ----------
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// ---------- Your INT8 model ----------
#include "model_data_int8.h"   // g_model, g_model_len

// ===================== USER CONFIG =====================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define MQTT_HOST "192.168.1.10"
#define MQTT_PORT 1883

// Topics
const char* TOPIC_STATE = "cps/lab/ldr/pred";   // publishes JSON
const char* TOPIC_CMD   = "cps/lab/ldr/cmd";    // subscribe commands

// Hardware
static const int LDR_PIN = 36;  // ADC1_CH0
// =======================================================

// ---------- MQTT ----------
WiFiClient espClient;
PubSubClient mqtt(espClient);

// ---------- TFLM globals ----------
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 10 * 1024;  // increase if needed
static uint8_t tensor_arena[kTensorArenaSize];

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// ---------- Control ----------
String mode = "auto";              // "auto" | "dark" | "normal" | "bright"
uint32_t publish_ms = 500;         // default publish rate
uint32_t last_pub = 0;

// ---------- Helpers ----------
static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

static inline int8_t clamp_int8(int v) {
  if (v < -128) return -128;
  if (v > 127)  return 127;
  return (int8_t)v;
}

const char* label_from_idx(int idx) {
  switch (idx) {
    case 0: return "dark";
    case 1: return "normal";
    case 2: return "bright";
    default: return "unknown";
  }
}

int argmax3(const float p[3]) {
  int best = 0;
  if (p[1] > p[best]) best = 1;
  if (p[2] > p[best]) best = 2;
  return best;
}

// ---------- WiFi / MQTT ----------
void wifi_connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi OK. IP: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  msg.toLowerCase();

  // Commands:
  //   mode:auto | mode:dark | mode:normal | mode:bright
  //   rate:500   (ms)
  if (msg.startsWith("mode:")) {
    String m = msg.substring(5);
    if (m == "auto" || m == "dark" || m == "normal" || m == "bright") {
      mode = m;
      Serial.printf("[CMD] mode=%s\n", mode.c_str());
    }
  } else if (msg.startsWith("rate:")) {
    uint32_t r = (uint32_t) msg.substring(5).toInt();
    if (r >= 100 && r <= 10000) {
      publish_ms = r;
      Serial.printf("[CMD] publish_ms=%lu\n", (unsigned long)publish_ms);
    }
  } else {
    Serial.printf("[CMD] unknown: %s\n", msg.c_str());
  }
}

void mqtt_connect() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqtt_callback);

  while (!mqtt.connected()) {
    Serial.print("MQTT connecting...");
    String clientId = "esp32-ldr-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_CMD);
      mqtt.publish(TOPIC_STATE, "{\"status\":\"online\"}", true);
    } else {
      Serial.printf("fail rc=%d, retry...\n", mqtt.state());
      delay(1000);
    }
  }
}

// ---------- TFLM setup ----------
void tflm_setup() {
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Schema mismatch: model=%d runtime=%d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    while (1) delay(10);
  }

  // Minimal ops for 1-8-3: FullyConnected, Relu, Softmax
  static tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed! Increase tensor arena.");
    while (1) delay(10);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TFLM ready.");
  Serial.printf("Input type=%d, Output type=%d\n", input->type, output->type);

  if (input->type != kTfLiteInt8 || output->type != kTfLiteInt8) {
    Serial.println("Warning: This sketch expects FULL INT8 model I/O.");
  }
}

// ---------- Inference + publish ----------
void publish_prediction() {
  // Read LDR
  int adc = analogRead(LDR_PIN);
  float x = clamp01((float)adc / 4095.0f);

  // Quantize input (float -> int8)
  const float in_scale = input->params.scale;
  const int in_zero    = input->params.zero_point;
  int q_in = (int)lrintf(x / in_scale) + in_zero;
  input->data.int8[0] = clamp_int8(q_in);

  // Invoke
  uint32_t t0 = micros();
  TfLiteStatus st = interpreter->Invoke();
  uint32_t t1 = micros();

  if (st != kTfLiteOk) {
    Serial.println("Invoke failed.");
    return;
  }

  // Dequantize output (int8 -> float)
  const float out_scale = output->params.scale;
  const int out_zero    = output->params.zero_point;

  float p[3];
  for (int i = 0; i < 3; i++) {
    int8_t q = output->data.int8[i];
    p[i] = ((int)q - out_zero) * out_scale;
  }

  int cls = argmax3(p);
  const char* cls_label = label_from_idx(cls);

  // Allow override mode (for demo / testing digital twin)
  const char* final_label = cls_label;
  if (mode != "auto") final_label = mode.c_str();

  // Build JSON payload (no ArduinoJson required)
  char payload[256];
  snprintf(payload, sizeof(payload),
           "{"
             "\"ts\":%lu,"
             "\"adc\":%d,"
             "\"x\":%.6f,"
             "\"p\":[%.3f,%.3f,%.3f],"
             "\"class\":\"%s\","
             "\"mode\":\"%s\","
             "\"lat_us\":%lu"
           "}",
           (unsigned long)millis(),
           adc,
           x,
           p[0], p[1], p[2],
           final_label,
           mode.c_str(),
           (unsigned long)(t1 - t0));

  // Publish
  mqtt.publish(TOPIC_STATE, payload, false);

  // Serial log too
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  analogReadResolution(12); // 0..4095

  wifi_connect();
  mqtt_connect();
  tflm_setup();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    wifi_connect();
  }
  if (!mqtt.connected()) {
    mqtt_connect();
  }
  mqtt.loop();

  uint32_t now = millis();
  if (now - last_pub >= publish_ms) {
    last_pub = now;
    publish_prediction();
  }
}
