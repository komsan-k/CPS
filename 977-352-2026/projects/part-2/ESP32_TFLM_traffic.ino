/*
  ESP32_TFLM_traffic.ino
  ------------------------------------------------------------
  Smart traffic light controller using TensorFlow Lite for
  Microcontrollers (TFLM) on ESP32.

  Intersection:
  - East-West (EW) = major road
  - North-South (NS) = minor road
  - 2 sensors per road (4 total)

  Model input features:
  [ ew1, ew2, ns1, ns2, ew_queue, ns_queue, phase, green_time ]

  Output classes:
  0 = keep EW green
  1 = switch to NS
  2 = keep NS green
  3 = switch to EW

  Notes:
  - This sketch expects a model header named: model_data.h
  - The model should be an INT8 TFLite model converted to a C array
  - Safety logic remains outside the ML model
*/

#include <Arduino.h>

#include <tflm_esp32.h> 

// ---- TFLM ----
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// ---- Exported TFLite model ----
#include "model_data.h"   // must provide: g_model[]

// -----------------------------
// Pin assignment
// -----------------------------
// Traffic sensors
static const int EW_S1_PIN = 14;   // upstream EW
static const int EW_S2_PIN = 35;   // stop-line EW
static const int NS_S1_PIN = 16;   // upstream NS
static const int NS_S2_PIN = 33;   // stop-line NS

// Traffic lights
static const int EW_RED_PIN    = 23;
static const int EW_YELLOW_PIN = 19;
static const int EW_GREEN_PIN  = 18;

static const int NS_RED_PIN    = 2;
static const int NS_YELLOW_PIN = 13;
static const int NS_GREEN_PIN  = 12;

// -----------------------------
// Safety timing (milliseconds)
// -----------------------------
static const unsigned long MIN_GREEN_MS = 5000;
static const unsigned long MAX_GREEN_MS = 30000;
static const unsigned long YELLOW_MS    = 2000;
static const unsigned long ALL_RED_MS   = 1000;

// -----------------------------
// Phase definition
// -----------------------------
enum PhaseState {
  PHASE_EW_GREEN = 0,
  PHASE_NS_GREEN = 1
};

PhaseState currentPhase = PHASE_EW_GREEN;
unsigned long phaseStartMs = 0;
unsigned long nsWaitingStartMs = 0;

// -----------------------------
// TFLM globals
// -----------------------------
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 12 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Op resolver: Dense/FullyConnected + Softmax are enough for a small MLP
tflite::MicroMutableOpResolver<2> resolver;

// -----------------------------
// Helper functions
// -----------------------------
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

// Read digital-like occupancy from ADC pin.
// Change threshold or preprocessing to match real sensors.
float readOccupancy01(int pin, int threshold = 1800) {
  int raw = analogRead(pin);  // 0..4095
  return (raw > threshold) ? 1.0f : 0.0f;
}

float weightedQueue(float s1, float s2) {
  // stop-line sensor is slightly more important
  return clamp01(0.4f * s1 + 0.6f * s2);
}

float elapsedGreenSec() {
  return (millis() - phaseStartMs) / 1000.0f;
}

float nsWaitingSec() {
  if (currentPhase == PHASE_NS_GREEN) return 0.0f;
  return (millis() - nsWaitingStartMs) / 1000.0f;
}

void setLights(bool ewR, bool ewY, bool ewG, bool nsR, bool nsY, bool nsG) {
  digitalWrite(EW_RED_PIN, ewR);
  digitalWrite(EW_YELLOW_PIN, ewY);
  digitalWrite(EW_GREEN_PIN, ewG);

  digitalWrite(NS_RED_PIN, nsR);
  digitalWrite(NS_YELLOW_PIN, nsY);
  digitalWrite(NS_GREEN_PIN, nsG);
}

void setEWGreen() {
  setLights(false, false, true, true, false, false);
  currentPhase = PHASE_EW_GREEN;
  phaseStartMs = millis();
}

void setNSGreen() {
  setLights(true, false, false, false, false, true);
  currentPhase = PHASE_NS_GREEN;
  phaseStartMs = millis();
  nsWaitingStartMs = millis(); // reset wait timer when NS gets service
}

void transitionEWtoNS() {
  // EW yellow
  setLights(false, true, false, true, false, false);
  delay(YELLOW_MS);

  // all red
  setLights(true, false, false, true, false, false);
  delay(ALL_RED_MS);

  setNSGreen();
}

void transitionNStoEW() {
  // NS yellow
  setLights(true, false, false, false, true, false);
  delay(YELLOW_MS);

  // all red
  setLights(true, false, false, true, false, false);
  delay(ALL_RED_MS);

  setEWGreen();
}

int argmaxInt8(const int8_t* arr, int n) {
  int idx = 0;
  int8_t best = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] > best) {
      best = arr[i];
      idx = i;
    }
  }
  return idx;
}

bool initTFLM() {
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch.");
    return false;
  }

  resolver.AddFullyConnected();
  resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize, error_reporter
  );
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed.");
    return false;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  if (input == nullptr || output == nullptr) {
    Serial.println("Failed to get input/output tensor.");
    return false;
  }

  Serial.println("TFLM initialized.");
  return true;
}

// Quantize a float feature into int8 using tensor scale/zero-point
int8_t quantizeFeature(float x, float scale, int zero_point) {
  int32_t q = (int32_t)roundf(x / scale) + zero_point;
  if (q < -128) q = -128;
  if (q > 127) q = 127;
  return (int8_t)q;
}

int runInference(
  float ew1, float ew2, float ns1, float ns2,
  float ewQueue, float nsQueue,
  float phase, float greenTimeNorm
) {
  if (input->type != kTfLiteInt8) {
    Serial.println("Expected INT8 input model.");
    return -1;
  }

  const float scale = input->params.scale;
  const int zero = input->params.zero_point;

  float features[8] = {
    ew1, ew2, ns1, ns2, ewQueue, nsQueue, phase, greenTimeNorm
  };

  for (int i = 0; i < 8; i++) {
    input->data.int8[i] = quantizeFeature(features[i], scale, zero);
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed.");
    return -1;
  }

  int outSize = output->dims->data[output->dims->size - 1];
  if (outSize < 4) {
    Serial.println("Unexpected output size.");
    return -1;
  }

  return argmaxInt8(output->data.int8, outSize);
}

void printStatus(
  float ew1, float ew2, float ns1, float ns2,
  float ewQueue, float nsQueue, int action
) {
  Serial.print("EW(");
  Serial.print(ew1); Serial.print(",");
  Serial.print(ew2); Serial.print(")  ");

  Serial.print("NS(");
  Serial.print(ns1); Serial.print(",");
  Serial.print(ns2); Serial.print(")  ");

  Serial.print("EWq=");
  Serial.print(ewQueue, 2);
  Serial.print(" NSq=");
  Serial.print(nsQueue, 2);

  Serial.print(" Phase=");
  Serial.print((currentPhase == PHASE_EW_GREEN) ? "EW" : "NS");

  Serial.print(" GreenTime=");
  Serial.print(elapsedGreenSec(), 1);

  Serial.print("s Action=");
  Serial.println(action);
}

void applyDecision(int action) {
  unsigned long elapsed = millis() - phaseStartMs;

  // Safety rule 1: minimum green must pass
  if (elapsed < MIN_GREEN_MS) return;

  // Safety rule 2: maximum green forces a switch
  if (elapsed >= MAX_GREEN_MS) {
    if (currentPhase == PHASE_EW_GREEN) transitionEWtoNS();
    else transitionNStoEW();
    return;
  }

  // ML-assisted switching
  if (currentPhase == PHASE_EW_GREEN) {
    // Relevant actions while EW is green:
    // 0 keep EW, 1 switch to NS
    if (action == 1) transitionEWtoNS();
  } else {
    // Relevant actions while NS is green:
    // 2 keep NS, 3 switch to EW
    if (action == 3) transitionNStoEW();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(EW_RED_PIN, OUTPUT);
  pinMode(EW_YELLOW_PIN, OUTPUT);
  pinMode(EW_GREEN_PIN, OUTPUT);

  pinMode(NS_RED_PIN, OUTPUT);
  pinMode(NS_YELLOW_PIN, OUTPUT);
  pinMode(NS_GREEN_PIN, OUTPUT);

  analogReadResolution(12);

  // Initial traffic state
  setEWGreen();
  nsWaitingStartMs = millis();

  if (!initTFLM()) {
    Serial.println("TFLM init failed. System will still run with fixed fallback behavior.");
  }
}

void loop() {
  // 1) Read sensors
  float ew1 = readOccupancy01(EW_S1_PIN);
  float ew2 = readOccupancy01(EW_S2_PIN);
  float ns1 = readOccupancy01(NS_S1_PIN);
  float ns2 = readOccupancy01(NS_S2_PIN);

  // 2) Compute features
  float ewQueue = weightedQueue(ew1, ew2);
  float nsQueue = weightedQueue(ns1, ns2);

  // phase feature: 0 = EW green, 1 = NS green
  float phaseFeature = (currentPhase == PHASE_EW_GREEN) ? 0.0f : 1.0f;

  // normalize green time to 0..1 using MAX_GREEN_MS
  float greenTimeNorm = clamp01((float)(millis() - phaseStartMs) / (float)MAX_GREEN_MS);

  // Optional fairness heuristic:
  // if NS is waiting while EW is green, keep tracking wait timer
  if (currentPhase == PHASE_EW_GREEN) {
    // do nothing: timer already running
  } else {
    nsWaitingStartMs = millis();
  }

  int action = -1;

  // 3) Run TFLM inference if model initialized
  if (interpreter != nullptr && input != nullptr && output != nullptr) {
    action = runInference(ew1, ew2, ns1, ns2, ewQueue, nsQueue, phaseFeature, greenTimeNorm);
  }

  // Fallback advisory if model is not available
  if (action < 0) {
    if (currentPhase == PHASE_EW_GREEN) {
      action = (nsQueue > 0.8f && elapsedGreenSec() > 8.0f) ? 1 : 0;
    } else {
      action = (ewQueue > 0.8f && elapsedGreenSec() > 8.0f) ? 3 : 2;
    }
  }

  printStatus(ew1, ew2, ns1, ns2, ewQueue, nsQueue, action);

  // 4) Apply safety rules + action
  applyDecision(action);

  // 5) Polling interval
  delay(250);
}
