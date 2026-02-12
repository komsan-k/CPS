# TinyML × CPS (5C) Lab Series

## ESP32 + LDR + LM73 + MPU6050

------------------------------------------------------------------------

## Overview

This lab series develops a complete **Cyber-Physical System (CPS)**
using ESP32 and multi-modal sensors.

You will progressively move from:

Sense → Convert → Model → Predict → Decide → Act

All activities align with the **5C CPS Architecture**.

------------------------------------------------------------------------

## Hardware Platform

**Controller** - ESP32 DevKit

**Sensors** - LDR (Light Intensity - Analog) - LM73 (Digital
Temperature - I2C) - MPU6050 (Accelerometer + Gyroscope - I2C)

**Actuators** - LED - Buzzer - Relay (optional)

------------------------------------------------------------------------

## CPS 5C Mapping

  CPS Layer       Implementation
  --------------- --------------------------------
  Connection      LDR, LM73, MPU6050
  Conversion      Filtering + Feature Extraction
  Cyber           MQTT + Node-RED Dashboard
  Cognition       TinyML Model (ESP32)
  Configuration   Automatic LED / Alarm Control

------------------------------------------------------------------------

## Lab Progression

### Lab 0 -- Communication Foundation

-   Setup ESP32 + MQTT
-   Build Digital Twin dashboard

### Lab 1 -- Multi-Sensor Data Acquisition

-   Stream LDR, LM73, MPU6050 data
-   Publish structured JSON

### Lab 2 -- Signal Conditioning

-   Implement moving average filter
-   Compare raw vs filtered signals

### Lab 3 -- Feature Extraction

-   Compute RMS acceleration
-   Compute light trend slope
-   Publish feature vectors

### Lab 4 -- Rule-Based Anomaly Detection

-   Detect abnormal vibration or temperature
-   Output NORMAL / WARNING / FAULT

### Lab 5 -- TinyML Classification

-   Train 2--3 class model
-   Deploy TensorFlow Lite Micro model

### Lab 6 -- TinyML Regression

-   Predict future lux or temperature trend
-   Evaluate MAE / RMSE

### Lab 7 -- Hybrid Intelligence

-   Combine ML output with safety rules

### Lab 8 -- Closed-Loop Control

-   Automatically activate LED or buzzer

### Lab 9 -- Digital Twin + Fault Injection

-   Inject sensor disturbance
-   Observe system response

### Lab 10 -- Energy & Latency Evaluation

-   Measure inference time
-   Compare ML vs rule-based

### Lab 11 -- Capstone CPS

Integrate: - All sensors - TinyML inference - Digital twin - Closed-loop
control

------------------------------------------------------------------------

## Deliverables (Each Lab)

-   ESP32 Arduino code
-   Node-RED flow
-   Screenshot or demo video
-   Short technical explanation (CPS mapping)

------------------------------------------------------------------------

## Final Outcome

You will build a fully functional TinyML-enabled Cyber-Physical System
running entirely on ESP32.
