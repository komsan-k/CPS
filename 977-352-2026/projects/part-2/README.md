# Final Course Project Instruction 20% 

# Smart Traffic Light Controller Using ESP32 + TFLM + Node-RED Digital-Twin

---
---

# Final Course Project Instruction (10%)

The **Final Course Project** contributes **10% of the total course grade**.

Students are required to complete the **Smart Traffic Light Controller Using ESP32 + TFLM + Node-RED** as the final project.

The project will be assessed in **two submission stages**.

---

## 1. Progress Report — 8 April
**Submission Deadline:** **8 April**

### Required Contents
- Project title and team information
- Project overview
- System architecture diagram
- Work completed
- Current results
- Problems / challenges

### Progress Report Marks (4%)
| Item | Marks |
|---|---:|
| project progress | 1 |
| architecture design | 1 |
| preliminary implementation | 1 |
| clarity of report | 1 |
| **Total** | **4** |

---

## 2. Final Report and Demonstration — 20 April
**Submission Deadline:** **20 April**

### Required Final Submission
- Final technical report
- Complete source code
- Node-RED flow
- Trained model
- Digital twin
- Demonstration video / live demo

### Final Report Marks (6%)
| Item | Marks |
|---|---:|
| final implementation | 2 |
| TinyML model | 1 |
| dashboard + digital twin | 1 |
| report quality | 1 |
| final presentation/demo | 1 |
| **Total** | **6** |

---

## Total Final Course Project Marks
| Stage | Marks |
|---|---:|
| Progress Report (8 Apr) | 4 |
| Final Report (20 Apr) | 6 |
| **Total** | **10** |

---
---

## 1. Project Overview
Design and implement a **Cyber-Physical System (CPS)-based intelligent traffic light controller** using:
- ESP32
- TensorFlow Lite for Microcontrollers (TFLM)
- Node-RED Dashboard
- MQTT communication
- 4 traffic sensors
- Edge AI decision logic

## 2. Learning Objectives
Students should be able to:
- design CPS architecture
- interface sensors with ESP32
- train and deploy TinyML models
- build Node-RED dashboards
- evaluate real-time traffic control performance

## 3. System Requirements
```text
4 Traffic Sensors
    ↓
ESP32 Sensor Interface
    ↓
Feature Extraction
    ↓
TFLM Inference
    ↓
Safety Logic
    ↓
Traffic Light Control
    ↓
MQTT + Node-RED Dashboard
```

## 4. Deliverables
- Technical report (PDF)
- `ESP32_TFLM_traffic.ino`
- `train_model.py`
- `model_data.h`
- Node-RED flow JSON
- Architecture diagram
- 5–10 minute demo 

## 5. Grading Rubric
| Component | Marks |
|---|---:|
| Hardware implementation | 2 |
| ESP32 + TFLM code | 2 |
| Node-RED dashboard |3 |
| AI model training | 3 |
| Testing & evaluation | 4 |
| Report quality | 3 |
| Final presentation | 3 |
| **Total** | **20** |

## 6. Addition
Pedestrian crossing, emergency priority, digital twin
