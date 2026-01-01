# Lab 6 – On-Device Inference and CPS Decision Logic (C4 → C5)

## 🎯 Objective

The objectives of this laboratory are to:

- Execute **on-device inference** using trained Edge Impulse models
- Design **decision logic** based on ML outputs
- Integrate perception (C4) with action (C5) in a CPS
- Control actuators using ESP32 based on inference results
- Understand the transition from **cognition to configuration** in CPS

---

## 🧠 Background Theory

### 1. From Cognition to Configuration in CPS

In the 5C CPS model:
- **C4 (Cognition)** provides system understanding through ML inference
- **C5 (Configuration)** adapts system behavior based on decisions

This lab bridges these two layers by converting ML outputs into concrete actions.

---

### 2. On-Device Inference

On-device inference means:
- ML models run locally on the ESP32
- Decisions are made without cloud dependency
- Latency and energy consumption are minimized

This is critical for real-time CPS operation.

---

### 3. Decision Logic in CPS

Decision logic maps inference results to actions:
- Threshold-based logic
- Rule-based systems
- Finite State Machines (FSM)

In this lab, simple **rule-based logic** is used.

---

## 🧱 CPS Focus of This Lab

- **C4 – Cognition:** ML inference  
- **C5 – Configuration:** Decision and actuation  

Lower layers (C1, C2) provide sensing and features.

---

## ⚙️ Edge Impulse Focus

- C++ inference API
- Interpreting classification probabilities or anomaly scores
- Real-time inference loop on ESP32

---

## 🧰 Hardware Requirements

- ESP32 development board
- Actuator (choose one):
  - LED
  - Buzzer
  - Relay module
  - Servo motor
- USB cable

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse firmware library
- Edge Impulse Studio

---

## 🔧 Experimental Setup

### Example: LED Actuation

| Component | ESP32 Pin |
|---------|-----------|
| LED (+) | GPIO 2 |
| LED (−) | GND |
| Resistor | 220 Ω |

---

## 🧪 Experimental Procedure

### Step 1: Prepare Inference Firmware
1. Export trained model as Arduino library.
2. Import library into Arduino IDE.
3. Open inference example sketch.

---

### Step 2: Implement Inference Loop
1. Acquire sensor data.
2. Run Edge Impulse classifier.
3. Read classification results or anomaly score.

---

### Step 3: Design Decision Logic
Example logic:
- If class == "active" → LED ON
- If anomaly score > threshold → buzzer ON

---

### Step 4: Implement Actuation
1. Configure GPIO pins.
2. Trigger actuator based on decision logic.
3. Upload firmware to ESP32.

---

### Step 5: Test CPS Behavior
1. Stimulate physical system.
2. Observe inference outputs.
3. Verify actuator response.

---

## 📊 Results and Observations

Students should observe:
- Real-time inference running on ESP32
- Correct decision-making behavior
- Immediate physical response via actuator

---

## 📝 Discussion Questions

1. Why is on-device inference important in CPS?
2. How does decision logic affect system safety?
3. What are the limitations of rule-based decisions?
4. How can FSM improve CPS configuration?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Deploy ML inference on ESP32
- Design CPS decision logic
- Integrate cognition with configuration
- Control actuators using ML outputs

---

## 🧾 Conclusion

This lab completed the CPS perception–action loop by integrating Edge Impulse inference with decision logic and actuation on ESP32. The CPS can now perceive, decide, and act autonomously.

---

## 📚 References

1. Edge Impulse Documentation  
2. Embedded Control Systems  
3. CPS Architecture Literature  

---

### ▶️ Next Lab

**Lab 7 – CPS Control Loop Integration and Supervisory Control**
