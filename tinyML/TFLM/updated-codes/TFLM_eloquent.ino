#include <Arduino.h>
// #include "ldr_model_int8.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

#include "ldr_model_float32.h"


#define ARENA_SIZE 8000
#define TF_NUM_OPS 2
#define MODEL_DATA g_model
#define NUM_INPUTS 1
#define NUM_OUTPUTS 3

Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

const char *CLASS_NAMES[NUM_OUTPUTS] = {
  "Dark (expected 0.80-0.88)",
  "Normal (expected 0.90-0.98)",
  "Bright (expected 0.99-1.00)"
};

float randomFloat(float minVal, float maxVal) {
  long r = random(0, 10000);
  float t = r / 9999.0f;
  return minVal + t * (maxVal - minVal);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  randomSeed(micros());

  tf.setNumInputs(NUM_INPUTS);
  tf.setNumOutputs(NUM_OUTPUTS);
  tf.resolver.AddFullyConnected();
  tf.resolver.AddSoftmax();

  while (!tf.begin(MODEL_DATA).isOk()) {
    Serial.println(tf.exception.toString());
    delay(1000);
  }
}

void loop() {
  //float x = randomFloat(0.75f, 1.05f);

  int adc = analogRead(36);

  // Normalize to [0,1]
  float x = (float)1.0f - (adc / 4095.0f);
  
  float input[1] = {x};

  if (!tf.predict(input).isOk()) {
    Serial.println(tf.exception.toString());
    delay(1000);
    return;
  }

  Serial.println();
  Serial.print("Input: ");
  Serial.println(x, 4);

  Serial.print("  Predicted class: ");
  Serial.print(tf.classification);
  Serial.print(" (");
  if (tf.classification >= 0 && tf.classification < NUM_OUTPUTS) {
    Serial.print(CLASS_NAMES[tf.classification]);
  }
  else {
    Serial.print("Unknown");
  }
  Serial.println(")");

  Serial.print("  Inference time: ");
  Serial.print(tf.benchmark.microseconds());
  Serial.println(" us");

  delay(1000);
}

// void loop() {
//   float x = randomFloat(0.75f, 1.05f);
//   float input[1] = {x};

//   if (!tf.predict(input).isOk()) {
//     Serial.println(tf.exception.toString());
//     delay(1000);
//     return;
//   }

//   Serial.println();
//   Serial.print("Input: ");
//   Serial.println(x, 4);

//   Serial.print("  Output probabilities: [");
//   for (int i = 0; i < NUM_OUTPUTS; i++) {
//     Serial.print(tf.result(i), 6);
//     if (i < NUM_OUTPUTS - 1) Serial.print(", ");
//   }
//   Serial.println("]");

//   Serial.print("  Predicted class: ");
//   Serial.print(tf.classification);
//   Serial.print(" (");
//   Serial.print(CLASS_NAMES[tf.classification]);
//   Serial.println(")");

//   Serial.print("  Inference time: ");
//   Serial.print(tf.benchmark.microseconds());
//   Serial.println(" us");

//   delay(1000);
// }
