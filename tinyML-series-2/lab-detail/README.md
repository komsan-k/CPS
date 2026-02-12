# 🧠 TinyML × CPS (5C) Lab Series

## ESP32 + LDR + LM73 + MPU6050

------------------------------------------------------------------------

## 🔷 System Vision

This lab series develops a complete **Cyber-Physical System (CPS)**
using ESP32 and multi-modal sensors. Students progressively move from:

**Raw sensing → Feature conversion → Edge intelligence → TinyML
cognition → Closed-loop control**

All aligned with the **5C CPS architecture**.

------------------------------------------------------------------------

## 📡 Hardware Platform

### 🖥 Controller

-   ESP32 DevKit

### 🌡 Sensors

-   **LDR** → Light intensity (analog)
-   **LM73** → Digital temperature (I²C)
-   **MPU6050** → 3-axis acceleration + gyro (I²C)

### 🔌 Actuators

-   LED (status / backup)
-   Buzzer (alarm)
-   Relay (optional)

------------------------------------------------------------------------

## 🏗 CPS 5C Mapping Across Labs

  CPS Layer          Implementation in This Series
  ------------------ ----------------------------------------------
  1️⃣ Connection      LDR (ADC), LM73 (I²C), MPU6050 (I²C)
  2️⃣ Conversion      Filtering, normalization, feature extraction
  3️⃣ Cyber           MQTT + Node-RED Digital Twin
  4️⃣ Cognition       TinyML classification / regression
  5️⃣ Configuration   Automatic control policy (LED / alarm)

------------------------------------------------------------------------

# 📘 Revised Lab Series Overview

------------------------------------------------------------------------

## 🔹 Lab 0 -- CPS Communication Foundation

**ESP32 + MQTT + Dashboard**

-   Publish system status (RSSI, uptime)
-   Create Digital Twin dashboard
-   Show sensor placeholders

🎯 5C Focus: **Connection (infrastructure)**

------------------------------------------------------------------------

## 🔹 Lab 1 -- Multi-Sensor Data Acquisition

**Objective:** Acquire and stream real sensor data.

Sensors: - LDR → ADC read - LM73 → Temperature read - MPU6050 →
Acceleration (ax, ay, az)

**Structured JSON Example:**

``` json
{
  "lux": 523,
  "temp": 29.4,
  "ax": 0.02,
  "ay": -0.01,
  "az": 0.98
}
```

🎯 5C Focus: **Connection**

------------------------------------------------------------------------

## 🔹 Lab 2 -- Signal Conditioning & Filtering

**Objective:** Convert raw signals into stable signals.

Techniques: - Moving average (LDR) - Offset correction (MPU6050) -
Temperature smoothing (LM73)

Dashboard comparison: - Raw vs filtered signals

🎯 5C Focus: **Conversion**

------------------------------------------------------------------------

## 🔹 Lab 3 -- Feature Extraction for TinyML

**Objective:** Prepare features usable by ML.

### For LDR

-   Slope (trend)
-   Rate of change

### For LM73

-   Temperature gradient

### For MPU6050

-   RMS acceleration
-   Magnitude:

\|a\| = sqrt(ax² + ay² + az²)

Publish feature vector instead of raw data.

🎯 5C Focus: **Conversion → Cyber**

------------------------------------------------------------------------

## 🔹 Lab 4 -- Rule-Based Anomaly Detection

**Objective:** Build baseline intelligence (without ML).

Examples: - Sudden vibration spike → fault - Rapid lux drop → lighting
anomaly - Temperature \> threshold → overheating

Output states: - NORMAL - WARNING - FAULT

🎯 5C Focus: **Cyber**

------------------------------------------------------------------------

## 🔹 Lab 5 -- TinyML Classification (2--3 Classes)

**Objective:** Deploy a TinyML classifier.

Example classes: - NORMAL_OPERATION - VIBRATION_FAULT - LIGHT_FAILURE

Steps: 1. Collect dataset 2. Train small dense NN (PC / Edge Impulse) 3.
Convert to TFLite Micro 4. Deploy to ESP32 5. Show live classification +
confidence

🎯 5C Focus: **Cognition**

------------------------------------------------------------------------

## 🔹 Lab 6 -- TinyML Regression (Prediction Trend)

**Objective:** Predict future system state.

Examples: - Predict next 5-sec lux - Predict temperature rise trend -
Predict vibration increase

Dashboard: - Actual vs predicted plot

Metrics: - MAE - RMSE

🎯 5C Focus: **Cognition**

------------------------------------------------------------------------

## 🔹 Lab 7 -- Hybrid Intelligence (Safe CPS)

Combine: - TinyML output - Rule-based safety constraints

Example: If model predicts NORMAL but vibration RMS \> safety threshold
→ override.

🎯 5C Focus: **Cyber + Cognition**

------------------------------------------------------------------------

## 🔹 Lab 8 -- Closed-Loop Configuration

**Objective:** Autonomous actuation.

Examples: - Lux predicted low → Turn on LED - Temperature rising →
Activate cooling signal - Vibration fault → Trigger buzzer

Control loop:

Sense → Convert → Predict → Decide → Act

🎯 5C Focus: **Configuration**

------------------------------------------------------------------------

## 🔹 Lab 9 -- Digital Twin Synchronization

Dashboard displays: - Physical state - Inferred state - Predicted
state - System decision

Fault injection: - Cover LDR - Shake MPU6050 - Heat LM73 slightly

🎯 5C Focus: **Cyber + Configuration**

------------------------------------------------------------------------

## 🔹 Lab 10 -- Energy & Latency Evaluation

Measure: - Inference time (ms) - CPU usage - Approximate power impact

Compare: - Rule-based vs TinyML

🎯 5C Focus: **System Optimization**

------------------------------------------------------------------------

## 🔹 Lab 11 -- Capstone: Smart Environmental Monitoring CPS

Students must integrate:

✔ LDR + LM73 + MPU6050\
✔ Feature extraction\
✔ TinyML inference\
✔ Digital Twin\
✔ Closed-loop control\
✔ Performance evaluation

Final output: - Fully working intelligent CPS node

🎯 5C Focus: **All Layers**

------------------------------------------------------------------------

# 📊 Unified Architecture

                ┌──────────────────────────┐
                │       CONFIGURATION      │
                │  LED / Buzzer / Control  │
                └──────────────────────────┘
                             ▲
                ┌──────────────────────────┐
                │        COGNITION         │
                │  TinyML Model (ESP32)    │
                └──────────────────────────┘
                             ▲
                ┌──────────────────────────┐
                │           CYBER          │
                │  MQTT + Node-RED Twin    │
                └──────────────────────────┘
                             ▲
                ┌──────────────────────────┐
                │        CONVERSION        │
                │ Filtering + Features     │
                └──────────────────────────┘
                             ▲
                ┌──────────────────────────┐
                │        CONNECTION        │
                │ LDR + LM73 + MPU6050     │
                └──────────────────────────┘

------------------------------------------------------------------------

# 🎯 Learning Outcomes

After completing the series, students will:

-   Implement multi-sensor CPS on ESP32
-   Design TinyML models for embedded deployment
-   Build digital twins using MQTT
-   Develop hybrid rule + ML safety systems
-   Evaluate latency and energy tradeoffs
-   Understand full 5C CPS lifecycle
