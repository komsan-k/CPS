# Lab 10: Edge-to-Cloud Monitoring for TinyML Systems  
## On-Device Intelligence with Cloud Visibility  
### CPS Focus: C5–C6 (Control → Communication)

---

## 1. Lab Overview

This laboratory extends **TinyML-based edge intelligence** with **cloud connectivity for monitoring and visualization**.

Unlike cloud AI, **all ML inference remains on the ESP32**. Only **low-bandwidth, high-level information**—such as predictions, confidence values, anomaly scores, and system status—is sent to the cloud.

**Key principle demonstrated in Lab 10:**

> **Think locally (TinyML), observe globally (Cloud).**

Students will publish TinyML results via **Wi‑Fi (MQTT or HTTP)** and visualize system behavior on a **dashboard (e.g., Node‑RED)**.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Explain **edge vs cloud roles** in TinyML‑based CPS  
- Connect ESP32 to **Wi‑Fi networks**  
- Publish **TinyML inference results** to the cloud  
- Build a **real-time monitoring dashboard**  
- Analyze **communication cost, latency, and scalability**

---

## 3. Background Theory

### 3.1 Why Edge-to-Cloud CPS?

**Pure edge systems**
- Very fast  
- Privacy‑preserving  
- Limited global visibility  

**Pure cloud systems**
- Scalable  
- High latency  
- Privacy and bandwidth concerns  

**Hybrid CPS (Edge + Cloud)** combines both:

- **Edge:** sensing, inference, control  
- **Cloud:** monitoring, logging, analytics  

This hybrid model dominates **Industry 4.0 / Society 5.0** systems.

---

### 3.2 Data Minimization Principle

In TinyML‑CPS systems:

- ❌ Raw sensor data stays on-device  
- ✅ Only **semantic results** are transmitted  

Typical transmitted data:
- Predicted class label  
- Anomaly score  
- Confidence value  
- Timestamp / system status  

This significantly reduces **bandwidth, energy consumption, and privacy risk**.

---

## 4. CPS Perspective (C5–C6)

### CPS Mapping for Lab 10

| CPS Layer | Role |
|---------|------|
| Physical | Sensors & actuators |
| Cyber | Feature extraction |
| Cognition | TinyML inference |
| Control | Safe actuation |
| Communication (C6) | Edge‑to‑cloud reporting |
| Cloud | Monitoring & logging |

**Lab 10 introduces system‑level visibility without sacrificing autonomy.**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Sensor + actuator from Labs 6–9  
- Wi‑Fi access point  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Wi‑Fi library  
- MQTT or HTTP client library  
- Node‑RED (or equivalent dashboard tool)

---

## 6. System Architecture

```
[Sensor Stream]
       ↓
 Feature Extraction
       ↓
 TinyML Inference (On ESP32)
       ↓
 Control Logic & Actuation
       ↓
 Tiny Data Packet (label, score)
       ↓
 Wi‑Fi (MQTT / HTTP)
       ↓
 Cloud Dashboard
```

**Key Rule:**
- ❌ No raw data upload  
- ✅ Only inference results are transmitted  

---

## 7. Lab Tasks

### Task 1: Connect ESP32 to Wi‑Fi

```cpp
#include <WiFi.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}
```

---

### Task 2: Prepare TinyML Result Packet

```cpp
struct TinyMLResult {
  int label;
  float confidence;
  float anomalyScore;
};
```

Only **compact semantic data** is transmitted.

---

### Task 3: Publish Data via MQTT (Example)

```cpp
void publishResult(int label, float score) {
  String payload = "{";
  payload += "\"label\":" + String(label) + ",";
  payload += "\"score\":" + String(score);
  payload += "}";

  client.publish("tinyml/esp32/status", payload.c_str());
}
```

---

### Task 4: Dashboard Visualization

Students create a dashboard showing:
- Current classification  
- Anomaly score trend  
- System status (ON / OFF)  

Recommended update rate:
- **1–2 messages per second** (sufficient for monitoring)

---

### Task 5: Bandwidth & Latency Analysis

Students measure:
- Message size (bytes)  
- Publish interval  
- Network latency  

Compare:
```
Raw data upload   >>   TinyML result upload
```

---

## 8. Observations and Discussion

Students should analyze:
- Responsiveness of dashboard vs edge action  
- Stability of CPS during network loss  
- Energy cost of communication  

**Important Insight:**  
> TinyML systems must never depend on the cloud to function.

---

## 9. Edge Failure Scenario Test

Students simulate:
- Wi‑Fi disconnection  
- Cloud unavailability  

Expected behavior:
- Local inference continues  
- Actuation remains correct  
- Cloud updates resume automatically  

This validates **CPS resilience**.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Connectivity | Stable Wi‑Fi connection | 20% |
| Data design | Minimal, meaningful payload | 20% |
| Cloud integration | Live dashboard updates | 25% |
| CPS robustness | Edge works without cloud | 25% |
| Discussion | Architecture reasoning | 10% |

---

## 11. Student Reflection Questions

- Why should raw data stay at the edge?  
- What happens if cloud latency increases?  
- How would this architecture scale to **1,000 devices**?

---

## 12. Preparation for Next Lab

In **Lab 11 (Capstone)**, students will:
- Design a **complete TinyML CPS system**  
- Combine sensing, inference, control, and cloud monitoring  
- Evaluate **performance, latency, and robustness**

---

## 13. Key Takeaway

**Lab 10 adds visibility without dependency:**  
> TinyML keeps intelligence at the edge, while the cloud provides insight, monitoring, and scalability.
