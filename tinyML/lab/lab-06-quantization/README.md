# Lab 5: Model Quantization and Memory Optimization  
## Efficient TinyML on ESP32 (Accuracy vs Efficiency Trade-off)  
### CPS Focus: C4 — Cognition (with Optimization)

---

## 1. Lab Overview

This laboratory focuses on **model quantization**, one of the most important techniques for **TinyML deployment on microcontrollers**.

Students will convert the **floating-point model from Lab 4** into a **quantized INT8 model**, deploy it on the **ESP32**, and compare:

- Flash memory usage  
- RAM consumption  
- Inference latency  
- Classification accuracy  

Lab 5 highlights a core TinyML principle:

> **Efficiency is as important as accuracy.**

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Explain why **quantization** is essential in TinyML  
- Understand **INT8 vs floating-point** representations  
- Deploy a **quantized model** on ESP32  
- Measure **Flash, RAM, and inference latency**  
- Analyze **accuracy–efficiency trade-offs**

---

## 3. Background Theory

### 3.1 Why Quantization?

Microcontrollers such as the ESP32 have:

- Limited RAM (≈ 520 KB)  
- Limited Flash (≈ 4 MB)  
- No GPU and limited floating-point performance  

Quantization converts:

```
float32  →  int8
32 bits  →  8 bits per parameter
```

This results in:
- ~4× smaller model size  
- Faster inference  
- Lower power consumption  

---

### 3.2 Quantization Concept

Real-valued parameters are approximated by integers:

```
x_real → x_int8 = (x_real / scale) + zero_point
```

During inference:

```
x_real ≈ scale × (x_int8 − zero_point)
```

This approximation is sufficiently accurate for many TinyML applications.

---

## 4. CPS Perspective

### CPS Mapping for Lab 5

| CPS Layer | Role |
|---------|------|
| Physical | Sensor input |
| Cyber | Feature extraction |
| Cognition | Quantized ML inference |
| Actuation | Output control |
| Optimization | Memory and latency tuning |

**Lab 5 optimizes the Cognition layer for embedded deployment.**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Same sensor setup as Lab 4  
- LED or other actuator  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor  
- (Optional) TensorFlow Lite for Microcontrollers  

---

## 6. Model Comparison Setup

Two versions of the same classifier are used:

| Model Type | Precision |
|----------|-----------|
| Baseline | Float32 |
| Optimized | INT8 (Quantized) |

Both models:
- Use the same feature set (Lab 3)  
- Perform the same classification task (Lab 4)  

---

## 7. Quantized Model Representation

### 7.1 Quantized Model Header

```cpp
// your_model_int8.h

#ifndef TINYML_MODEL_INT8_H
#define TINYML_MODEL_INT8_H

#define INPUT_SIZE 6
#define NUM_CLASSES 3

extern const int8_t weights_int8[NUM_CLASSES][INPUT_SIZE];
extern const int32_t bias_int32[NUM_CLASSES];
extern const float scale;
extern const int zero_point;

int tinyml_predict_int8(float *features);

#endif
```

---

### 7.2 INT8 Inference Function

```cpp
int tinyml_predict_int8(float *x) {
  int32_t scores[NUM_CLASSES] = {0};

  for (int i = 0; i < NUM_CLASSES; i++) {
    scores[i] = bias_int32[i];
    for (int j = 0; j < INPUT_SIZE; j++) {
      int8_t qx = (int8_t)(x[j] / scale) + zero_point;
      scores[i] += qx * weights_int8[i][j];
    }
  }

  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scores[i] > scores[best]) best = i;

  return best;
}
```

This implementation uses **integer-dominant computation**, which is significantly faster on ESP32.

---

## 8. Lab Tasks

### Task 1: Measure Model Memory Usage

Compile and record memory usage for each model:

```
Sketch uses XXXX bytes (X%) of program storage space.
Global variables use YYYY bytes (Y%) of dynamic memory.
```

Record:
- Flash usage  
- RAM usage  

---

### Task 2: Measure Inference Latency

```cpp
unsigned long t0 = micros();
int pred = tinyml_predict_int8(features);
unsigned long t1 = micros();

Serial.print("Inference time (us): ");
Serial.println(t1 - t0);
```

Repeat for both **float** and **INT8** models.

---

### Task 3: Accuracy Comparison

Evaluate both models under identical conditions:
- Same environment  
- Same feature input  
- Same test duration  

Record:
- Correct predictions  
- Misclassifications  

---

## 9. Results Analysis

Students should summarize results in a comparison table:

| Metric | Float Model | INT8 Model |
|------|-------------|------------|
| Flash size |  |  |
| RAM usage |  |  |
| Inference time (µs) |  |  |
| Accuracy (%) |  |  |

---

## 10. Observations and Discussion

Key discussion points:
- How much memory was saved?  
- How much faster is inference?  
- Did accuracy degrade significantly?  

**Important Insight:**  
> A slightly less accurate model that runs efficiently and reliably is often better for CPS than a perfect but impractical model.

---

## 11. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Quantization | Correct INT8 implementation | 30% |
| Measurement | Memory and latency recorded | 20% |
| Comparison | Clear float vs INT8 analysis | 20% |
| Functionality | Correct inference behavior | 20% |
| Reflection | Insightful discussion | 10% |

---

## 12. Student Reflection Questions

- Why is INT8 preferred over float in TinyML?  
- When might quantization hurt performance?  
- Would you deploy a less accurate but faster model? Why?

---

## 13. Preparation for Next Lab

In **Lab 6**, students will:
- Implement **sliding window streaming inference**  
- Perform **continuous TinyML decisions**  
- Analyze **latency vs responsiveness trade-offs**

---

## 14. Key Takeaway

**Lab 5 makes TinyML practical:**  
> Quantization enables intelligent models to fit within the tight limits of microcontrollers.
