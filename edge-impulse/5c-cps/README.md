# Edge Impulse in the 5C Cyber-Physical System (CPS) Model

## 📌 Overview

This README presents a structured mapping of **Edge Impulse (EI)** into the classical **5C Cyber-Physical System (CPS) architecture**.  
The 5C model provides a systematic way to design intelligent CPS by organizing functionality from physical sensing to adaptive configuration.

Edge Impulse primarily strengthens the **Conversion** and **Cognition** layers by enabling embedded machine learning at the edge.

---

## 🧠 The 5C CPS Model

The **5C CPS architecture** consists of five layers:

1. **Connection** – Data acquisition from the physical world  
2. **Conversion** – Transforming raw data into meaningful information  
3. **Cyber** – System integration and communication  
4. **Cognition** – Understanding, reasoning, and decision support  
5. **Configuration** – Adaptive control and system self-improvement  

---

## 📊 Edge Impulse Mapping Using the 5C CPS Model

| **5C Layer** | **CPS Function** | **Edge Impulse Mapping** | **Typical Technologies / Outputs** |
|-------------|------------------|--------------------------|------------------------------------|
| **C1 – Connection** | Sensor data acquisition | Data collection via MCU, Linux device, or mobile app | IMU, microphone, camera, vibration sensors; CSV/WAV/image datasets |
| **C2 – Conversion** | Data preprocessing and feature extraction | EI DSP blocks (FFT, MFCC, spectrograms, statistics) | Feature vectors, embeddings, compressed representations |
| **C3 – Cyber** | Communication and data integration | Inference results integrated with IoT middleware | MQTT, HTTP, OPC UA, Node-RED, cloud dashboards |
| **C4 – Cognition** | Intelligence and system understanding | EI ML models (classification, regression, anomaly detection) | Health scores, anomaly indices, activity labels |
| **C5 – Configuration** | Adaptation and optimization | Model retraining, threshold tuning, redeployment | OTA updates, adaptive policies, improved control strategies |

---

## 🔍 Interpretation

- **Strongest contribution of Edge Impulse**
  - **C2 – Conversion:** Efficient signal-to-feature processing on-device
  - **C4 – Cognition:** Embedded intelligence and perception

- **Edge Impulse does NOT replace**
  - Classical controllers (PID, MPC)
  - CPS middleware or orchestration layers

Edge Impulse acts as the **perception and diagnostics brain** of a CPS.

---

## 🔄 Closed-Loop CPS Operation (5C Perspective)

1. **C1:** Sensors capture physical signals  
2. **C2:** Edge Impulse extracts features on-device  
3. **C3:** Results are shared via cyber infrastructure  
4. **C4:** System state and health are interpreted  
5. **C5:** System adapts through control, retraining, or redeployment  

This enables a **self-adaptive and learning CPS**.

---

## 🏭 Example: Predictive Maintenance CPS

| 5C Layer | Example |
|--------|---------|
| C1 | Accelerometer mounted on a motor |
| C2 | Spectrogram and feature extraction using EI |
| C3 | MQTT publishes anomaly score |
| C4 | Cognition detects bearing degradation |
| C5 | Maintenance scheduled and model retrained |

---

## 🎯 Key Takeaway

> **Edge Impulse operationalizes the Conversion and Cognition layers of the 5C CPS model by embedding real-time intelligence directly at the edge.**

---

## 📄 License & Usage

This README is intended for **educational, research, and academic use**.  
Refer to the official Edge Impulse documentation for platform licensing details.
