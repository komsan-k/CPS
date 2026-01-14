# Lab 7: Activity Recognition Using Motion / Vibration Sensors  
## Multi-Axis Streaming TinyML on ESP32  
### CPS Focus: C4–C5 (Cognition → Configuration)

---

## 1. Lab Overview

This laboratory applies **streaming TinyML** (from Lab 6) to a real-world **multi-axis sensor scenario**:  
**activity recognition using motion or vibration data**.

Students will interface a **motion sensor** (e.g., MPU6050 accelerometer) with the ESP32, extract **multi-axis features**, and perform **real-time, on-device activity classification**.

Typical activities include:
- **Still**
- **Move**
- **Shake / Vibration**

This lab demonstrates how **TinyML enables context awareness** in Cyber-Physical Systems (CPS).

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Interface a **multi-axis motion or vibration sensor** with ESP32  
- Acquire **synchronized time-series data** from multiple channels  
- Extract **multi-axis features** suitable for TinyML  
- Perform **real-time activity recognition** using streaming inference  
- Analyze **stability, latency, and misclassification causes**

---

## 3. Background Theory

### 3.1 Activity Recognition in CPS

Activity recognition is a core CPS capability used in:
- Wearable devices  
- Smart machines  
- Structural health monitoring  
- Predictive maintenance  

Rather than measuring a single value, the system analyzes **patterns over time**.

---

### 3.2 Multi-Axis Sensor Data

Motion sensors typically provide:

| Axis | Meaning |
|----|---------|
| X | Horizontal movement |
| Y | Vertical movement |
| Z | Depth / vibration |

Each axis alone is ambiguous, but **combined features describe activities accurately**.

---

## 4. CPS Perspective (C4–C5)

### CPS Mapping for Lab 7

| CPS Layer | Role |
|---------|------|
| Physical | Motion / vibration |
| Conversion | ADC / I2C data acquisition |
| Cyber | Feature extraction |
| Cognition | Activity classification |
| Control | Indicator / alert logic |
| Actuation | LED / buzzer |

**Lab 7 introduces context-based cognition and adaptive response.**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- MPU6050 accelerometer or vibration sensor  
- LED or buzzer  
- Breadboard and jumper wires  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- I2C library (`Wire.h`)  
- Serial Monitor / Serial Plotter  

---

## 6. System Architecture

```
[Motion Sensor (X, Y, Z)]
          ↓
      I2C / ADC
          ↓
   Sliding Window Buffer
          ↓
 Multi-Axis Feature Extraction
          ↓
  TinyML Classifier (INT8)
          ↓
     Activity Label
          ↓
   LED / Buzzer / Log
```

This represents a **complete streaming TinyML CPS pipeline**.

---

## 7. Feature Set for Activity Recognition

Features are computed **per axis**, then combined:

| Feature | Description |
|-------|-------------|
| Mean (X,Y,Z) | Average motion |
| RMS (X,Y,Z) | Signal energy |
| Variance (X,Y,Z) | Motion intensity |
| Magnitude RMS | Overall activity strength |

This results in a **compact and informative feature vector**.

---

## 8. Lab Tasks

### Task 1: Read Multi-Axis Motion Data

```cpp
int16_t ax, ay, az;

void readMotion() {
  ax = readAccelX();
  ay = readAccelY();
  az = readAccelZ();
}
```

(Actual functions depend on the sensor library used.)

---

### Task 2: Sliding Window Buffer (Multi-Axis)

```cpp
#define WIN 20
int axBuf[WIN], ayBuf[WIN], azBuf[WIN];
int idx = 0;

void addMotionSample() {
  axBuf[idx] = ax;
  ayBuf[idx] = ay;
  azBuf[idx] = az;
  idx = (idx + 1) % WIN;
}
```

---

### Task 3: Feature Extraction

```cpp
float rmsAxis(int *buf) {
  long sumSq = 0;
  for (int i = 0; i < WIN; i++)
    sumSq += buf[i] * buf[i];
  return sqrt(sumSq / (float)WIN);
}
```

**Combined magnitude feature:**

```cpp
float magRMS = sqrt(rmsX*rmsX + rmsY*rmsY + rmsZ*rmsZ);
```

---

### Task 4: Streaming Inference

```cpp
float features[4] = { rmsX, rmsY, rmsZ, magRMS };
int activity = tinyml_predict_int8(features);
```

---

### Task 5: Activity-Based Actuation

```cpp
void updateOutput(int label) {
  if (label == 0) digitalWrite(LED_PIN, LOW);        // Still
  else if (label == 1) digitalWrite(LED_PIN, HIGH); // Move
  else tone(BUZZER_PIN, 2000);                       // Shake
}
```

---

## 9. Observations and Discussion

Students should analyze:
- Stability of predictions during activity transitions  
- False positives between **Move** and **Shake**  
- Impact of window size on recognition accuracy  

**Key Insight:**  
> Activity recognition depends on temporal patterns, not individual samples.

---

## 10. Experimental Analysis

Students compare configurations such as:

| Parameter | Case A | Case B |
|---------|--------|--------|
| Window size | 15 | 30 |
| Features | RMS only | RMS + Variance |
| Accuracy |  | ↑ |
| Latency | ↓ |  |

This reinforces **design trade-offs**.

---

## 11. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Sensor interface | Correct multi-axis reading | 20% |
| Feature design | Meaningful feature set | 25% |
| Streaming inference | Stable real-time output | 25% |
| Actuation | Correct activity response | 20% |
| Analysis | Discussion & insight | 10% |

---

## 12. Student Reflection Questions

- Why is magnitude useful for activity recognition?  
- What causes confusion between activities?  
- How would you extend this system to more activities?

---

## 13. Preparation for Next Lab

In **Lab 8**, students will:
- Detect **anomalies instead of fixed activities**  
- Train **normal-only models**  
- Implement **fault or abnormal behavior detection**

---

## 14. Key Takeaway

**Lab 7 enables contextual intelligence:**  
> TinyML can recognize physical activities in real time using compact, on-device models.
