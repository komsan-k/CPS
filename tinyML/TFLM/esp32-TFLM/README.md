# ESP32 + TFLM + 1–8–3 MLP (LDR Classification)

This is a complete **end-to-end pipeline** for deploying a TinyML model on ESP32 using TensorFlow Lite for Microcontrollers (TFLM).

## 🔧 Pipeline Overview

1. Python training → model.tflite  
2. Convert → model_data.h  
3. Deploy → ESP32 inference  

---

## 1️⃣ Python: Train Model

```python
import numpy as np
import tensorflow as tf

num_samples = 3000

x_dark   = np.random.uniform(0.00, 0.33, num_samples // 3)
x_normal = np.random.uniform(0.33, 0.66, num_samples // 3)
x_bright = np.random.uniform(0.66, 1.00, num_samples // 3)

X = np.concatenate([x_dark, x_normal, x_bright]).astype(np.float32).reshape(-1, 1)
y = np.concatenate([
    np.zeros(num_samples // 3),
    np.ones(num_samples // 3),
    np.full(num_samples // 3, 2)
])

model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation="relu", input_shape=(1,)),
    tf.keras.layers.Dense(3, activation="softmax")
])

model.compile(optimizer="adam", loss="sparse_categorical_crossentropy")
model.fit(X, y, epochs=30)

converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

open("model.tflite", "wb").write(tflite_model)
```

---

## 2️⃣ Convert to C Header

```bash
xxd -i model.tflite > model_data.h
```

---

## 3️⃣ ESP32 TFLM Code

```cpp
#include <Arduino.h>
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h"

const int LDR_PIN = 36;

constexpr int tensor_arena_size = 10 * 1024;
uint8_t tensor_arena[tensor_arena_size];

tflite::MicroErrorReporter error_reporter;
const tflite::Model* model;
tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

void setup() {
  Serial.begin(115200);

  model = tflite::GetModel(g_model);

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, tensor_arena_size, &error_reporter);

  interpreter = &static_interpreter;
  interpreter->AllocateTensors();

  input = interpreter->input(0);
  output = interpreter->output(0);
}

void loop() {
  float x = analogRead(LDR_PIN) / 4095.0;

  input->data.f[0] = x;

  uint32_t t0 = micros();
  interpreter->Invoke();
  uint32_t t1 = micros();

  float* p = output->data.f;

  int cls = 0;
  if (p[1] > p[cls]) cls = 1;
  if (p[2] > p[cls]) cls = 2;

  const char* labels[] = {"dark", "normal", "bright"};

  Serial.printf("x=%.3f => %s | %lu us\n",
                x, labels[cls], (unsigned long)(t1 - t0));

  delay(200);
}
```

---

## ⚠️ Common Issues

- Increase tensor_arena_size if allocation fails  
- Ensure model is float32  
- Use real data for better accuracy  

---

## ✅ Summary

ESP32 + TFLM enables real-time classification for CPS and IoT applications.

