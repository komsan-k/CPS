# 🧪 Midterm Course Project --- Digital Twin + TinyML CPS

## Digital Twin for an On--Off Machine with TinyML Extension

**Course:** 977-352 Cyber-Physical Systems\
**Project Type:** Midterm Implementation\
**Last Updated:** 2026-02-11

https://github.com/komsan-k/CPS/tree/main/tinyML/lab
------------------------------------------------------------------------

# 📌 Project Overview

This midterm project extends the **Digital Twin On--Off Machine** into a
**TinyML-enabled Cyber‑Physical System (CPS)**.

Students will integrate:

-   ESP32 physical machine (LED + Button + LDR + MQTT)
-   Node‑RED Digital Twin dashboard
-   TinyML data pipeline (dataset → feature → model → inference)

The project follows the CPS 5C concept:

Physical → Connection → Conversion → Cyber → Cognition → Configuration

------------------------------------------------------------------------

# 🎯 Learning Outcomes

Students will be able to:

-   Implement an ESP32 Digital Twin architecture
-   Collect analog sensor datasets for TinyML
-   Design feature extraction on embedded devices
-   Deploy a lightweight ML model on ESP32
-   Integrate CPS feedback with AI-assisted decision logic

------------------------------------------------------------------------

# 🧩 System Architecture

ESP32 (LED + LDR + Button)\
↓ MQTT\
Node‑RED Digital Twin\
↓\
TinyML Model\
↓\
Predictive Control / Backup Logic

------------------------------------------------------------------------

# 🔬 Midterm Implementation Phases

## Phase A --- Digital Twin Core (DT Implementation)

-   Interrupt-based LED On--Off machine
-   LDR perception for fault detection
-   MQTT publishing and Node‑RED visualization

Deliverable: - Working Digital Twin dashboard

------------------------------------------------------------------------

## Phase B --- Dataset Creation (TinyML Lab 2)

Goal: Build datasets from analog sensors.

Tasks:

-   Read LDR ADC values
-   Apply moving average filtering
-   Log CSV-style serial output

Deliverables:

-   Dataset file
-   ADC scaling explanation

------------------------------------------------------------------------

## Phase C --- Feature Engineering (TinyML Lab 3)

Goal: Convert signals into features.

Required features:

-   Mean
-   Variance
-   RMS
-   Min / Max
-   Slope

Deliverables:

-   Feature extraction function
-   Feature table

------------------------------------------------------------------------

## Phase D --- Tiny Classifier Deployment (TinyML Lab 4)

Goal: Train and deploy a small model.

Example Classes:

-   Bright
-   Normal
-   Dark

Deliverables:

-   Trained model
-   Exported C-array
-   ESP32 inference controlling LED

------------------------------------------------------------------------

## Phase E --- Optimization (TinyML Lab 5)

Goal: Improve performance.

Tasks:

-   Convert model to INT8
-   Compare latency and memory

Deliverables:

-   Float vs INT8 comparison table

------------------------------------------------------------------------

## Phase F --- Streaming CPS Monitoring (TinyML Lab 6)

Goal: Sliding-window inference.

Tasks:

-   Implement buffer window
-   Real-time predictions

Deliverables:

-   Streaming inference demo

------------------------------------------------------------------------

# 📊 Grading Scheme --- Midterm (100 Marks)

  Component                Description                        Marks
  ------------------------ ------------------------------ ---------
  Digital Twin Core        Interrupt + MQTT + Dashboard          20
  Dataset Creation         Logging + Filtering                   15
  Feature Engineering      Embedded features                     15
  TinyML Deployment        Model + Inference                     20
  Optimization             Quantization comparison               10
  Streaming Monitoring     Windowed inference                    10
  Documentation & Report   README + Explanation                  10
  **Total**                                                 **100**

------------------------------------------------------------------------

# 📝 Report Rubric (10%)

Students must include:

-   System architecture diagram
-   CPS loop explanation
-   Dataset description
-   Model summary
-   Results table (accuracy / latency / memory)
-   Screenshots of dashboard

------------------------------------------------------------------------

# 📄 Report Template (Student Submission)

## 1. Introduction

Explain the Digital Twin + TinyML concept.

## 2. System Design

Hardware + software architecture.

## 3. Dataset & Features

How data was collected and processed.

## 4. Model Training

Model type, classes, evaluation.

## 5. Deployment Results

Inference behavior and control outcome.

## 6. Discussion

Challenges and improvements.

## 7. Conclusion

------------------------------------------------------------------------

# ✅ Midterm Checklist

-   [ ] ESP32 Digital Twin works
-   [ ] Dataset collected
-   [ ] Feature extractor implemented
-   [ ] TinyML inference running
-   [ ] Dashboard synchronized
-   [ ] Report completed

------------------------------------------------------------------------

# 📚 Keywords

Digital Twin · CPS · TinyML · ESP32 · MQTT · Node‑RED · Edge AI
