# Big Idea -- Smart Traffic Light Controller Using TensorFlow Lite for Microcontrollers (TFLM)

## 1. Overview
This project presents a **smart traffic light controller for a single intersection** using **TensorFlow Lite for Microcontrollers (TFLM)** running on an **ESP32**.

The system is designed for:
- **East–West (EW) major road**
- **North–South (NS) minor road**
- **2 traffic sensors per road**
- **4 sensors total**

The objective is to replace fixed-time traffic control with an **adaptive Edge AI controller**.

## 2. System Concept
Traditional traffic lights often use fixed timing such as:
- EW green = 30 s
- NS green = 10 s

This project uses a tiny neural network model to decide whether to keep green, switch phase, or extend green time.

## 3. Input Features
`x = [ew1, ew2, ns1, ns2, ew_queue, ns_queue, phase, green_time]`

## 4. Output Classes
- Class 0 = Keep EW green
- Class 1 = Switch to NS
- Class 2 = Keep NS green
- Class 3 = Switch to EW

## 5. Recommended Tiny Neural Network
`8 -> 8 -> 4`

## 6. Safety Logic
Always enforce:
- minimum green time
- maximum green time
- yellow clearance
- all-red interval
- watchdog fallback

## 7. CPS Architecture
Sensors -> Feature Extraction -> TFLM on ESP32 -> Safety Logic -> Traffic Lights

## 8. Summary
This project demonstrates an **Edge AI smart traffic controller** suitable for CPS labs, TinyML experiments, and smart city research.
