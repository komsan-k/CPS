#include <Arduino.h>

// ----- TFLM -----
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// ----- INT8 model -----
#include "model_data_int8.h"   // provides: g_model, g_model_len

static const int LDR_PIN = 36;

// TFLM globals
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 10 * 1024;  // increase if AllocateTensors fails
static uint8_t tensor_arena[kTensorArenaSize];

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

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
  delay(200);
  analogReadResolution(12); // 0..4095

  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model schema mismatch! model=%d runtime=%d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    while (1) delay(10);
  }

  // Use minimal resolver for small binary
  static tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed! Increase kTensorArenaSize.");
    while (1) delay(10);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TFLM INT8 ready.");
  Serial.printf("Input type=%d, Output type=%d\n", input->type, output->type);

  // Expect int8 input/output for full integer model
  if (input->type != kTfLiteInt8 || output->type != kTfLiteInt8) {
    Serial.println("Warning: model is not full-int8 (input/output). Check converter settings.");
  }
}

void loop() {
  int adc = analogRead(LDR_PIN);

  // Normalize to float [0,1] (your model was trained on this)
  float x = clamp01((float)adc / 4095.0f);

  // -------- Quantize input float -> int8 --------
  // q = round(x / scale) + zero_point
  const float in_scale = input->params.scale;
  const int in_zero = input->params.zero_point;

  int q_in = (int)lrintf(x / in_scale) + in_zero;
  input->data.int8[0] = clamp_int8(q_in);

  // -------- Invoke --------
  uint32_t t0 = micros();
  TfLiteStatus st = interpreter->Invoke();
  uint32_t t1 = micros();

  if (st != kTfLiteOk) {
    Serial.println("Invoke failed!");
    delay(500);
    return;
  }

  // -------- Dequantize output int8 -> float --------
  const float out_scale = output->params.scale;
  const int out_zero = output->params.zero_point;

  float p[3];
  for (int i = 0; i < 3; i++) {
    int8_t q = output->data.int8[i];
    p[i] = (float)( (int)q - out_zero ) * out_scale;
  }

  int cls = argmax3_float(p);
  const char* label = label_from_idx(cls);

  Serial.printf("ADC=%d x=%.3f | q_in=%d (scale=%.6f zp=%d) | "
                "p=[%.3f %.3f %.3f] => %s | latency=%lu us\n",
                adc, x, (int)input->data.int8[0], in_scale, in_zero,
                p[0], p[1], p[2], label, (unsigned long)(t1 - t0));

  delay(200);
}
