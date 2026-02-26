#include <Arduino.h>

// ----- TFLM includes (TensorFlowLite_ESP32 / Arduino_TensorFlowLite style) -----
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// ----- Your model C array -----
#include "model_data.h"   // provides: g_model, g_model_len

// ----- Hardware -----
static const int LDR_PIN = 36;  // ADC1_CH0 on ESP32

// ----- TFLM Globals -----
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

// Tensor Arena: adjust if you get "Arena allocation failed"
constexpr int kTensorArenaSize = 10 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

// Tensors
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// ----- Helpers -----
static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

static int argmax3(const float* p) {
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

  // 1) Map the model
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model schema mismatch! model=%d, runtime=%d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    while (1) delay(10);
  }

  // 2) Resolver (all ops is simplest; later you can switch to MicroMutableOpResolver for smaller binary)
  static tflite::AllOpsResolver resolver;

  // 3) Interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // 4) Allocate tensors
  TfLiteStatus alloc_status = interpreter->AllocateTensors();
  if (alloc_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed. Try increasing kTensorArenaSize.");
    while (1) delay(10);
  }

  // 5) Get I/O tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Sanity checks (expect input [1,1] float32 and output [1,3] float32)
  Serial.println("TFLM ready.");
  Serial.printf("Input type=%d, dims=%d\n", input->type, input->dims->size);
  Serial.printf("Output type=%d, dims=%d\n", output->type, output->dims->size);

  // ADC setup (optional)
  analogReadResolution(12); // 0..4095
}

void loop() {
  // Read LDR ADC
  int adc = analogRead(LDR_PIN);

  // Normalize to [0,1]
  float x = (float)adc / 4095.0f;
  x = clamp01(x);

  // Write to model input
  if (input->type == kTfLiteFloat32) {
    input->data.f[0] = x;
  } else {
    // If you later switch to int8 quantized model, you must quantize x here
    Serial.println("Input is not float32. If using int8 model, update quantization code.");
    delay(1000);
    return;
  }

  // Run inference
  uint32_t t0 = micros();
  TfLiteStatus status = interpreter->Invoke();
  uint32_t t1 = micros();

  if (status != kTfLiteOk) {
    Serial.println("Invoke failed!");
    delay(500);
    return;
  }

  // Read output probabilities
  if (output->type == kTfLiteFloat32) {
    float p0 = output->data.f[0];
    float p1 = output->data.f[1];
    float p2 = output->data.f[2];

    int cls = argmax3(output->data.f);
    const char* label = label_from_idx(cls);

    Serial.printf("ADC=%d, x=%.3f | p=[%.3f %.3f %.3f] => %s | latency=%lu us\n",
                  adc, x, p0, p1, p2, label, (unsigned long)(t1 - t0));
  } else {
    Serial.println("Output is not float32. If using int8 model, dequantize output.");
  }

  delay(200);
}
