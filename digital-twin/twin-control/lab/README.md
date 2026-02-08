# Lab: RSSI-Based Twin-Control Beam Steering with ESP32 

## 1. Objective
In this lab, you will implement a Twin-Control beam steering system using two ESP32 boards.
One ESP32 controls a reconfigurable antenna (beam states), while the other ESP32 measures RSSI
and feeds back the best beam configuration.

By the end of this lab, you will understand how RSSI sensing closes the control loop
between the physical antenna and its digital twin.

---

## 2. Learning Outcomes
- Understand Twin-Control in antenna systems
- Use ESP32 Wi-Fi RSSI as a sensing mechanism
- Implement closed-loop beam selection
- Observe adaptive beam steering behavior

---

## 3. System Overview
ESP32-TX (Beam Controller)  ---> Wi-Fi --->  ESP32-RX (RSSI Sensor)
       |                                        |
       |-- Beam GPIO control                   |-- RSSI measurement
       <------------ Best Beam ID --------------

---

## 4. Hardware Requirements
- 2 × ESP32 development boards
- Reconfigurable antenna or LED array (beam-state emulator)
- Jumper wires
- USB cables

---

## 5. Software Requirements
- Arduino IDE
- ESP32 board support package

---

## 6. Experiment Steps
1. Upload TX code to ESP32-TX
2. Upload RX code to ESP32-RX
3. Power both boards
4. Observe RSSI values in Serial Monitor
5. Identify which beam ID is selected
6. Move the RX board and observe beam re-selection

---

## 7. Questions for Students
1. Why is RSSI noisy?
2. Why is averaging or median filtering needed?
3. How does this system differ from open-loop beam steering?
4. What happens if the environment changes?

---

## 8. Deliverables
- Screenshots of RSSI logs
- Short explanation of beam selection behavior
- Discussion on Twin-Control advantages
