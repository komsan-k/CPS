#include <Arduino.h>

#include "ldr_model_float32.h"

#include <tflm_esp32.h> 

// ----- TFLM -----
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
// #include "tensorflow/lite/version.h"

// ----- FLOAT32 model -----
// #include "model_data.h"   // provides: g_model, g_model_len

static const int LDR_PIN = 36;

// TFLM globals
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

// Increase if AllocateTensors() fails
constexpr int kTensorArenaSize = 10 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

static int argmax3_float(const float* p) {
  int best = 0;
  if (p[1] > p[best]) best = 1;
  if (p[2] > p[best]) best = 2;
  return best;
}

const char* label_from_idx(int idx) {
  switch (idx) {
    case 0: return "dark";
    case 1: return "normal";
    case 2: return "bright";
    default: return "unknown";
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12); // ESP32 ADC: 0..4095

  model = tflite::GetModel(g_model);
  if (model == nullptr) {
    Serial.println("Failed to load model.");
    while (1) delay(10);
  }

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model schema mismatch! model=%d runtime=%d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    while (1) delay(10);
  }

  // Adjust ops if your float32 model uses different layers
  static tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  // For newer TFLM constructor style
  static tflite::MicroInterpreter static_interpreter(
      model,
      resolver,
      tensor_arena,
      kTensorArenaSize,
      error_reporter,
      nullptr,
      nullptr);

  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed! Increase kTensorArenaSize.");
    while (1) delay(10);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  if (input == nullptr || output == nullptr) {
    Serial.println("Input/output tensor is null.");
    while (1) delay(10);
  }

  Serial.println("TFLM FLOAT32 ready.");
  Serial.printf("Input type=%d, Output type=%d\n", input->type, output->type);

  if (input->type != kTfLiteFloat32) {
    Serial.println("Warning: input tensor is not float32.");
  }
  if (output->type != kTfLiteFloat32) {
    Serial.println("Warning: output tensor is not float32.");
  }
}

void loop() {
  int adc = analogRead(LDR_PIN);

  // Normalize to [0,1]
  float x = clamp01((float)1.0f - (adc / 4095.0f));

  // -------- FLOAT32 input --------
  input->data.f[0] = x;

  // -------- Invoke --------
  uint32_t t0 = micros();
  TfLiteStatus st = interpreter->Invoke();
  uint32_t t1 = micros();

  if (st != kTfLiteOk) {
    Serial.println("Invoke failed!");
    delay(500);
    return;
  }

  // -------- FLOAT32 output --------
  float p[3];
  for (int i = 0; i < 3; i++) {
    p[i] = output->data.f[i];
  }

  int cls = argmax3_float(p);
  const char* label = label_from_idx(cls);

  Serial.printf("ADC=%d x=%.3f | p=[%.3f %.3f %.3f] => %s | latency=%lu us\n",
                adc, x, p[0], p[1], p[2], label,
                (unsigned long)(t1 - t0));

  delay(500);
}
