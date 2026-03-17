#include <Arduino.h>

// ======================================================
// TensorFlow Lite for Microcontrollers
// ======================================================
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// ======================================================
// Your generated model header
// ======================================================
#include "ldr_model_int8.h"   // contains g_model and g_model_len

// ======================================================
// Hardware
// ======================================================
static const int LDR_PIN = 36;   // ADC1_CH0

// ======================================================
// TFLM globals
// ======================================================
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Adjust if AllocateTensors() fails
constexpr int kTensorArenaSize = 10 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

// ======================================================
// Utility functions
// ======================================================
static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

static inline int8_t clamp_int8(int v) {
  if (v < -128) return -128;
  if (v > 127) return 127;
  return (int8_t)v;
}

const char* label_from_index(int idx) {
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

// ======================================================
// Setup
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32 + TFLM INT8 LDR Classifier");
  Serial.println("Initializing...");

  // ESP32 ADC resolution
  analogReadResolution(12);   // 0..4095

  // Load model
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.print("Model schema mismatch! Model version: ");
    Serial.print(model->version());
    Serial.print(" Runtime version: ");
    Serial.println(TFLITE_SCHEMA_VERSION);
    while (1) delay(10);
  }

  // Resolver: only the ops needed for 1-8-3 MLP
  static tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();

  // Interpreter
  static tflite::MicroInterpreter static_interpreter(
    model,
    resolver,
    tensor_arena,
    kTensorArenaSize,
    error_reporter
  );
  interpreter = &static_interpreter;

  // Allocate tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed.");
    Serial.println("Try increasing kTensorArenaSize.");
    while (1) delay(10);
  }

  // Get input and output tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TFLM initialized successfully.");
  Serial.print("Input type: ");
  Serial.println(input->type);
  Serial.print("Output type: ");
  Serial.println(output->type);

  if (input->type != kTfLiteInt8 || output->type != kTfLiteInt8) {
    Serial.println("Warning: This sketch expects full INT8 input/output.");
  }

  Serial.println("Setup complete.");
  Serial.println();
}

// ======================================================
// Main loop
// ======================================================
void loop() {
  // ------------------------------------------
  // 1. Read analog input
  // ------------------------------------------
  int adc = analogRead(LDR_PIN);

  // Normalize to 0..1
  float x = (float)adc / 4095.0f;
  x = clamp01(x);

  // ------------------------------------------
  // 2. Quantize input for INT8 model
  // q = round(x / scale) + zero_point
  // ------------------------------------------
  float in_scale = input->params.scale;
  int in_zero_point = input->params.zero_point;

  int q_input = (int)round(x / in_scale) + in_zero_point;
  input->data.int8[0] = clamp_int8(q_input);

  // ------------------------------------------
  // 3. Run inference
  // ------------------------------------------
  uint32_t t0 = micros();
  TfLiteStatus invoke_status = interpreter->Invoke();
  uint32_t t1 = micros();

  if (invoke_status != kTfLiteOk) {
    Serial.println("Inference failed.");
    delay(500);
    return;
  }

  // ------------------------------------------
  // 4. Dequantize output
  // y = (q - zero_point) * scale
  // ------------------------------------------
  float out_scale = output->params.scale;
  int out_zero_point = output->params.zero_point;

  float p[3];
  for (int i = 0; i < 3; i++) {
    int8_t q = output->data.int8[i];
    p[i] = ((int)q - out_zero_point) * out_scale;
  }

  // ------------------------------------------
  // 5. Classification
  // ------------------------------------------
  int cls = argmax3(p);
  const char* label = label_from_index(cls);

  // ------------------------------------------
  // 6. Print results
  // ------------------------------------------
  Serial.print("ADC = ");
  Serial.print(adc);

  Serial.print(" | x = ");
  Serial.print(x, 4);

  Serial.print(" | q_in = ");
  Serial.print((int)input->data.int8[0]);

  Serial.print(" | p = [");
  Serial.print(p[0], 3);
  Serial.print(", ");
  Serial.print(p[1], 3);
  Serial.print(", ");
  Serial.print(p[2], 3);
  Serial.print("]");

  Serial.print(" | class = ");
  Serial.print(label);

  Serial.print(" | latency = ");
  Serial.print((unsigned long)(t1 - t0));
  Serial.println(" us");

  delay(500);
}
