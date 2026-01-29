# Transportation 5.0 CPS Lab – Report Template

## 1. Introduction
Briefly describe:
- Intelligent Transportation Systems (ITS)
- Transportation 5.0 concept
- Objective of this lab

---

## 2. System Architecture

### 2.1 Overall CPS Architecture
Describe the closed-loop:
ESP32 → MQTT → Node-RED → Decision → ESP32

Include a block diagram or screenshot.

### 2.2 CPS 5C Mapping
Explain how your system maps to:
- Connection
- Conversion
- Cyber
- Cognition
- Configuration

---

## 3. Hardware Implementation

### 3.1 ESP32 Setup
- Sensors used
- GPIO mapping
- Actuators (traffic LEDs)

### 3.2 Firmware Design
- Telemetry structure
- Command handling
- Safety / fail-safe behavior

---

## 4. Node-RED & Control Logic

### 4.1 Data Processing & Digital Twin
Explain:
- Normalization
- Stored state variables
- Demand estimation

### 4.2 Policy-Aware Controller
Explain:
- Eco mode logic
- Pedestrian priority logic
- Phase transition rules

### 4.3 Explainability
Show examples of “reason” messages and explain them.

---

## 5. Experiments & Results

### 5.1 Experimental Scenarios
1. Baseline (no eco, no priority)
2. Eco mode only
3. Eco + pedestrian priority

### 5.2 Metrics
Report (tables or plots):
- Pedestrian waiting time
- Phase switching frequency
- Unserved demand time

### 5.3 Discussion
Compare scenarios and explain trade-offs.

---

## 6. Transportation 5.0 Perspective
Discuss how your system demonstrates:
- Human-centered design
- Sustainability
- Policy-aware CPS behavior

---

## 7. Conclusion
Summarize findings and limitations.

---

## Appendix
- Node-RED flow screenshot
- ESP32 code excerpt
- Raw data (if any)
