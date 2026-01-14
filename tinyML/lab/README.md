# TinyML Lab Series (ESP32-Based)
## Lab 0 → Lab 11: Cyber-Physical Systems with TinyML

This repository describes a **complete TinyML laboratory sequence (Lab 0 – Lab 11)** designed for **ESP32-based microcontrollers**.  
Each lab integrates **Cyber-Physical Systems (CPS) concepts** with **hands-on TinyML practice**, progressing from basic embedded setup to a full end-to-end TinyML CPS capstone project.

---

## Lab 0 — Setup & “Hello TinyML”

**Goal:**  
Set up the ESP32 development environment and run the first on-device inference.

**CPS Focus:**  
- Overview of CPS loop: **Sensor → Decision → Actuator**

**TinyML Focus:**  
- Meaning of inference on a microcontroller  
- Constraints: RAM, Flash, latency

**Hardware:**  
- ESP32  
- USB cable  
- LED (or onboard LED)

**Deliverables:**  
- ESP32 programmed successfully  
- Tiny threshold-based classifier controlling LED  
- Short report: inference latency and memory usage

---

## Lab 1 — Digital I/O Sensors (No ML Yet)

**Goal:**  
Build reliable sensor input and actuator control.

**CPS Focus:**  
- C1: Connection (GPIO wiring and reliability)

**TinyML Focus:**  
- Data quality fundamentals (noise, sampling)

**Sensors:**  
- Button / PIR / tilt switch

**Deliverables:**  
- Debounced digital input → actuator (LED or buzzer)  
- Serial logging of events

---

## Lab 2 — Analog Sensors & Data Logging

**Goal:**  
Acquire clean analog sensor data and create datasets.

**CPS Focus:**  
- C2: Conversion (ADC, scaling, normalization)

**TinyML Focus:**  
- Dataset generation for ML training

**Sensors:**  
- LDR / potentiometer / analog temperature sensor (optional)

**Deliverables:**  
- CSV-style serial data logging  
- Basic signal filtering (e.g., moving average)

---

## Lab 3 — Feature Engineering on ESP32

**Goal:**  
Convert raw sensor signals into meaningful features.

**CPS Focus:**  
- C3: Cyber (embedded processing pipeline)

**TinyML Focus:**  
- Feature extraction techniques

**Features:**  
- Mean, variance, RMS  
- Min / max  
- Slope or trend

**Deliverables:**  
- Feature extraction function  
- Feature table (and optional plots)

---

## Lab 4 — Train a Tiny Classifier & Deploy to ESP32

**Goal:**  
Complete the full ML pipeline: train → convert → deploy → infer.

**CPS Focus:**  
- Closed-loop intelligent decision-making

**TinyML Focus:**  
- Small models (2–3 classes)  
- Model evaluation (accuracy)

**Example Classes:**  
- Bright / Normal / Dark (LDR)

**Deliverables:**  
- Trained model (small NN or decision tree)  
- Model exported as C array  
- ESP32 inference with LED pattern output

---

## Lab 5 — Quantization & Memory Budget

**Goal:**  
Optimize model size and inference speed.

**CPS Focus:**  
- Real-time constraints

**TinyML Focus:**  
- Quantization (INT8)  
- Accuracy vs memory trade-offs

**Deliverables:**  
- Comparison table: float vs INT8  
- Model size, latency, accuracy analysis

---

## Lab 6 — Time-Series Windowing (Streaming Inference)

**Goal:**  
Perform inference on streaming sensor data.

**CPS Focus:**  
- Online monitoring

**TinyML Focus:**  
- Sliding windows  
- Overlap vs latency trade-offs

**Sensors:**  
- LDR (time-series) or accelerometer

**Deliverables:**  
- Window buffer implementation  
- Periodic inference results  
- Confusion-style evaluation

---

## Lab 7 — Motion / Vibration Activity Recognition

**Goal:**  
Recognize simple motion patterns at the edge.

**CPS Focus:**  
- Embedded intelligence in physical systems

**TinyML Focus:**  
- Multi-axis features  
- Simple MLP or CNN

**Sensors:**  
- MPU6050 (recommended) or vibration sensor

**Classes:**  
- Still / Shake / Move

**Deliverables:**  
- Live classification  
- Serial monitor dashboard output

---

## Lab 8 — Anomaly Detection

**Goal:**  
Detect abnormal behavior instead of fixed classes.

**CPS Focus:**  
- Fault detection and safety monitoring

**TinyML Focus:**  
- One-class learning  
- Threshold-based anomaly detection

**Scenario:**  
- Normal vs abnormal vibration

**Deliverables:**  
- Trained normal-only model  
- Anomaly alarm using actuator

---

## Lab 9 — TinyML-Based Actuation Control

**Goal:**  
Use ML decisions to control actuators safely.

**CPS Focus:**  
- Decision → Actuator with stability constraints

**TinyML Focus:**  
- Post-processing  
- Hysteresis and debouncing ML outputs

**Actuators:**  
- Relay, servo, fan, buzzer, LED

**Deliverables:**  
- Stable ML-driven control (no flicker or oscillation)

---

## Lab 10 — Edge-to-Cloud Reporting (Optional)

**Goal:**  
Keep inference on-device while reporting results remotely.

**CPS Focus:**  
- Cyber connectivity and monitoring

**TinyML Focus:**  
- On-device inference + lightweight telemetry

**Options:**  
- MQTT or HTTP  
- Node-RED dashboard

**Deliverables:**  
- ESP32 publishes predictions, confidence, and timestamps

---

## Lab 11 — Capstone Mini-Project

**Goal:**  
Develop a complete end-to-end TinyML CPS system.

**CPS Focus:**  
- Full loop integration  
- Evaluation and reliability

**TinyML Focus:**  
- Model selection  
- Dataset design  
- Deployment and testing

**Project Ideas:**  
- Smart light monitoring with anomaly alerts  
- Activity recognition with dashboard  
- Predictive vibration monitoring with buzzer alert

**Deliverables:**  
- GitHub repository (code + README)  
- Demo video (1–2 minutes)  
- Results table: accuracy, latency, memory, power notes

---

## Learning Outcomes

By completing Lab 0 – Lab 11, students will be able to:
- Design CPS architectures with embedded intelligence  
- Build and deploy TinyML models on ESP32  
- Optimize models for memory, latency, and power  
- Develop reliable, real-time TinyML-based systems

---

**This lab series is suitable for courses in:**  
TinyML, Embedded AI, Cyber-Physical Systems, IoT, and Edge Intelligence.
