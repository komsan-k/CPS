# Lab 8: Anomaly Detection Using TinyML  
## Normal-Only Learning for Fault and Abnormal Behavior Detection  
### CPS Focus: C4–C5 (Cognition → Configuration)

---

## 1. Lab Overview

This laboratory introduces **anomaly detection**, a powerful TinyML paradigm where the system learns **only normal behavior** and flags **abnormal or unexpected patterns** during operation.

Unlike classification (Labs 4–7), anomaly detection **does not require labeled fault data**, making it highly suitable for real-world **Cyber-Physical Systems (CPS)** where failures are rare, unknown, or unpredictable.

Students will implement a **TinyML-based anomaly detector on ESP32** using **statistical or lightweight reconstruction-based methods**, and trigger alerts when abnormal behavior is detected.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Explain the concept of **normal-only learning**  
- Collect and model **baseline (normal) sensor behavior**  
- Compute **anomaly scores** from streaming data  
- Set and tune **decision thresholds**  
- Deploy a **real-time fault/anomaly alert system** on ESP32  

---

## 3. Background Theory

### 3.1 What Is Anomaly Detection?

Anomaly detection identifies patterns that **deviate significantly from normal behavior**.

In CPS, anomalies may indicate:
- Mechanical faults  
- Sensor failure  
- Abnormal usage  
- Security or safety issues  

**Key Idea:**  
> Normal behavior is predictable; anomalies are not.

---

### 3.2 Why Anomaly Detection for TinyML?

**Advantages:**
- No need for fault labels  
- Compact models  
- Naturally suited to edge deployment  
- Early warning capability  

**Typical TinyML approaches:**
- Statistical thresholds  
- Distance-based methods  
- Lightweight autoencoders  
- Reconstruction error models  

This lab focuses on **resource-efficient techniques** suitable for ESP32.

---

## 4. CPS Perspective (C4–C5)

### CPS Mapping for Lab 8

| CPS Layer | Role |
|---------|------|
| Physical | Normal vs abnormal behavior |
| Conversion | Sensor sampling |
| Cyber | Feature extraction |
| Cognition | Anomaly detection model |
| Control | Alarm logic |
| Actuation | LED / buzzer / shutdown |

This lab emphasizes **safety, resilience, and fault awareness**.

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Sensor (vibration, motion, LDR, or temperature)  
- LED and/or buzzer  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor / Serial Plotter  

---

## 6. System Architecture

```
[Sensor Stream]
       ↓
     ADC / I2C
       ↓
 Sliding Window Buffer
       ↓
 Feature Extraction
       ↓
 Normal Behavior Model
       ↓
 Anomaly Score
       ↓
 Threshold Decision
       ↓
 Alert / Actuator
```

This pipeline runs **continuously on the ESP32**.

---

## 7. Feature Selection for Anomaly Detection

Features are chosen to describe **normal behavior patterns**:

| Feature | Meaning |
|------|---------|
| Mean | Typical signal level |
| RMS | Normal energy |
| Variance | Normal fluctuation |
| Magnitude (motion) | Overall activity |

Features must be:
- Stable during normal operation  
- Sensitive to abnormal changes  

---

## 8. Lab Tasks

### Task 1: Collect Normal Behavior Data

Students operate the system **under normal conditions only** (e.g., normal vibration or lighting).

```cpp
float normalMean = 0;
float normalVar  = 0;
```

Baseline statistics are computed during a **training phase**.

---

### Task 2: Compute Anomaly Score (Statistical)

```cpp
float anomalyScore(float x, float mean, float var) {
  return abs(x - mean) / sqrt(var + 1e-6);
}
```

- Higher score → more abnormal behavior  

---

### Task 3: Threshold-Based Detection

```cpp
#define THRESHOLD 3.0

bool isAnomaly(float score) {
  return score > THRESHOLD;
}
```

**Threshold tuning is critical** to balance sensitivity and false alarms.

---

### Task 4: Streaming Anomaly Detection

```cpp
if (windowReady) {
  float rms = computeRMS();
  float score = anomalyScore(rms, normalMean, normalVar);

  if (isAnomaly(score)) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2000);
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
}
```

The system reacts **immediately** to abnormal patterns.

---

### Task 5: (Optional) Reconstruction-Based Concept

Students discuss (or optionally implement):

```
Reconstruction Error = |input − reconstructed|
```

- Normal → small error  
- Anomaly → large error  

This prepares students for **autoencoder-based TinyML**.

---

## 9. Observations and Discussion

Students should observe:
- Stable behavior under normal conditions  
- Sharp increase in anomaly score during abnormal events  
- Effect of threshold choice on false alarms  

**Key Insight:**  
> Anomaly detection is about **deviation**, not classification.

---

## 10. Performance & Sensitivity Analysis

Students compare threshold settings:

| Parameter | Low Threshold | High Threshold |
|---------|---------------|---------------|
| Sensitivity | High | Low |
| False alarms | High | Low |
| Missed anomalies | Low | High |

This builds intuition for **CPS safety tuning**.

---

## 11. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Normal modeling | Correct baseline estimation | 25% |
| Anomaly score | Meaningful deviation detection | 25% |
| Streaming logic | Real-time detection | 20% |
| Actuation | Correct alert behavior | 20% |
| Discussion | Threshold & trade-off analysis | 10% |

---

## 12. Student Reflection Questions

- Why is anomaly detection useful when faults are rare?  
- What happens if “normal” behavior changes over time?  
- How would you adapt this system to self-update?

---

## 13. Preparation for Next Lab

In **Lab 9**, students will:
- Combine **TinyML decisions with control logic**  
- Prevent **oscillations and false triggering**  
- Build **safe ML-driven actuation systems**

---

## 14. Key Takeaway

**Lab 8 enables resilience:**  
> TinyML can detect unknown faults and abnormal behaviors using only normal data, making CPS safer and more adaptive.
