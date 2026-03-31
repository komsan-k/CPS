# Final Course Project Instruction 10% 
# Smart Traffic Light Controller Using ESP32 + TFLM + Node-RED Digital-Twin

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
