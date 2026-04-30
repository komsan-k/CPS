# Lab Report Template  
## Human-Centered Cyber-Physical System with Digital Twin and Policy Loop  
### Society 5.0 Laboratory: ESP32 → Node-RED → Digital Twin → Policy → ESP32

---

## 1. Student Information

- **Course:** 977-352 Cyber-Physical Systems
- **Lab Title:** Human-Centered CPS with Digital Twin and Policy Loop
- **Student Name:** __________________________________
- **Student ID:** __________________________________

---

## 2. Abstract

Write 150–250 words summarizing the lab.

Include:
- CPS system implemented
- ESP32, MQTT, Node-RED, and Digital Twin roles
- Policy rule used
- Human-in-the-loop function
- Main result or observation

**Abstract:**  
................................................................................  
................................................................................  
................................................................................  
................................................................................

---

## 3. Objectives

1. Design a closed-loop CPS architecture based on Society 5.0 principles.
2. Implement physical sensing and actuation using ESP32.
3. Use MQTT for communication between the physical and cyber layers.
4. Develop a Node-RED Digital Twin for real-time monitoring.
5. Apply policy-based control constraints.
6. Integrate human-in-the-loop supervision using a dashboard.
7. Evaluate CPS behavior under normal and policy-constrained conditions.

---

## 4. Background and Theory

### 4.1 Society 5.0

Explain Society 5.0 and why it emphasizes human-centered cyber-physical integration.

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.2 Cyber-Physical Systems

Explain how CPS integrates sensing, computation, communication, decision-making, and actuation.

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.3 Digital Twin

Explain the role of a Digital Twin in this lab:
- real-time sensor representation
- expected operating range
- deviation or residual
- dashboard visualization

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.4 Policy Loop

Explain how policy rules constrain system decisions.

Example:
> LED cannot be ON for more than 60% of the time in any 10-minute window.

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.5 Human-in-the-Loop Control

Explain why human supervision is important for trust, safety, explainability, and governance.

**Write here:**  
................................................................................  
................................................................................  
................................................................................

---

## 5. Hardware and Software Requirements

### 5.1 Hardware

Component Description 

### 5.2 Software

Software Purpose

---

## 6. System Architecture

### 6.1 Overall System Flow

```text
Physical World: ESP32 + Sensor
        ↓
MQTT Communication
        ↓
Cyber Space: Node-RED
   ├── Data Validation
   ├── Digital Twin Model
   ├── Decision Logic
   └── Policy Enforcement
        ↓
Actuation Command via MQTT
        ↓
Physical World: ESP32 + LED Actuator
```

### 6.2 Architecture Diagram

**Figure 1. Human-centered CPS architecture**

[Insert figure here]

### 6.3 CPS Feedback Loop

```text
Sense → Model → Decide → Constrain by Policy → Act → Sense
```

**Explanation:**  
................................................................................  
................................................................................  
................................................................................

---

## 7. MQTT Communication Design

### 7.1 MQTT Topics

| Topic | Direction | Purpose |
|---|---|---|
| __________________ | ESP32 → Node-RED | Sensor telemetry |
| __________________ | Node-RED → ESP32 | LED command |
| __________________ | Node-RED Dashboard | Digital Twin status |
| __________________ | Node-RED Dashboard | Policy alert |

### 7.2 Telemetry Payload

Example JSON telemetry:

```json
{
  "device": "esp32_lab01",
  "timestamp": 1700000000,
  "ldr": 612
}
```

Record your actual payload format:

```json
{
}
```

### 7.3 Command Payload

Record the command message sent from Node-RED to ESP32.

```json
{
}
```

---

## 8. Experimental Procedure

### 8.1 Task 1: Physical Layer — ESP32 Sensing and Actuation

Describe how ESP32 was programmed to:
- read sensor data
- publish structured JSON data
- subscribe to actuator commands
- control the LED safely

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 8.2 Task 2: Cyber Layer — Node-RED Data Handling

Describe how Node-RED was configured to:
- subscribe to MQTT telemetry
- validate sensor range and timestamp
- display raw data
- detect abnormal values

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 8.3 Task 3: Digital Twin Modeling

Describe the Digital Twin logic.

Formula:

```text
Residual = |Measured Value − Expected Value|
```

Include current sensor state, expected operating range, residual, and alert color/state.

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 8.4 Task 4: Decision Intelligence

Example logic:

```text
If LDR < threshold → LED ON
If LDR ≥ threshold → LED OFF
```

- **Threshold value:** __________________

Explain the decision reasoning:

................................................................................  
................................................................................  
................................................................................

### 8.5 Task 5: Policy Layer

- **Policy name:** __________________________________

**Policy rule:**  
................................................................................

Explain how the policy was enforced:
- override decision
- log event
- notify human supervisor

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 8.6 Task 6: Human-in-the-Loop Interface

Describe dashboard functions:
- manual override: ON / OFF / AUTO
- decision explanation message
- policy violation alert
- reset or approve control

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 8.7 Task 7: Closed-Loop Actuation

Describe final command transmission back to ESP32:
- command topic
- fail-safe condition
- rate-limiting method
- LED response

**Write here:**  
................................................................................  
................................................................................  
................................................................................

---

## 9. Node-RED Flow Description

### 9.1 Main Nodes Used

| Node | Function |
|---|---|
| MQTT In | __________________________________ |
| JSON | __________________________________ |
| Function | __________________________________ |
| Switch | __________________________________ |
| Dashboard Gauge | __________________________________ |
| Dashboard Button | __________________________________ |
| MQTT Out | __________________________________ |
| Debug | __________________________________ |

### 9.2 Node-RED Flow Screenshot

**Figure 2. Node-RED flow**

[Insert screenshot here]

### 9.3 Dashboard Screenshot

**Figure 3. Human-in-the-loop dashboard**

[Insert screenshot here]

---

## 10. ESP32 Program Summary

### 10.1 Sensor Reading
................................................................................

### 10.2 MQTT Publishing
................................................................................

### 10.3 MQTT Subscription
................................................................................

### 10.4 LED Actuation
................................................................................

### 10.5 Safety or Bounds Checking
................................................................................

---

## 11. Results

### 11.1 Sensor Data Results

| Test No. | Condition | Sensor Value | Digital Twin State | Residual | Alert Level |
|---:|---|---:|---|---:|---|
| 1 | __________________ | ______ | ______ | ______ | ______ |
| 2 | __________________ | ______ | ______ | ______ | ______ |
| 3 | __________________ | ______ | ______ | ______ | ______ |
| 4 | __________________ | ______ | ______ | ______ | ______ |
| 5 | __________________ | ______ | ______ | ______ | ______ |

### 11.2 Decision Logic Results

| Test No. | Sensor Value | Initial Decision | Policy Status | Final Action | Explanation |
|---:|---:|---|---|---|---|
| 1 | ______ | ______ | ______ | ______ | ______ |
| 2 | ______ | ______ | ______ | ______ | ______ |
| 3 | ______ | ______ | ______ | ______ | ______ |
| 4 | ______ | ______ | ______ | ______ | ______ |

### 11.3 Policy Enforcement Results

| Event No. | Time | Policy Condition | Action Taken | Human Notification |
|---:|---|---|---|---|
| 1 | ______ | ______ | ______ | ______ |
| 2 | ______ | ______ | ______ | ______ |
| 3 | ______ | ______ | ______ | ______ |

### 11.4 Manual Override Results

| Test No. | Mode | Human Input | System Response | Result |
|---:|---|---|---|---|
| 1 | AUTO | ______ | ______ | ______ |
| 2 | ON | ______ | ______ | ______ |
| 3 | OFF | ______ | ______ | ______ |
| 4 | AUTO | ______ | ______ | ______ |

---

## 12. Analysis and Discussion

Discuss:
- Was the CPS loop stable?
- Did the Digital Twin correctly represent the physical system?
- Did policy enforcement change the final action?
- Was the decision explanation understandable?
- Did manual override improve trust and safety?
- What happened when sensor values were abnormal?
- What risks arise if policy constraints are removed?

**Write here:**  
................................................................................  
................................................................................  
................................................................................  
................................................................................  
................................................................................

---

## 13. Society 5.0 Reflection

### 13.1 How does this lab embody Society 5.0 principles?
................................................................................  
................................................................................

### 13.2 Where does human trust emerge in this CPS?
................................................................................  
................................................................................

### 13.3 How does policy enforcement change system behavior?
................................................................................  
................................................................................

### 13.4 What risks arise if policy constraints are removed?
................................................................................  
................................................................................

---

## 14. Limitations

1. ..............................................................................
2. ..............................................................................
3. ..............................................................................
4. ..............................................................................

---

## 15. Improvements and Future Work

1. ..............................................................................
2. ..............................................................................
3. ..............................................................................
4. ..............................................................................

---

## 16. Conclusion

Summarize what was implemented, what was learned, and how the system demonstrates Society 5.0 CPS concepts.

**Conclusion:**  
................................................................................  
................................................................................  
................................................................................  
................................................................................

---

## 17. Required Deliverables Checklist

| Deliverable | Completed? |
|---|---|
| ESP32 source code | ☐ |
| Node-RED JSON flow | ☐ |
| System architecture diagram | ☐ |
| Dashboard screenshots | ☐ |
| Policy impact analysis | ☐ |
| Society 5.0 reflection | ☐ |
| 6–8 page technical report | ☐ |

---

## 18. References

1. ..............................................................................
2. ..............................................................................
3. ..............................................................................
4. ..............................................................................

---

## 19. Appendix

### Appendix A: ESP32 Source Code

```cpp
// Paste ESP32 code here
```

### Appendix B: Node-RED JSON Flow

```json
{
}
```

### Appendix C: Additional Screenshots

[Insert screenshots here]

### Appendix D: Raw Experimental Data

| Time | Sensor Value | Decision | Policy State | Final Action |
|---|---:|---|---|---|
| ______ | ______ | ______ | ______ | ______ |
| ______ | ______ | ______ | ______ | ______ |
| ______ | ______ | ______ | ______ | ______ |

---

## 20. Instructor Assessment Rubric

| Criterion | Max Score | Score |
|---|---:|---:|
| CPS architecture design | 15 | ____ |
| ESP32 sensing and actuation | 15 | ____ |
| MQTT communication | 10 | ____ |
| Node-RED Digital Twin implementation | 15 | ____ |
| Policy enforcement | 15 | ____ |
| Human-in-the-loop dashboard | 10 | ____ |
| Results and analysis | 10 | ____ |
| Society 5.0 reflection | 10 | ____ |
| **Total** | **100** | **____** |

**Instructor comments:**  
................................................................................  
................................................................................  
................................................................................
