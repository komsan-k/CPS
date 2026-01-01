# Lab 7 – CPS Control Loop Integration and Supervisory Control (C3 & C5)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand **feedback control loops** in Cyber-Physical Systems (CPS)
- Integrate **Edge Impulse intelligence** with classical control
- Implement **supervisory control** on an ESP32
- Combine sensing, inference, decision logic, and actuation
- Analyze closed-loop CPS behavior

---

## 🧠 Background Theory

### 1. Feedback Control in CPS

A feedback control loop consists of:
- Sensor (measurement)
- Controller (decision logic)
- Actuator (action)
- Physical plant (system)

Feedback allows CPS to:
- Maintain stability
- Reject disturbances
- Adapt to changing conditions

---

### 2. Supervisory Control Concept

In modern CPS:
- Classical controllers (PID, on–off) handle fast dynamics
- **Supervisory controllers** adjust control modes or parameters

Edge Impulse models act as **supervisors**, not low-level controllers.

---

### 3. Role of Edge Intelligence in Control Loops

Edge intelligence can:
- Detect operating modes
- Identify abnormal conditions
- Enable mode switching (normal / safe / emergency)

This improves robustness and safety.

---

## 🧱 CPS Focus of This Lab

- **C3 – Cyber:** Information flow and coordination  
- **C5 – Configuration:** Adaptive control and supervision  
- C1–C4: Supporting layers  

---

## ⚙️ Edge Impulse Focus

- Using inference outputs as supervisory signals
- Integrating ML outputs into control logic
- Continuous inference in closed-loop operation

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor from previous labs
- Actuator (motor, servo, relay, LED)
- USB cable

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse inference library
- Serial monitor or logging tool

---

## 🔧 Experimental Setup

### Example: Motor Speed Supervisory Control

| Component | ESP32 Pin |
|---------|-----------|
| Motor PWM | GPIO 18 |
| Direction | GPIO 19 |
| Enable | GPIO 23 |
| Sensor | GPIO / I2C |

---

## 🧪 Experimental Procedure

### Step 1: Implement Baseline Controller
1. Write a simple controller (on–off or PID).
2. Verify stable operation without ML.

---

### Step 2: Integrate Edge Impulse Inference
1. Run inference in parallel with control loop.
2. Obtain classification or anomaly outputs.

---

### Step 3: Design Supervisory Logic
Example:
- Normal state → full operation
- Warning state → reduced speed
- Anomaly detected → shutdown

---

### Step 4: Closed-Loop Testing
1. Apply disturbances.
2. Observe CPS response.
3. Compare behavior with and without supervision.

---

## 📊 Results and Observations

Students should observe:
- Stable closed-loop control
- Improved safety with ML supervision
- Clear mode switching behavior

---

## 📝 Discussion Questions

1. Why should ML not replace low-level controllers?
2. How does supervision improve CPS safety?
3. What latency constraints exist in CPS loops?
4. How does C3 support supervisory control?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain CPS feedback control loops
- Implement supervisory control
- Integrate Edge Impulse with classical control
- Evaluate closed-loop CPS behavior

---

## 🧾 Conclusion

This lab demonstrated **ML-assisted supervisory control** in a CPS by integrating Edge Impulse inference with feedback control loops on ESP32.

---

## 📚 References

1. Edge Impulse Documentation  
2. Control Systems Engineering Texts  
3. CPS Architecture Literature  

---

### ▶️ Next Lab

**Lab 8 – CPS Cyber Layer Integration with IoT and MQTT**
