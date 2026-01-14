# Lab 9: TinyML-Driven Smart Control and Actuation  
## From ML Decisions to Safe Physical Actions  
### CPS Focus: C5 — Control

---

## 1. Lab Overview

This laboratory focuses on **closing the Cyber-Physical System (CPS) loop** by connecting **TinyML decisions** to **real physical actuation** in a **safe, stable, and reliable** manner.

While previous labs emphasized sensing, inference, and detection, **Lab 9 addresses a critical CPS question**:

> **How can ML decisions be trusted to control physical systems safely?**

Students will integrate TinyML outputs (classification or anomaly detection) with **control logic** such as **decision smoothing, hysteresis, and time confirmation** to drive actuators (LED, relay, motor, buzzer) **without oscillation or instability**.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Integrate TinyML inference results with actuator control  
- Prevent unstable behavior caused by noisy ML predictions  
- Implement **decision smoothing** and **hysteresis**  
- Design **safe ML-driven actuation logic**  
- Explain CPS **safety considerations** when ML controls hardware  

---

## 3. Background Theory

### 3.1 Why Control Logic Is Needed After ML

TinyML outputs are **probabilistic and imperfect**:

- Predictions may fluctuate  
- Confidence may vary  
- Noise and uncertainty are unavoidable  

Direct mapping:

```
ML Output → Actuator
```

can cause:
- Rapid ON/OFF switching  
- Mechanical wear  
- Unsafe or unstable behavior  

Therefore, **control logic must wrap ML decisions** before actuation.

---

### 3.2 Control Techniques Used in This Lab

| Technique | Purpose |
|---------|---------|
| Decision smoothing | Reduce rapid class changes |
| Hysteresis | Prevent oscillation |
| Time confirmation | Ensure persistence before actuation |
| Safety fallback | Force default safe state |

These techniques are standard in **industrial CPS and safety-critical systems**.

---

## 4. CPS Perspective (C5: Control)

### CPS Mapping for Lab 9

| CPS Layer | Role |
|---------|------|
| Physical | Environment & actuator |
| Conversion | Sensor sampling |
| Cyber | Feature extraction |
| Cognition | TinyML inference |
| Control | Safety logic |
| Actuation | Motor / relay / alert |

**Lab 9 is where AI meets responsibility.**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Sensor (from Labs 6–8)  
- Actuator (choose one):  
  - LED (basic)  
  - Buzzer  
  - Relay module  
  - DC motor / fan  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor  

---

## 6. System Architecture

```
[Sensor Stream]
       ↓
 Feature Extraction
       ↓
 TinyML Inference
       ↓
 Decision Buffer / Smoother
       ↓
 Control Logic (Hysteresis)
       ↓
 Safe Actuation
```

This architecture ensures **robust CPS operation**.

---

## 7. Lab Tasks

### Task 1: TinyML Prediction Stream

```cpp
int prediction = tinyml_predict_int8(features);
```

Predictions may fluctuate:

```
1, 1, 0, 1, 0, 1, ...
```

---

### Task 2: Decision Smoothing (Majority Vote)

```cpp
#define DECISION_WIN 5
int decisionBuf[DECISION_WIN];
int dIdx = 0;

void addDecision(int d) {
  decisionBuf[dIdx++] = d;
  if (dIdx >= DECISION_WIN) dIdx = 0;
}

int smoothDecision() {
  int sum = 0;
  for (int i = 0; i < DECISION_WIN; i++)
    sum += decisionBuf[i];
  return (sum > DECISION_WIN/2) ? 1 : 0;
}
```

This reduces **prediction jitter**.

---

### Task 3: Hysteresis-Based Control

```cpp
bool actuatorState = false;

void controlWithHysteresis(int decision) {
  if (!actuatorState && decision == 1) {
    actuatorState = true;
    digitalWrite(ACT_PIN, HIGH);
  }
  else if (actuatorState && decision == 0) {
    actuatorState = false;
    digitalWrite(ACT_PIN, LOW);
  }
}
```

This prevents **rapid switching**.

---

### Task 4: Time-Confirmed Actuation

```cpp
unsigned long decisionStart = 0;

void controlWithTimeConfirm(int decision) {
  if (decision == 1) {
    if (decisionStart == 0)
      decisionStart = millis();

    if (millis() - decisionStart > 1000)
      digitalWrite(ACT_PIN, HIGH);
  } else {
    decisionStart = 0;
    digitalWrite(ACT_PIN, LOW);
  }
}
```

Only **persistent decisions** trigger actuation.

---

### Task 5: Safety Fallback

```cpp
if (sensorError || modelError) {
  digitalWrite(ACT_PIN, LOW); // safe state
}
```

**Safety always overrides ML decisions.**

---

## 8. Observations and Discussion

Students should observe:
- Raw ML output vs smoothed output  
- Significant improvement in actuator stability  
- Delay introduced by safety logic  

**Key Insight:**  
> ML decides — control logic ensures safety.

---

## 9. Experimental Comparison

| Configuration | Stability | Responsiveness |
|--------------|-----------|----------------|
| Direct ML → Actuator | ❌ Poor | ✅ Fast |
| Smoothed ML | ✅ Good | ⚠ Medium |
| Smoothed + Hysteresis | ✅✅ Excellent | ⚠ Slower |

Students justify the best **CPS trade-off**.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| ML integration | Correct use of inference | 20% |
| Smoothing logic | Stable decision behavior | 25% |
| Hysteresis / timing | No oscillation | 25% |
| Safety handling | Proper fallback | 20% |
| Discussion | CPS reasoning | 10% |

---

## 11. Student Reflection Questions

- Why should ML never control actuators directly?  
- What risks arise in safety-critical CPS?  
- How would certification affect ML-controlled systems?

---

## 12. Preparation for Next Lab

In **Lab 10**, students will:
- Keep TinyML inference **on-device**  
- Send decisions and confidence **to the cloud**  
- Build **monitoring dashboards** (Edge-to-Cloud CPS)

---

## 13. Key Takeaway

**Lab 9 completes the CPS loop responsibly:**  
> TinyML provides intelligence, but safe control logic makes that intelligence usable in the real world.
