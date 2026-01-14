# Lab 11: TinyML CPS Capstone Project  
## Design, Implement, and Evaluate an End-to-End TinyML System

---

## 1. Lab Overview

**Lab 11** is the **capstone laboratory** of the TinyML (ESP32-based) series.  
In this lab, students integrate everything learned from **Lab 0 – Lab 10** to **design, implement, and evaluate** a complete **Cyber-Physical System (CPS)** with **on-device TinyML intelligence**.

Students work individually or in small teams to build a **real-world TinyML application** that includes:
- Sensing  
- Feature extraction  
- TinyML inference  
- Safe control and actuation  
- *(Optional)* Edge-to-cloud monitoring  

This lab emphasizes **system thinking, robustness, and engineering trade-offs**, not just model accuracy.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Design a **complete TinyML-based CPS architecture**  
- Select appropriate **sensors, features, and models**  
- Deploy a **TinyML model on ESP32** for real-time inference  
- Implement **safe and stable ML-driven control**  
- Evaluate system performance (**accuracy, latency, memory, robustness**)  
- Communicate results clearly through **documentation and demonstration**

---

## 3. Capstone Concept

### 3.1 What Is a TinyML CPS Capstone?

A TinyML CPS capstone is **not just an ML demo**.  
It must satisfy **all** of the following:

✔ On-device inference (no cloud ML)  
✔ Real physical sensing  
✔ Real actuation  
✔ Continuous operation  
✔ Safety-aware control logic  
✔ Performance evaluation  

> The system **must continue working even if the internet is disconnected**.

---

## 4. CPS Perspective (Full Stack C1–C6)

### CPS Layer Mapping

| CPS Layer | Implemented in Capstone |
|---------|--------------------------|
| C1 – Connection | Sensor wiring & GPIO / I2C |
| C2 – Conversion | ADC / digital conversion |
| C3 – Cyber | Feature extraction |
| C4 – Cognition | TinyML inference |
| C5 – Control | Safe actuation logic |
| C6 – Communication | Optional cloud monitoring |

**Lab 11 validates full CPS integration.**

---

## 5. Project Requirements

### 5.1 Mandatory Components

Each capstone project **must include**:

- ESP32 microcontroller  
- At least **one sensor**  
- At least **one actuator**  
- A **TinyML model** (classifier or anomaly detector)  
- On-device **feature extraction**  
- **Safe control logic**

---

### 5.2 Optional Components (Bonus)

- Cloud dashboard (MQTT / HTTP)  
- Multiple sensors  
- Multi-class inference  
- Power / energy discussion  

---

## 6. Suggested Project Topics

Students may choose one of the following or propose their own.

### Option A: Smart Environment Monitor
- **Sensor:** LDR / temperature  
- **TinyML:** Light or condition classification  
- **Actuator:** LED / relay  
- **Bonus:** Cloud visualization  

### Option B: Activity Recognition System
- **Sensor:** MPU6050 / vibration sensor  
- **TinyML:** Still / Move / Shake  
- **Actuator:** LED / buzzer  
- **Bonus:** Confidence reporting  

### Option C: Anomaly-Based Fault Detector
- **Sensor:** Vibration / temperature  
- **TinyML:** Normal-only anomaly detection  
- **Actuator:** Alarm / shutdown  
- **Bonus:** Threshold auto-tuning  

### Option D: Custom CPS Idea
- Must be approved by instructor  
- Must satisfy all mandatory requirements  

---

## 7. System Architecture (Example)

```
[Physical Sensor]
        ↓
   Data Acquisition
        ↓
 Feature Extraction
        ↓
 TinyML Inference
        ↓
 Decision Smoothing
        ↓
 Safe Control Logic
        ↓
   Actuator Output
        ↓
 (Optional) Cloud Dashboard
```

Students must **clearly justify** why each block is required.

---

## 8. Lab Tasks

### Task 1: System Design Proposal

Students submit:
- Block diagram  
- Sensor & actuator selection  
- TinyML approach (classification or anomaly detection)

---

### Task 2: Implementation

Students implement:
- Continuous sensing  
- Streaming inference  
- Stable actuation  

Example inference call:

```cpp
int result = tinyml_predict_int8(features);
```

---

### Task 3: Safety and Robustness Handling

Students must demonstrate:
- No actuator oscillation  
- Safe fallback state  
- Stable operation under noise and disturbances  

---

### Task 4: Performance Evaluation

Each project must report:

| Metric | Description |
|------|-------------|
| Accuracy | Classification or detection performance |
| Latency | Inference time (µs or ms) |
| Memory | Flash & RAM usage |
| Stability | Behavior over time |
| Robustness | Network loss handling (if used) |

---

### Task 5: Demonstration & Documentation

**Deliverables:**
- Live demo or recorded video (1–2 minutes)  
- Source code (GitHub repository or ZIP)  
- `README.md` or short technical report  

---

## 9. Example Evaluation Table

| Aspect | Result |
|------|--------|
| Model type | INT8 classifier |
| Window size | 20 samples |
| Inference latency | 1.2 ms |
| Flash usage | 78 KB |
| RAM usage | 12 KB |
| System stability | Stable |

---

## 10. Evaluation Criteria (Grading Rubric)

| Component | Weight |
|---------|--------|
| System design | 15% |
| TinyML implementation | 25% |
| CPS integration | 25% |
| Performance evaluation | 20% |
| Documentation & demo | 15% |

---

## 11. Student Reflection Questions

Students must answer:
- What was the **hardest CPS challenge** in your project?  
- Which **design trade-off** mattered most (accuracy, latency, safety)?  
- How would you **scale this system** to many devices?

---

## 12. Learning Outcomes (Course-Level)

By completing Lab 11, students demonstrate:

✔ Understanding of TinyML principles  
✔ Ability to design **edge-intelligent CPS**  
✔ Awareness of **safety and reliability**  
✔ Readiness for **real-world IoT / CPS projects**

---

## 13. Final Key Takeaway

**Lab 11 proves mastery:**  
> Students can design, build, and evaluate a complete **TinyML-powered Cyber-Physical System** that operates intelligently, safely, and efficiently at the edge.
