# Lab 9 – CPS and Digital Twin Integration (C3 & C4)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand the concept of a **Digital Twin** in Cyber-Physical Systems (CPS)
- Integrate ESP32-based CPS data with a **digital representation**
- Stream Edge Impulse inference results to a digital twin dashboard
- Visualize system states, health indicators, and trends
- Enable human-in-the-loop monitoring and analysis

---

## 🧠 Background Theory

### 1. Digital Twin Concept

A **Digital Twin** is a virtual representation of a physical system that:
- Mirrors the physical system’s state
- Receives real-time data from sensors
- Enables visualization, analysis, and prediction

In CPS, digital twins enhance understanding, diagnostics, and optimization.

---

### 2. Digital Twin in the CPS Architecture

Digital twins typically span:
- **C3 – Cyber layer:** data aggregation and communication
- **C4 – Cognition layer:** interpretation of system states

They provide a bridge between edge intelligence and human decision-making.

---

### 3. Role of Edge Intelligence in Digital Twins

Edge Impulse provides:
- Classification results (system modes, activities)
- Anomaly scores (health indicators)
- Event timestamps

These outputs serve as **inputs to the digital twin**.

---

## 🧱 CPS Focus of This Lab

- **C3 – Cyber:** data streaming and aggregation  
- **C4 – Cognition:** system-state interpretation  
- C1, C2, C5: Supporting layers  

---

## ⚙️ Edge Impulse Focus

- Using inference outputs as digital twin variables
- Structuring CPS telemetry for visualization
- Continuous inference for live twin updates

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor and actuator from previous labs
- USB cable
- Wi-Fi network

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse inference library
- MQTT broker
- Node-RED or similar dashboard tool

---

## 🔧 Experimental Setup

### Example: Digital Twin Variables

| Variable | Description |
|--------|-------------|
| state | Classified CPS mode |
| anomaly_score | Health indicator |
| confidence | Inference confidence |
| timestamp | Event time |

---

## 🧪 Experimental Procedure

### Step 1: Prepare CPS Telemetry
1. Run inference on ESP32.
2. Format results as JSON.
3. Include timestamps and identifiers.

---

### Step 2: Transmit Data to Cyber Layer
1. Publish telemetry via MQTT.
2. Verify message integrity.

---

### Step 3: Build Digital Twin Dashboard
1. Subscribe to MQTT topics.
2. Map variables to dashboard widgets.
3. Display real-time system state.

---

### Step 4: Human-in-the-Loop Analysis
1. Observe trends and anomalies.
2. Correlate physical events with digital states.
3. Identify abnormal behavior.

---

## 📊 Results and Observations

Students should observe:
- Real-time mirroring of CPS states
- Live anomaly and health visualization
- Improved situational awareness

---

## 📝 Discussion Questions

1. What distinguishes a digital twin from simple monitoring?
2. Which CPS layers are involved in digital twin operation?
3. How does edge intelligence improve digital twins?
4. What decisions can humans make using twin data?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain digital twin concepts in CPS
- Integrate CPS data into a digital twin
- Visualize and analyze system states
- Support human-in-the-loop CPS operation

---

## 🧾 Conclusion

This lab demonstrated how Edge Impulse–enabled CPS can be extended into a **digital twin**, providing real-time visibility and analytical capability beyond the physical system.

---

## 📚 References

1. Edge Impulse Documentation  
2. Digital Twin Literature  
3. Node-RED Documentation  
4. CPS Architecture References  

---

### ▶️ Next Lab

**Lab 10 – Energy-Aware and Resource-Constrained CPS**
