/*
  ESP32_TFLM_traffic_float32_ped.ino
  ------------------------------------------------------------
  Smart traffic light controller using TensorFlow Lite for
  Microcontrollers (TFLM) on ESP32 with pedestrian button input.

  FLOAT32 model input features:
  [ ew1, ew2, ns1, ns2, ew_queue, ns_queue, phase, green_time, ped_request ]

  Output classes:
  0 = keep EW green
  1 = switch to NS
  2 = keep NS green
  3 = switch to EW

  Notes:
  - Expects a float32 model header named: traffic_model_float32_ped.h
  - Safety logic remains outside the ML model
  - Pedestrian request is advisory to switching logic, not direct unsafe control
*/

#include <Arduino.h>

// ----- TFLM -----
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// ----- Exported TFLite float32 model -----
#include "traffic_model_float32_ped.h"   // provides g_model[]

// ============================================================
// Pin assignment
// ============================================================

// Traffic sensors
static const int EW_S1_PIN = 34;   // EW upstream
static const int EW_S2_PIN = 35;   // EW stop-line
static const int NS_S1_PIN = 32;   // NS upstream
static const int NS_S2_PIN = 33;   // NS stop-line

// Pedestrian button
static const int PED_BUTTON_PIN = 4;  // use INPUT_PULLUP

// Traffic lights
static const int EW_RED_PIN    = 14;
static const int EW_YELLOW_PIN = 27;
static const int EW_GREEN_PIN  = 26;

static const int NS_RED_PIN    = 25;
static const int NS_YELLOW_PIN = 18;
static const int NS_GREEN_PIN  = 19;

// Optional pedestrian walk LED
static const int PED_WALK_PIN  = 5;

// ============================================================
// Timing parameters (milliseconds)
// ============================================================
static const unsigned long MIN_GREEN_MS   = 5000;
static const unsigned long MAX_GREEN_MS   = 30000;
static const unsigned long YELLOW_MS      = 2000;
static const unsigned long ALL_RED_MS     = 1000;
static const unsigned long PED_WALK_MS    = 5000;
static const unsigned long PED_DEBOUNCE_MS = 200;

// ============================================================
// Phase definition
// ============================================================
enum PhaseState {
  PHASE_EW_GREEN = 0,
  PHASE_NS_GREEN = 1
};

PhaseState currentPhase = PHASE_EW_GREEN;
unsigned long phaseStartMs = 0;

// pedestrian request latch
bool pedRequestLatched = false;
unsigned long lastPedPressMs = 0;

// ============================================================
// TFLM globals
// ============================================================
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

constexpr int kTensorArenaSize = 12 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Dense + Softmax are enough for small MLP
tflite::MicroMutableOpResolver<2> resolver;

// ============================================================
// Helper functions
// ============================================================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

// Simple ADC -> occupancy mapping
// Adjust threshold for your sensor hardware
float readOccupancy01(int pin, int threshold = 1800) {
  int raw = analogRead(pin);  // 0..4095
  return (raw > threshold) ? 1.0f : 0.0f;
}

float weightedQueue(float s1, float s2) {
  return clamp01(0.4f * s1 + 0.6f * s2);
}

float elapsedGreenSec() {
  return (millis() - phaseStartMs) / 1000.0f;
}

float elapsedGreenNorm() {
  return clamp01((float)(millis() - phaseStartMs) / (float)MAX_GREEN_MS);
}

void setLights(bool ewR, bool ewY, bool ewG,
               bool nsR, bool nsY, bool nsG,
               bool pedWalk) {
  digitalWrite(EW_RED_PIN, ewR);
  digitalWrite(EW_YELLOW_PIN, ewY);
  digitalWrite(EW_GREEN_PIN, ewG);

  digitalWrite(NS_RED_PIN, nsR);
  digitalWrite(NS_YELLOW_PIN, nsY);
  digitalWrite(NS_GREEN_PIN, nsG);

  digitalWrite(PED_WALK_PIN, pedWalk);
}

void setEWGreen() {
  setLights(false, false, true,
            true, false, false,
            false);
  currentPhase = PHASE_EW_GREEN;
  phaseStartMs = millis();
}

void setNSGreen() {
  setLights(true, false, false,
            false, false, true,
            false);
  currentPhase = PHASE_NS_GREEN;
  phaseStartMs = millis();
}

void allRed() {
  setLights(true, false, false,
            true, false, false,
            false);
}

void pedestrianWalkPhase() {
  // During pedestrian walk, keep all vehicle lights red
  allRed();
  digitalWrite(PED_WALK_PIN, HIGH);
  delay(PED_WALK_MS);
  digitalWrite(PED_WALK_PIN, LOW);
}

void transitionEWtoNS(bool servePedestrian) {
  // EW yellow
  setLights(false, true, false,
            true, false, false,
            false);
  delay(YELLOW_MS);

  // all red
  allRed();
  delay(ALL_RED_MS);

  if (servePedestrian && pedRequestLatched) {
    pedestrianWalkPhase();
    pedRequestLatched = false;
  }

  setNSGreen();
}

void transitionNStoEW(bool servePedestrian) {
  // NS yellow
  setLights(true, false, false,
            false, true, false,
            false);
  delay(YELLOW_MS);

  // all red
  allRed();
  delay(ALL_RED_MS);

  if (servePedestrian && pedRequestLatched) {
    pedestrianWalkPhase();
    pedRequestLatched = false;
  }

  setEWGreen();
}

int argmaxFloat(const float* arr, int n) {
  int idx = 0;
  float best = arr[0];
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

  if (input->type != kTfLiteFloat32) {
    Serial.println("Expected FLOAT32 input model.");
    return false;
  }

  if (output->type != kTfLiteFloat32) {
    Serial.println("Expected FLOAT32 output model.");
    return false;
  }

  Serial.println("TFLM float32 initialized.");
  return true;
}

int runInferenceFloat32(
  float ew1, float ew2, float ns1, float ns2,
  float ewQueue, float nsQueue,
  float phaseFeature, float greenTimeNorm,
  float pedRequest
) {
  if (input == nullptr || output == nullptr) return -1;
  if (input->type != kTfLiteFloat32) return -1;

  // 9 inputs
  input->data.f[0] = ew1;
  input->data.f[1] = ew2;
  input->data.f[2] = ns1;
  input->data.f[3] = ns2;
  input->data.f[4] = ewQueue;
  input->data.f[5] = nsQueue;
  input->data.f[6] = phaseFeature;
  input->data.f[7] = greenTimeNorm;
  input->data.f[8] = pedRequest;

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed.");
    return -1;
  }

  int outSize = output->dims->data[output->dims->size - 1];
  if (outSize < 4) {
    Serial.println("Unexpected output size.");
    return -1;
  }

  return argmaxFloat(output->data.f, outSize);
}

void readPedestrianButton() {
  // Button uses INPUT_PULLUP, so pressed = LOW
  int btn = digitalRead(PED_BUTTON_PIN);
  unsigned long now = millis();

  if (btn == LOW) {
    if (now - lastPedPressMs > PED_DEBOUNCE_MS) {
      pedRequestLatched = true;
      lastPedPressMs = now;
      Serial.println("Pedestrian request latched.");
    }
  }
}

void printStatus(
  float ew1, float ew2, float ns1, float ns2,
  float ewQueue, float nsQueue,
  float pedReq, int action
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

  Serial.print(" Green=");
  Serial.print(elapsedGreenSec(), 1);
  Serial.print("s");

  Serial.print(" Ped=");
  Serial.print(pedReq, 0);

  Serial.print(" Action=");
  Serial.println(action);
}

void applyDecision(int action) {
  unsigned long elapsed = millis() - phaseStartMs;

  // Safety 1: minimum green
  if (elapsed < MIN_GREEN_MS) return;

  // Safety 2: hard maximum green
  if (elapsed >= MAX_GREEN_MS) {
    if (currentPhase == PHASE_EW_GREEN) {
      transitionEWtoNS(true);
    } else {
      transitionNStoEW(true);
    }
    return;
  }

  // ML-assisted action
  if (currentPhase == PHASE_EW_GREEN) {
    // valid actions: 0 keep EW, 1 switch to NS
    if (action == 1) {
      transitionEWtoNS(true);
    }
  } else {
    // valid actions: 2 keep NS, 3 switch to EW
    if (action == 3) {
      transitionNStoEW(true);
    }
  }
}

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(EW_RED_PIN, OUTPUT);
  pinMode(EW_YELLOW_PIN, OUTPUT);
  pinMode(EW_GREEN_PIN, OUTPUT);

  pinMode(NS_RED_PIN, OUTPUT);
  pinMode(NS_YELLOW_PIN, OUTPUT);
  pinMode(NS_GREEN_PIN, OUTPUT);

  pinMode(PED_WALK_PIN, OUTPUT);
  pinMode(PED_BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12);

  setEWGreen();

  if (!initTFLM()) {
    Serial.println("TFLM init failed. Using fallback advisory logic.");
  }
}

// ============================================================
// Loop
// ============================================================
void loop() {
  // 1) Read pedestrian button
  readPedestrianButton();

  // 2) Read traffic sensors
  float ew1 = readOccupancy01(EW_S1_PIN);
  float ew2 = readOccupancy01(EW_S2_PIN);
  float ns1 = readOccupancy01(NS_S1_PIN);
  float ns2 = readOccupancy01(NS_S2_PIN);

  // 3) Compute features
  float ewQueue = weightedQueue(ew1, ew2);
  float nsQueue = weightedQueue(ns1, ns2);
  float phaseFeature = (currentPhase == PHASE_EW_GREEN) ? 0.0f : 1.0f;
  float greenTimeNorm = elapsedGreenNorm();
  float pedReq = pedRequestLatched ? 1.0f : 0.0f;

  int action = -1;

  // 4) ML inference
  if (interpreter != nullptr && input != nullptr && output != nullptr) {
    action = runInferenceFloat32(
      ew1, ew2, ns1, ns2,
      ewQueue, nsQueue,
      phaseFeature, greenTimeNorm,
      pedReq
    );
  }

  // 5) Fallback logic if model unavailable
  if (action < 0) {
    if (currentPhase == PHASE_EW_GREEN) {
      if (elapsedGreenSec() < 5.0f) {
        action = 0;
      } else if (pedReq > 0.5f) {
        action = 1;
      } else if (nsQueue > ewQueue + 0.2f) {
        action = 1;
      } else {
        action = 0;
      }
    } else {
      if (elapsedGreenSec() < 5.0f) {
        action = 2;
      } else if (pedReq > 0.5f) {
        action = 3;
      } else if (ewQueue > nsQueue + 0.2f) {
        action = 3;
      } else {
        action = 2;
      }
    }
  }

  // 6) Print debug
  printStatus(ew1, ew2, ns1, ns2, ewQueue, nsQueue, pedReq, action);

  // 7) Apply safety + action
  applyDecision(action);

  delay(250);
}
