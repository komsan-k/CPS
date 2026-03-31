# Final Course Project Instruction 10% 
# Smart Traffic Light Controller Using ESP32 + TFLM + Node-RED

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
- PNG architecture diagram
- 5–10 minute demo video

## 5. Grading Rubric
| Component | Marks |
|---|---:|
| Hardware implementation | 1 |
| ESP32 + TFLM code | 1 |
| Node-RED dashboard | 2 |
| AI model training | 2 |
| Testing & evaluation | 1 |
| Report quality | 1 |
| Final presentation | 2 |
| **Total** | **10** |

## 6. Bonus
Optional: pedestrian crossing, emergency priority, cloud logging, digital twin
