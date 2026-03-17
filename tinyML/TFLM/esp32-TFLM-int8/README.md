# ESP32 + TFLM INT8 + 1–8–3 MLP (LDR Classification)

This guide provides a complete **end-to-end INT8 quantized pipeline** for deploying a TinyML model on ESP32 using TensorFlow Lite for Microcontrollers (TFLM).

It includes:

- Python: train + full integer quantization → `model_int8.tflite`
- Convert `.tflite` → `model_data_int8.h`
- ESP32 Arduino sketch: INT8 input + INT8 output dequantization
- Smaller and faster than float32 deployment

---

## 1️⃣ Python: Train + Export INT8 `model_int8.tflite`

Save as `train_export_1_8_3_int8.py`

```python
import numpy as np
import tensorflow as tf

# ------------------ 1) Dataset (replace with your real LDR samples if you have them) ------------------
num_samples = 3000

x_dark   = np.random.uniform(0.00, 0.33, num_samples // 3)
x_normal = np.random.uniform(0.33, 0.66, num_samples // 3)
x_bright = np.random.uniform(0.66, 1.00, num_samples // 3)

X = np.concatenate([x_dark, x_normal, x_bright]).astype(np.float32).reshape(-1, 1)
y = np.concatenate([
    np.zeros(num_samples // 3, dtype=np.int32),
    np.ones(num_samples // 3, dtype=np.int32),
    np.full(num_samples // 3, 2, dtype=np.int32),
])

idx = np.random.permutation(len(X))
X, y = X[idx], y[idx]

# ------------------ 2) Model 1-8-3 ------------------
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(1,), name="x"),
    tf.keras.layers.Dense(8, activation="relu", name="dense_8"),
    tf.keras.layers.Dense(3, activation="softmax", name="dense_3")
])

model.compile(optimizer=tf.keras.optimizers.Adam(0.01),
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])
model.fit(X, y, epochs=30, batch_size=32, validation_split=0.2, verbose=1)

# ------------------ 3) Full Integer Quantization ------------------
def representative_dataset():
    # Use real samples if possible. Here we use training samples as a placeholder.
    for i in range(300):
        yield [X[i:i+1]]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset

# Force full-int8 model (input/output int8 too)
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_int8 = converter.convert()

with open("model_int8.tflite", "wb") as f:
    f.write(tflite_int8)

print("Saved: model_int8.tflite")
```

Run:

```bash
python train_export_1_8_3_int8.py
```

---

## 2️⃣ Convert `model_int8.tflite` → `model_data_int8.h`

### Option A: `xxd`

```bash
xxd -i model_int8.tflite > model_data_int8.h
```

### Option B: Python converter

```bash
python tflite_to_c.py model_int8.tflite model_data_int8.h
```

---

## 3️⃣ ESP32 Arduino Sketch (TFLM INT8) + LDR on GPIO36

Save as `esp32_tflm_ldr_1_8_3_int8.ino`

```cpp
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
    p[i] = (float)((int)q - out_zero) * out_scale;
  }

  int cls = argmax3_float(p);
  const char* label = label_from_idx(cls);

  Serial.printf("ADC=%d x=%.3f | q_in=%d (scale=%.6f zp=%d) | "
                "p=[%.3f %.3f %.3f] => %s | latency=%lu us\n",
                adc, x, (int)input->data.int8[0], in_scale, in_zero,
                p[0], p[1], p[2], label, (unsigned long)(t1 - t0));

  delay(200);
}
```

---

## 📌 Notes for Real Accuracy

### ✅ Best practice: use a representative dataset from real LDR readings

In Python, replace `representative_dataset()` with **real samples measured from your ESP32** (normalized to 0..1). That makes INT8 quantization much more accurate.

### ✅ If `AllocateTensors()` fails

Increase:

```cpp
constexpr int kTensorArenaSize = 16 * 1024;
```

---

## ✅ Why INT8?

Compared with float32:

- Smaller model size
- Lower memory usage
- Faster inference
- Better suited for ESP32 edge deployment

---

## 🚀 Summary

This workflow gives you a complete **INT8 TinyML deployment pipeline** for:

- ESP32
- LDR sensor on GPIO36
- 1–8–3 MLP
- 3-class output: **dark / normal / bright**

It is a strong baseline for:

- Smart lighting
- CPS edge intelligence
- TinyML labs
- IoT classification systems
