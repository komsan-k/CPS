# 🧑‍🎓 Midterm Project --- Digital Twin + TinyML 

## 🎯 Project Goal

Build a simple **Digital Twin system** using **ESP32 + MQTT + Node‑RED**
and extend it with **TinyML intelligence**.

You will create a Cyber‑Physical System that: Sensor → Decision →
Actuator → Visualization

------------------------------------------------------------------------

## 🧩 What You Will Build

-   ESP32 controls an LED machine
-   LDR sensor monitors brightness (health/perception)
-   MQTT sends data to Node‑RED
-   Node‑RED Dashboard acts as the Digital Twin
-   TinyML adds smart prediction or classification

------------------------------------------------------------------------

## 🛠️ Project Steps (Simple View)

### Phase 1 --- Manual Control

-   Button interrupt toggles LED
-   Basic ESP32 control works reliably

### Phase 2 --- Perception

-   LDR reads brightness
-   Detect abnormal condition (LED ON but low light)

### Phase 3 --- IoT Communication

-   Publish data via MQTT
-   Dashboard shows LED + LDR values

### Phase 4 --- Digital Twin

-   Node‑RED Dashboard mirrors the physical system
-   Charts + indicators update in real time

### Phase 5 --- TinyML Intelligence

Use ideas from TinyML Labs:

-   Lab 2: Data logging (analog sensors)
-   Lab 3: Feature extraction
-   Lab 4: Train small classifier
-   Lab 5: Quantization (optional)
-   Lab 6: Time‑series windowing

Example: Bright / Normal / Dark prediction → control LED safely

------------------------------------------------------------------------

## 📦 What You Must Submit

-   ESP32 Arduino code
-   Node‑RED JSON flow
-   Dashboard screenshot
-   Short report (3--5 pages)
-   Demo video (1--2 minutes)

------------------------------------------------------------------------

## 📊 Grading (Simple)

-   Manual Control: 15%
-   LDR Perception: 15%
-   MQTT + Node‑RED: 20%
-   Digital Twin Dashboard: 20%
-   TinyML Intelligence: 20%
-   Report & Presentation: 10%

------------------------------------------------------------------------

## 📝 Report Template (Very Simple)

1.  System Overview
2.  Hardware Setup
3.  Digital Twin Architecture
4.  TinyML Model (if used)
5.  Results (charts/screenshots)
6.  Discussion

------------------------------------------------------------------------

## ✅ Checklist Before Submission

-   [ ] LED control works
-   [ ] LDR data visible on dashboard
-   [ ] MQTT connected
-   [ ] Digital Twin updates live
-   [ ] TinyML or rule‑based logic added
-   [ ] Report completed

------------------------------------------------------------------------

## 💡 Key Idea

This project demonstrates a full CPS loop:

**Physical → Cyber → Cognition → Control**
