# Lab 0: Motion Detection Using Smartphone Sensors with Edge Impulse

## Overview
This laboratory introduces motion detection using a smartphone as a sensing node and Edge Impulse as the machine learning platform.

Students use a smartphone’s built-in accelerometer and gyroscope to collect motion data, train a machine learning model, and perform real-time motion classification. This lab emphasizes data-driven Cyber-Physical System (CPS) design before embedded deployment.

---

## Learning Objectives
After completing this lab, students will be able to:
- Explain how smartphones act as CPS sensing nodes
- Collect labeled motion data using phone sensors
- Train a motion classification model in Edge Impulse
- Perform real-time motion inference
- Understand the CPS data pipeline (sensor → model → decision)

---

## CPS Perspective
| CPS Component | Implementation |
|--------------|----------------|
| Physical World | Human / device motion |
| Sensor | Smartphone accelerometer & gyroscope |
| Cyber Layer | Edge Impulse ML pipeline |
| Decision | Motion classification |
| Actuation | Added in later labs |

---

## Tools and Requirements
### Hardware
- Smartphone (Android or iOS)

### Software
- Edge Impulse account (free)
- Edge Impulse mobile app

---

## Step 1: Create an Edge Impulse Project
1. Go to https://studio.edgeimpulse.com
2. Create a new project
3. Select **Motion / Accelerometer data**
4. Name the project: `smartphone-motion-detection`

---

## Step 2: Install Edge Impulse Mobile App
- Install Edge Impulse from Google Play Store or Apple App Store
- Log in using the same Edge Impulse account

Your smartphone now acts as a wireless motion sensor.

---

## Step 3: Data Acquisition
Record labeled motion samples using the mobile app.

### Recommended Classes
- no_motion
- walking
- shake
- pickup

### Recording Guidelines
- 3–5 seconds per sample
- 10–20 samples per class
- Keep phone orientation consistent

---

## Step 4: ML Pipeline
Recommended pipeline:
Accelerometer → Spectral Features → Neural Network

---

## Step 5: Model Training
- Train the model
- Observe accuracy and confusion matrix
- Tune parameters if needed

---

## Step 6: Live Classification
- Use Live Classification
- Move the phone and observe predictions

---

## Expected Results
- ≥80% classification accuracy
- Stable real-time predictions

---

## Deliverables
- Screenshot of training results
- Screenshot of live classification
- Short report (1–2 pages)

---

## Extensions
- Deploy model to ESP32
- Motion-triggered actuator control
- MQTT streaming and dashboards

---

## License
Educational use only.
