# CPS 5C Architecture Mapping for TinyML Labs (ESP32)

This document maps **Lab 0 → Lab 11** to the **CPS 5C architecture**:

- **C1: Connection** — sensing, wiring, acquisition, connectivity
- **C2: Conversion** — preprocessing, feature extraction, local analytics
- **C3: Cyber** — data/model management, system-level integration, digital representation
- **C4: Cognition** — learning, inference, decision-making, diagnostics
- **C5: Configuration** — closed-loop control, self-adjustment, optimization, resilience

> Note: Many labs span multiple “C” levels. The **Primary** column indicates the dominant level(s).

---

## Mapping Table (Lab 0 → Lab 11)

| Lab | Title (Short) | Primary 5C Level(s) | C1 Connection | C2 Conversion | C3 Cyber | C4 Cognition | C5 Configuration | Evidence / Typical Outputs |
|---:|---|---|:--:|:--:|:--:|:--:|:--:|---|
| 0 | Setup & Hello TinyML | C4 (+C1) | ✅ | ◻️ | ◻️ | ✅ | ◻️ | Toolchain + first “inference-like” decision; LED response |
| 1 | Digital I/O Sensors | C1 | ✅ | ◻️ | ◻️ | ◻️ | ✅ | Reliable GPIO reading (debounce) + actuator control |
| 2 | Analog Sensors & Logging | C2 (+C1) | ✅ | ✅ | ◻️ | ◻️ | ◻️ | ADC scaling, smoothing, clean serial/CSV dataset |
| 3 | Feature Engineering | C2 | ✅ | ✅ | ◻️ | ◻️ | ◻️ | Feature extractor (mean/RMS/slope); feature table |
| 4 | Train & Deploy Classifier | C4 (+C3) | ✅ | ✅ | ✅ | ✅ | ✅ | Model deployment; on-device inference drives LED patterns |
| 5 | Quantization & Budget | C4 (+C2) | ◻️ | ✅ | ✅ | ✅ | ◻️ | INT8 vs float comparisons: size/latency/accuracy |
| 6 | Windowing (Streaming) | C2+C4 | ✅ | ✅ | ◻️ | ✅ | ✅ | Sliding window buffer + periodic inference + stable response |
| 7 | Activity Recognition | C4 (+C2) | ✅ | ✅ | ◻️ | ✅ | ✅ | Multi-axis features/model; live classification output |
| 8 | Anomaly Detection | C4 (+C3) | ✅ | ✅ | ✅ | ✅ | ✅ | Normal-only model; anomaly score threshold; alarm actuation |
| 9 | Smart Control | C5 (+C4) | ✅ | ✅ | ◻️ | ✅ | ✅ | Post-processing/hysteresis; stable actuator behavior |
| 10 | Edge-to-Cloud Reporting | C3 (+C1) | ✅ | ✅ | ✅ | ✅ | ◻️ | Telemetry (MQTT/HTTP): prediction + confidence + timestamp |
| 11 | Capstone System | C1→C5 (All) | ✅ | ✅ | ✅ | ✅ | ✅ | End-to-end CPS with evaluation: accuracy/latency/memory/power |

Legend: ✅ = strongly emphasized, ◻️ = not a major focus in that lab

---

## Per-Lab 5C Notes (Quick Justification)

### Lab 0 — Setup & “Hello TinyML”
- **C1:** Establishes the MCU + toolchain “connection” to run embedded apps  
- **C4:** Introduces **decision-making/inference concept** under resource constraints

### Lab 1 — Digital I/O Sensors (No ML yet)
- **C1:** Reliable digital sensing + wiring + sampling correctness  
- **C5:** Basic closed-loop actuation (debounced input → stable output)

### Lab 2 — Analog Sensors & Data Logging
- **C1:** Sensor acquisition pipeline  
- **C2:** ADC scaling, smoothing, logging → foundation for ML dataset creation

### Lab 3 — Feature Engineering on ESP32
- **C2:** Core conversion stage: raw signal → compact informative features

### Lab 4 — Train + Deploy + Infer
- **C3:** Model artifacts (exported arrays), versioning, integration into firmware  
- **C4:** On-device inference, performance evaluation  
- **C5:** Decisions trigger actuator patterns → closed-loop CPS behavior

### Lab 5 — Quantization & Memory Budget
- **C2/C4:** Optimization of inference (INT8) as part of conversion + cognition performance

### Lab 6 — Streaming Inference with Windowing
- **C2:** Time-series windowing and real-time preprocessing  
- **C4:** Continuous inference  
- **C5:** Real-time responses under latency constraints

### Lab 7 — Motion / Vibration Activity Recognition
- **C2:** Multi-axis feature preparation  
- **C4:** Classification and decision logic in the edge device  
- **C5:** Action on recognition results

### Lab 8 — Anomaly Detection
- **C3:** “Normal profile” and thresholds as cyber knowledge  
- **C4:** Anomaly scoring/decision logic  
- **C5:** Fault/alarm configuration actions

### Lab 9 — TinyML + Actuation Control
- **C5:** Main focus: stable control rules, hysteresis, safety constraints  
- **C4:** ML output post-processing before actuation

### Lab 10 — Edge-to-Cloud Reporting
- **C3:** Cyber layer integration: telemetry, dashboards, remote monitoring  
- **C1:** Connectivity stack (Wi‑Fi/MQTT/HTTP) is essential

### Lab 11 — Capstone
- **C1→C5:** Full CPS lifecycle: sensing → preprocessing → cyber integration → inference → control and evaluation

---

## Suggested “5C Tags” (for folders / LMS)
- **C1:** `connection`, `sensing`, `gpio`, `acquisition`
- **C2:** `conversion`, `preprocess`, `features`, `windowing`
- **C3:** `cyber`, `telemetry`, `dashboard`, `digital-twin-ready`
- **C4:** `cognition`, `model`, `inference`, `anomaly`, `classification`
- **C5:** `configuration`, `control`, `hysteresis`, `safety`, `optimization`
