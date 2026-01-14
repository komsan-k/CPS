# Lab 6: Streaming Inference with Sliding Windows  
## Real-Time TinyML Decisions on ESP32 (Latency vs Responsiveness)  
### CPS Focus: C4 → C5 (Cognition → Configuration)

---

## 1. Lab Overview

This laboratory advances TinyML from **batch-style inference** to **continuous, real-time inference** using **sliding windows**.

Students will implement a **streaming pipeline on the ESP32** that continuously:
- Acquires sensor data  
- Updates a window buffer  
- Extracts features  
- Runs TinyML inference  
- Updates an actuator  

All operations run **without stopping the system**, reflecting real Cyber-Physical Systems (CPS).

Lab 6 emphasizes **temporal behavior, latency, and responsiveness**, which are critical for real-time CPS operation.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Implement a **circular buffer** for streaming sensor data  
- Perform **window-based feature extraction** continuously  
- Execute **periodic TinyML inference** on ESP32  
- Measure **end-to-end inference latency**  
- Analyze the trade-off between **window size, overlap, and responsiveness**

---

## 3. Background Theory

### 3.1 From Batch to Streaming TinyML

In earlier labs:
- Data was collected → processed → inferred (**batch mode**)

In real CPS:
- Data arrives **continuously**
- Decisions must be made **while data is flowing**

**Streaming TinyML** enables:
- Online monitoring  
- Real-time detection  
- Continuous control  

---

### 3.2 Sliding Window Concept

A sliding window contains the most recent **N samples**:

```
x[t−N+1], x[t−N+2], … , x[t]
```

After each new sample:
- Oldest sample is discarded  
- New sample is added  
- Features are recomputed  
- Inference may be triggered  

**Key parameters:**
- Window size (N)  
- Step size (S)  
- Inference period  

---

## 4. CPS Perspective

### CPS Mapping for Lab 6

| CPS Layer | Role |
|---------|------|
| Physical | Continuous sensor signal |
| Conversion | ADC sampling |
| Cyber | Sliding window & feature extraction |
| Cognition | Streaming TinyML inference |
| Actuation | Real-time response |

This lab integrates **all CPS layers continuously**.

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Analog sensor (LDR / potentiometer / vibration sensor)  
- LED or buzzer (actuator)

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor / Serial Plotter  

---

## 6. System Architecture

```
[Sensor Stream]
       ↓
     ADC
       ↓
 Circular Buffer (Window)
       ↓
 Feature Extraction
       ↓
 TinyML Inference
       ↓
 Actuator Update
       ↓
 (Repeat continuously)
```

This loop runs **non-stop** on the ESP32.

---

## 7. Lab Tasks

### Task 1: Circular Buffer for Streaming Data

```cpp
#define WINDOW_SIZE 20
int window[WINDOW_SIZE];
int head = 0;

void addSample(int sample) {
  window[head] = sample;
  head = (head + 1) % WINDOW_SIZE;
}
```

This ensures **constant-time updates**.

---

### Task 2: Periodic Sampling (Non-Blocking)

```cpp
unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_PERIOD = 20; // ms

void loop() {
  if (millis() - lastSampleTime >= SAMPLE_PERIOD) {
    lastSampleTime = millis();
    int sample = analogRead(SENSOR_PIN);
    addSample(sample);
  }
}
```

Using non-blocking timing avoids `delay()` and improves responsiveness.

---

### Task 3: Streaming Feature Extraction

```cpp
bool windowReady = false;
int sampleCount = 0;

void updateWindow(int sample) {
  addSample(sample);
  sampleCount++;
  if (sampleCount >= WINDOW_SIZE)
    windowReady = true;
}
```

Features from **Lab 3** are reused and recomputed continuously.

---

### Task 4: Periodic Inference

```cpp
unsigned long lastInferTime = 0;
const unsigned long INFER_PERIOD = 200; // ms

void tryInference() {
  if (windowReady && millis() - lastInferTime >= INFER_PERIOD) {
    lastInferTime = millis();

    float features[6];
    extractFeatures(features);   // mean, min, max, etc.
    int label = tinyml_predict_int8(features);

    updateActuator(label);
  }
}
```

Inference runs **slower than sampling**, saving energy.

---

### Task 5: End-to-End Latency Measurement

```cpp
unsigned long t0 = micros();
int label = tinyml_predict_int8(features);
unsigned long t1 = micros();

Serial.print("Inference latency (us): ");
Serial.println(t1 - t0);
```

Students record **latency distributions** over time.

---

## 8. Observations and Discussion

Students should investigate:
- Effect of window size on delay  
- Stability of predictions over time  
- Responsiveness to rapid changes  

**Key Insight:**  
> Smaller windows react faster but are noisier; larger windows are more stable but slower.

---

## 9. Performance Trade-Off Study

Students compare configurations such as:

| Parameter | Case A | Case B |
|---------|--------|--------|
| Window size | 10 | 30 |
| Inference period | 100 ms | 300 ms |
| Latency |  |  |
| Stability |  |  |

This builds intuition for **system tuning**.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Streaming buffer | Correct circular buffer | 20% |
| Scheduling | Non-blocking sampling & inference | 20% |
| Inference | Correct continuous predictions | 25% |
| Latency | Measured and analyzed | 20% |
| Discussion | Trade-off analysis | 15% |

---

## 11. Student Reflection Questions

- Why should sampling and inference rates differ?  
- How does window size affect CPS safety?  
- Where would streaming TinyML be critical in real life?

---

## 12. Preparation for Next Lab

In **Lab 7**, students will:
- Apply streaming TinyML to **motion or vibration data**  
- Perform **activity recognition**  
- Handle **multi-axis sensor inputs**

---

## 13. Key Takeaway

**Lab 6 makes TinyML alive:**  
> Intelligence now runs continuously, reacting to the physical world in real time.
