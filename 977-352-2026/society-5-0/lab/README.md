# 🧪 Graduate Lab
## Human-Centered Cyber-Physical System with Digital Twin and Policy Loop  
**ESP32 → Node-RED → Digital Twin → Policy → ESP32**

---

## 📘 Overview

This graduate-level laboratory is designed as a **standalone lab** that demonstrates the full **Society 5.0 cyber-physical loop**.

The lab integrates physical sensing, cyber intelligence, digital twin modeling, policy-based decision-making, and human-in-the-loop supervision into a **closed-loop Cyber-Physical System (CPS)**.

This lab is suitable for graduate courses in:
- Cyber-Physical Systems (CPS)
- Internet of Things (IoT)
- Society 5.0
- Digital Twin Systems

---

## 🎯 Lab Objective

To design, implement, and evaluate a **human-centered CPS** in which:
- Physical sensing is performed using an ESP32,
- Cyber intelligence is implemented using Node-RED and a Digital Twin,
- Policy-based constraints guide decision-making,
- Human supervision ensures trust, safety, and governance,

all operating within a **closed feedback loop** consistent with **Society 5.0 principles**.

---

## 🧠 Society 5.0 Focus

This lab emphasizes:

- Human-in-the-loop decision support  
- Policy as a first-class system constraint  
- Trustable and explainable CPS control  
- Feedback-driven CPS behavior beyond simple automation  

---

## 🏗️ System Architecture

```
Physical World (ESP32 + Sensor)
        ↓
     MQTT Communication
        ↓
Cyber Space (Node-RED)
   ├─ Data Validation
   ├─ Digital Twin Model
   ├─ Decision Logic
   └─ Policy Enforcement
        ↓
 Actuation Command (MQTT)
        ↓
Physical World (ESP32 + Actuator)
```

---

## 🧰 Hardware & Software

### Hardware
- ESP32 development board  
- LDR sensor (or temperature sensor)  
- LED (actuator)  

### Software
- Arduino IDE  
- MQTT Broker (Mosquitto)  
- Node-RED  
- Node-RED Dashboard  

---

## 📘 Learning Outcomes

After completing this lab, students will be able to:

1. Design a CPS with a **closed-loop feedback architecture**
2. Implement a **Digital Twin** using real-time sensor data
3. Encode **policy rules** as operational control constraints
4. Integrate **human-in-the-loop supervision**
5. Analyze CPS behavior under normal and constrained conditions

---

## 🔬 Lab Tasks

### 🔹 Task 1 — Physical Layer: ESP32 Sensing & Actuation

- Read sensor data (LDR)
- Publish data to MQTT as structured JSON
- Subscribe to actuator commands
- Control LED safely with bounds checking

**Example telemetry payload**
```json
{
  "device": "esp32_lab01",
  "timestamp": 1700000000,
  "ldr": 612
}
```

---

### 🔹 Task 2 — Cyber Layer: Node-RED Data Handling

- Subscribe to the sensor MQTT topic
- Validate data range and timestamps
- Display raw data on the dashboard
- Flag anomalies (out-of-range values)

**Key concept:**  
> Data correctness precedes intelligence.

---

### 🔹 Task 3 — Digital Twin Modeling

Create a Digital Twin that maintains:
- Current sensor state
- Expected operating range
- Short-term historical trends
- Deviation indicator (residual)

**Twin logic example**
```
Residual = |Measured − Expected|
```

Dashboard elements include:
- Live sensor value
- Expected operating band
- Deviation indicator (green / yellow / red)

---

### 🔹 Task 4 — Decision Intelligence

Implement explainable decision logic:
- If LDR < threshold → LED ON
- If LDR ≥ threshold → LED OFF

Log:
- Decision reason
- Digital Twin state at decision time

---

### 🔹 Task 5 — Policy Layer (Key Graduate Component)

Introduce a **policy constraint**, for example:

**Energy-Saving Policy**  
- LED cannot be ON for more than **60% of the time** in any **10-minute window**

Policy actions:
- Override decisions when violated
- Log policy enforcement events
- Notify the human supervisor

**Key insight:**  
> Policy constrains intelligence; it does not replace it.

---

### 🔹 Task 6 — Human-in-the-Loop Interface

The dashboard must include:
- Manual override controls (ON / OFF / AUTO)
- Decision explanation messages
- Policy violation alerts

Human actions include:
- Approving overrides
- Resetting the policy window

---

### 🔹 Task 7 — Closed-Loop Actuation

- Publish final actuation commands to ESP32
- Implement fail-safe defaults
- Ensure rate-limited actuation

---

## 🔁 CPS Feedback Loop Summary

```
Sense → Model → Decide → Constrain (Policy) → Act → Sense
```

---


## 📝 Required Deliverables

- ESP32 source code  
- Node-RED JSON flow  
- System architecture diagram  
- Dashboard screenshots  
- **6–8 page technical report** including:
  - Design rationale
  - Policy impact analysis
  - Society 5.0 reflection

---

## 🧠 Reflection Questions (Graduate Level)

1. How does policy enforcement change system behavior?
2. What risks arise if policy constraints are removed?
3. Where does human trust emerge in this CPS?
4. How does this lab embody Society 5.0 principles?

---

## 🎯 Why This Lab Is Graduate-Quality

- Goes beyond automation toward **governance-aware CPS**
- Explicit Digital Twin modeling
- Human-in-the-loop control
- Policy as a first-class system element
- Direct alignment with **Society 5.0 deep drivers**


