# Lab 8 – CPS Cyber Layer Integration with IoT and MQTT (C3: Cyber)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand the **Cyber (C3) layer** of the 5C Cyber-Physical System (CPS) model
- Integrate an ESP32-based CPS with **IoT networking**
- Publish CPS inference results to a **MQTT broker**
- Visualize CPS data using dashboards (e.g., Node-RED)
- Enable distributed coordination and monitoring of CPS components

---

## 🧠 Background Theory

### 1. Cyber Layer in CPS (C3)

The **Cyber layer (C3)** aggregates information from multiple CPS components and enables:
- Communication and interoperability
- System-wide visibility
- Coordination among distributed nodes

C3 does not control physical dynamics directly; it **connects, synchronizes, and contextualizes** CPS data.

---

### 2. IoT Messaging with MQTT

**MQTT** is a lightweight publish/subscribe protocol widely used in CPS and IoT due to:
- Low bandwidth overhead
- Asynchronous communication
- Scalability for many devices

In CPS, MQTT enables decoupling between **edge intelligence** and **cyber services**.

---

### 3. Edge Impulse Outputs in the Cyber Layer

Edge Impulse provides:
- Classification probabilities
- Anomaly scores
- Timestamps and metadata

These outputs are ideal for transmission via MQTT to cyber services.

---

## 🧱 CPS Focus of This Lab

- **C3 – Cyber:** Primary focus  
- C1 – Connection: Sensors  
- C2 – Conversion: Features  
- C4 – Cognition: Inference  
- C5 – Configuration: Local actuation  

---

## ⚙️ Edge Impulse Focus

- Consuming inference results
- Packaging outputs for network transmission
- Integrating ML results with IoT middleware

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
- MQTT broker (local or cloud)
- Node-RED (optional, for visualization)

---

## 🔧 Experimental Setup

### Example: MQTT Topics

| Topic | Description |
|------|-------------|
| cps/esp32/inference | Classification or anomaly results |
| cps/esp32/status | Device health and heartbeat |
| cps/esp32/control | Remote supervisory commands |

---

## 🧪 Experimental Procedure

### Step 1: Configure Wi-Fi Connectivity
1. Set Wi-Fi SSID and password in ESP32 firmware.
2. Verify network connection.

---

### Step 2: Set Up MQTT Client
1. Configure MQTT broker address and port.
2. Define publish topics.
3. Establish MQTT connection.

---

### Step 3: Publish Edge Impulse Inference Results
1. Run inference loop.
2. Package results as JSON.
3. Publish results periodically.

---

### Step 4: Dashboard Visualization (Optional)
1. Subscribe to topics using Node-RED.
2. Display inference values and trends.
3. Verify real-time updates.

---

### Step 5: Distributed CPS Test
1. Simulate multiple CPS nodes.
2. Observe cyber-level aggregation.
3. Analyze system-level behavior.

---

## 📊 Results and Observations

Students should observe:
- Successful MQTT communication
- Real-time CPS data streaming
- Cyber-layer visibility of CPS intelligence

---

## 📝 Discussion Questions

1. Why is the Cyber layer essential in large-scale CPS?
2. How does MQTT support scalability and decoupling?
3. What latency constraints exist for CPS networking?
4. How can cyber-layer data improve CPS optimization?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain the role of the Cyber layer in CPS
- Integrate ESP32 CPS nodes with IoT networks
- Publish ML inference results using MQTT
- Visualize and analyze CPS data streams

---

## 🧾 Conclusion

This lab enabled **cyber-layer integration** of a CPS by connecting ESP32-based edge intelligence to IoT infrastructure using MQTT. The CPS is now observable and scalable at the system level.

---

## 📚 References

1. Edge Impulse Documentation  
2. MQTT Specification  
3. Node-RED Documentation  
4. CPS Architecture Literature  

---

### ▶️ Next Lab

**Lab 9 – CPS and Digital Twin Integration**
