# Lab 3: Feature Engineering on ESP32  
## From Raw Signals to Meaningful Features  
### CPS Focus: C3 — Cyber

---

## 1. Lab Overview

This laboratory introduces **feature engineering**, a critical step between raw sensor data and TinyML models.

Students will transform **time-series analog data** (from Lab 2) into **compact, informative features** that can be efficiently processed on an **ESP32**.

Lab 3 corresponds to **CPS Level C3 (Cyber)**, where computation, signal processing, and abstraction occur before intelligent decision-making.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Explain the role of **feature engineering** in TinyML  
- Implement a **sliding window buffer** on ESP32  
- Compute common **statistical features** on-device  
- Construct **feature vectors** for ML inference  
- Evaluate trade-offs between **feature richness and resource usage**

---

## 3. Background Theory

### 3.1 Why Feature Engineering Matters in TinyML

TinyML systems operate under strict constraints:

- Limited RAM and Flash  
- Often no floating-point acceleration  
- Real-time inference requirements  

Instead of feeding raw data directly, features are extracted to:

- Reduce dimensionality  
- Capture essential signal characteristics  
- Improve accuracy with smaller models  

> **Good features enable simple models to perform well.**

---

### 3.2 Time-Series Windowing

Sensor data is inherently temporal.  
Rather than using a single sample, TinyML systems operate on a **window of samples**:

```
x[t-N+1], x[t-N+2], ..., x[t]
```

From each window, features such as **mean, variance, RMS, and slope** are computed.

---

## 4. CPS Perspective (C3: Cyber)

### CPS Mapping for Lab 3

| CPS Layer | Role |
|---------|------|
| Physical | Analog sensor (LDR, potentiometer, etc.) |
| Conversion | ADC sampling |
| Cyber | Feature extraction |
| Decision | ML model (next labs) |
| Actuation | Indicator / actuator |

**Lab 3 focuses entirely on cyber computation and abstraction.**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Analog sensor (from Lab 2: LDR or potentiometer)  
- Breadboard and jumper wires  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor / Serial Plotter  

---

## 6. System Architecture

```
[Analog Sensor]
       ↓
     ADC
       ↓
 Sliding Window Buffer
       ↓
 Feature Extraction
       ↓
  Feature Vector
       ↓
 Serial Output (Dataset)
```

This architecture mirrors the **input pipeline of a TinyML system**.

---

## 7. Feature Set Used in This Lab

| Feature | Meaning |
|-------|--------|
| Mean | Average signal level |
| Min | Lowest value in window |
| Max | Highest value in window |
| Variance | Signal fluctuation |
| RMS | Signal energy |
| Slope | Trend (increasing / decreasing) |

These features are:
- Easy to compute  
- Interpretable  
- Widely used in TinyML  

---

## 8. Lab Tasks

### Task 1: Implement a Sliding Window Buffer

```cpp
#define WINDOW_SIZE 20
int window[WINDOW_SIZE];
int idx = 0;

void addSample(int sample) {
  window[idx++] = sample;
  if (idx >= WINDOW_SIZE) idx = 0;
}
```

This buffer stores the most recent sensor samples.

---

### Task 2: Compute Statistical Features

```cpp
float computeMean() {
  long sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++)
    sum += window[i];
  return sum / (float)WINDOW_SIZE;
}

float computeVariance(float mean) {
  float var = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float diff = window[i] - mean;
    var += diff * diff;
  }
  return var / WINDOW_SIZE;
}
```

---

### Task 3: Compute Min, Max, and RMS

```cpp
int computeMin() {
  int m = window[0];
  for (int i = 1; i < WINDOW_SIZE; i++)
    if (window[i] < m) m = window[i];
  return m;
}

int computeMax() {
  int m = window[0];
  for (int i = 1; i < WINDOW_SIZE; i++)
    if (window[i] > m) m = window[i];
  return m;
}

float computeRMS() {
  long sumSq = 0;
  for (int i = 0; i < WINDOW_SIZE; i++)
    sumSq += window[i] * window[i];
  return sqrt(sumSq / (float)WINDOW_SIZE);
}
```

---

### Task 4: Trend (Slope) Feature

```cpp
float computeSlope() {
  return (window[WINDOW_SIZE - 1] - window[0]) / (float)WINDOW_SIZE;
}
```

This feature captures **increasing or decreasing trends**.

---

### Task 5: Output Feature Vector

```cpp
void printFeatures() {
  float mean = computeMean();
  Serial.print(mean); Serial.print(",");
  Serial.print(computeMin()); Serial.print(",");
  Serial.print(computeMax()); Serial.print(",");
  Serial.print(computeVariance(mean)); Serial.print(",");
  Serial.print(computeRMS()); Serial.print(",");
  Serial.println(computeSlope());
}
```

The output can be logged as a **feature-level dataset**.

---

## 9. Observations and Discussion

Students should analyze:
- Raw data vs feature stability  
- Effect of window size on responsiveness and latency  
- Which features best describe sensor behavior  

**Key Insight:**  
> TinyML models learn patterns from features, not raw signals.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Windowing | Correct sliding buffer | 20% |
| Features | Correct computation | 30% |
| Efficiency | Runs smoothly on ESP32 | 20% |
| Output | Valid feature vector | 20% |
| Explanation | CPS relevance | 10% |

---

## 11. Student Reflection Questions

- Why are features preferred over raw signals in TinyML?  
- How does window size affect latency and accuracy?  
- Which features would you remove to save memory?  

---

## 12. Preparation for Next Lab

In **Lab 4**, students will:
- Train a TinyML classifier on feature data  
- Convert the trained model to **C/C++**  
- Deploy the model on ESP32 for real inference  

---

## 13. Key Takeaway

**Lab 3 creates intelligence-ready data:**  
> Feature engineering bridges the physical world and TinyML decision-making.
