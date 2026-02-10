# 🧠 CPS Lab: Lux Prediction Using LDR Sensor
ESP32 + MQTT + Node-RED (Cognition Layer Demonstration)

---

## 📌 Lab Title
**Simple Cognition in a Cyber-Physical System: Predicting Future Light Intensity Using LDR Data**

---

## 🎯 Lab Objective
This lab demonstrates a **Cyber-Physical System (CPS)** with a **simple cognition layer** that predicts **future light intensity (lux)** using real-time LDR sensor data.

Students will:
- Build an end-to-end CPS using ESP32, MQTT, and Node-RED
- Apply trend-based prediction (no machine learning required)
- Implement anticipatory control using predicted sensor values
- Understand CPS operation using the **5C architecture**

---

## 🧩 CPS 5C Architecture Mapping

| CPS Level | Implementation |
|---------|----------------|
| Connection | ESP32 reads LDR (ADC) |
| Conversion | ADC → relative lux + smoothing |
| Cyber | Node-RED data buffering & visualization |
| Cognition | Lux trend estimation & prediction |
| Configuration | Predictive LED control |

---

## 🛠️ Hardware Requirements
- ESP32 Dev Board
- LDR + resistor (e.g., 10 kΩ)
- LED + resistor
- Breadboard & jumper wires

---

## 💻 Software Requirements
- Arduino IDE + ESP32 core
- MQTT Broker (Mosquitto / local / cloud)
- Node-RED
- node-red-dashboard

---

## 📡 System Overview

LDR → ESP32 → MQTT → Node-RED  
                     ↓  
                Cognition Layer  
                     ↓  
                Predict Future Lux  
                     ↓  
                LED Control

---

## 📤 MQTT Topics

| Purpose | Topic |
|-------|-------|
| LDR Data | `cps/ldr/data` |
| LED Command | `cps/led/cmd` |
| Status | `cps/ldr/status` |

---

## 📦 Data Format (MQTT JSON)

```json
{
  "device": "esp32-ldr-01",
  "ts": 1700000000000,
  "adc": 1234,
  "lux_index": 456.7,
  "lux_smooth": 440.2
}
```

---

## 🧠 Cognition Logic (Trend Prediction)

The cognition layer estimates a linear trend from recent lux samples:

L̂(t+Δ) = L(t) + slope × Δ

Where Δ = 5 seconds.

---

## ⚙️ Control Logic

| Predicted Lux | Action |
|--------------|--------|
| < 50 | LED ON |
| > 200 | LED OFF |

---

## 📊 Node-RED Dashboard
- Live lux vs predicted lux chart
- Numeric indicators
- Manual override switch
- Auto-control log

Access:
```
http://<node-red-ip>:1880/ui
```

---

## 🧪 Student Tasks
1. Verify LDR data reception in Node-RED
2. Observe predicted vs actual lux
3. Demonstrate predictive LED control

---

## 🎓 Learning Outcomes
Students will be able to:
- Explain CPS using the 5C model
- Design simple cognition without ML
- Implement anticipatory control
- Integrate ESP32, MQTT, and Node-RED

---

## 🏁 Conclusion
This lab proves that **explainable CPS intelligence** can be achieved using simple, interpretable models—ideal for CPS, Digital Twin, and Society 5.0 education.
