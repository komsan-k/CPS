# SensorML → TinyML → CPS Architecture (ESP32 Lab)

## 1. Architecture Overview (Concept)

### Goal
Enable a **Cyber-Physical System (CPS)** to *understand its sensors* through **SensorML metadata**, perform **on-device TinyML inference** on an ESP32, and safely control actuators.

### System Flow

1. **SensorML (Metadata)**  
   Describes sensor characteristics: unit, sampling rate, calibration, and uncertainty.

2. **ESP32 Sensing**  
   Reads raw sensor signals (e.g., LDR, temperature, IMU).

3. **Feature Extraction**  
   Converts raw data into features (mean, standard deviation, slope, FFT bins, etc.).

4. **TinyML Inference (TFLM)**  
   Performs classification or prediction locally on the device.

5. **Decision & Safety Layer**  
   Applies safety rules (thresholds, debounce, hysteresis, fail-safe logic).

6. **Actuation**  
   Controls actuators such as LEDs, relays, buzzers, or servos.

7. **(Optional) Telemetry & Integration**  
   Publishes data via MQTT/HTTP and shares SensorML metadata with a central system.

---

## 2. Recommended Lab (End-to-End)

### Lab Title
**Lab: Self-Describing Sensor with On-Device TinyML Control (ESP32)**

### Learning Outcomes
Students will be able to:
- Describe sensors using **SensorML (self-describing sensors)**  
- Map metadata to **sampling, units, and calibration** on ESP32  
- Build a **TinyML pipeline** (windowing → features → inference)  
- Use ML outputs to control actuators with a **CPS safety layer**

---

## 3. Essential SensorML Subset for the Lab

Full SensorML is not required for the first class.  
A **minimal but conceptually correct subset** is sufficient.

### Required Elements
- `identifier / name`
- `observedProperty`
- `uom` (unit of measurement)
- `samplingRate` or `samplePeriod`
- `calibration` (offset, scale)
- `accuracy / uncertainty`
- `location` (optional)

---

## 4. Simplified SensorML Example (Lab Use)

Store as `sensorml.xml` or embed as a string in ESP32 firmware.

```xml
<SensorML>
  <member>
    <System>
      <identifier>LDR_ESP32_01</identifier>
      <outputs>
        <output name="light">
          <uom>adc_counts</uom>
          <calibration>
            <scale>1.0</scale>
            <offset>0.0</offset>
          </calibration>
          <samplingRateHz>20</samplingRateHz>
          <uncertainty>0.05</uncertainty>
        </output>
      </outputs>
    </System>
  </member>
</SensorML>
```

### Teaching Mapping Ideas
- `samplingRateHz` → ESP32 `delay()` or hardware timer  
- `scale`, `offset` → ADC calibration before feature extraction  
- `uncertainty` → safety margin / hysteresis logic  

---

## 5. TinyML Model for LDR (Simple & Effective)

### Example Task
- **3-class classification**: `dark`, `normal`, `bright`

### Configuration
- Input window: 1–2 seconds (e.g., 40 samples @ 20 Hz)  
- Features: mean, std, min, max, slope  
- Model: small **Dense Neural Network (TFLM)** or Decision Tree  

---

## 6. ESP32 Code Structure

### Modules
- `sensorml_parser.h/.cpp` – read sampling rate, scale, offset  
- `features.h/.cpp` – compute features  
- `model.h` – TFLM model array  
- `inference.cpp` – run inference  
- `controller.cpp` – safety logic + actuator control  

### Core Pseudocode

```text
load SensorML config
set samplingRate from SensorML

loop:
  read sensor
  apply calibration (scale/offset)
  push into window buffer
  if window full:
     features = compute_features(window)
     y = tinyml_infer(features)
     action = safety_layer(y, uncertainty)
     actuate(action)
```

---

## 7. Safety Layer (Critical for CPS)

Teach at least **three professional CPS techniques**:

1. **Hysteresis** – prevent flickering  
2. **Debounce / Majority Vote** – require repeated predictions  
3. **Fail-Safe Mode** – safe state if confidence is low  

### Example Rules
- If `confidence < 0.6` → LED = OFF (safe state)  
- Relay activates only after **3 consecutive identical predictions**

---

## 8. Student Deliverables

- `sensorml.xml` for their own sensor (LDR / Temp / IMU)  
- ESP32 code mapping SensorML → sampling & calibration  
- Short report including:
  - Confusion matrix (small test set)
  - Inference latency
  - Memory footprint (Flash / RAM)
  - Sensor signal vs predicted class plot
- Demo video (1–2 minutes)

---

## 9. Simplified Rubric (10 Points)

| Item | Points |
|----|----|
| SensorML correctness & mapping | 2 |
| Data pipeline & features | 2 |
| TinyML inference | 2 |
| Safety layer | 2 |
| Documentation & demo | 2 |

---

## 10. Optional Modern Extensions

- Publish SensorML to Node-RED via MQTT:  
  `device/ldr01/metadata`
- Publish inference results:  
  `device/ldr01/pred`
- Dashboard showing:
  - Sensor metadata  
  - Real-time plots  
  - Actuator states  

---

**This lab demonstrates a full CPS loop:  
Self-describing sensors → Edge intelligence → Safe actuation.**
