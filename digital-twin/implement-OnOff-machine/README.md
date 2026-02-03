# 🧪 CPS / IoT Laboratory
## Digital Twin for an On–Off Machine (ESP32 + Node-RED + MQTT)
### **Grading Scheme: 100 Marks**

---

## 📌 Lab Overview

This laboratory guides students through the **design and implementation of a Digital Twin** for a simple **on–off machine controlling an LED load**.  
The system evolves through **five phases**, from manual control to **AI-driven predictive maintenance**, using **ESP32, LDR sensors, MQTT, and Node-RED**.

The lab emphasizes **Cyber-Physical Systems (CPS)** principles: perception, communication, visualization, cognition, and closed-loop control.

---

## 🎯 Learning Objectives

By completing this lab, students will be able to:

- Design an interrupt-based on–off controller using ESP32
- Implement perception using LDR sensors for fault detection
- Integrate IoT communication with MQTT and Node-RED
- Build a virtual machine (Digital Twin) with real-time visualization
- Apply basic AI/logic for failure prediction and backup activation
- Demonstrate a complete CPS feedback loop

---

## 🧩 System Architecture (Conceptual)

```
ESP32 (LED + Button + LDR)
        ↓ MQTT
     Node-RED
  (Dashboard + Logic)
        ↓
 Digital Twin
 (Virtual Machine)
        ↓
 Prediction / Backup
```

---

## 🔬 Lab Phases

### Phase 1: Manual On–Off Machine (ESP32 Interrupt)
- Button-triggered interrupt toggles LED
- Fast and deterministic response
- Foundation of the CPS

---

### Phase 2: LDR-Based Perception
- LDR measures LED brightness
- Detects abnormal conditions (LED ON but low brightness)
- Introduces perception and fault awareness

---

### Phase 3: IoT Integration (MQTT + Node-RED)
- ESP32 publishes LED and LDR data via MQTT
- Node-RED subscribes and visualizes data
- Remote on/off control enabled

---

### Phase 4: Virtual Machine & Visualization
- Digital Twin created using Node-RED Dashboard
- Live charts and indicators
- Health monitoring of the physical system

---

### Phase 5: Cognition & Predictive Control
- Rule-based or ML-based failure prediction
- Automatic backup activation when failure is detected
- Alerts to users (dashboard / log)

---

## 📊 Deliverables

Students must submit:

1. ESP32 source code (interrupt, LDR, MQTT)
2. Node-RED flow (JSON export)
3. Dashboard screenshots
4. Explanation of Digital Twin architecture
5. Short report (PDF or Markdown)

---

## 📝 Grading Criteria (Total: **100 Marks**)

| Component | Description | Marks |
|---------|-------------|------:|
| **Phase 1: Manual On–Off Machine** | Correct interrupt-based LED control, clean code, demonstration | 15 |
| **Phase 2: LDR-Based Perception** | Proper sensor integration, threshold logic, fault detection | 15 |
| **Phase 3: IoT Integration** | MQTT communication, Node-RED flow correctness, real-time data | 20 |
| **Phase 4: Virtual Machine & Visualization** | Digital Twin dashboard, clarity, usability, real-time sync | 20 |
| **Phase 5: Cognition & Predictive Control** | Prediction logic, backup activation, system autonomy | 20 |
| **Documentation & Presentation** | Code readability, explanation, screenshots, report quality | 10 |
| **Total** |  | **100** |

---

## 🎓 Evaluation Notes (Instructor)

- Partial marks should be awarded for partially working systems
- Bonus insight may be given for:
  - Clean CPS architecture explanation
  - Innovative visualization
  - Clear separation of physical vs digital logic
- AI component may be **rule-based** for undergraduate level

---

## ✅ Completion Checklist

- [ ] ESP32 interrupt works reliably  
- [ ] LDR detects LED anomalies  
- [ ] MQTT data visible in Node-RED  
- [ ] Dashboard updates in real time  
- [ ] Digital Twin reflects physical state  
- [ ] Failure triggers backup logic  

---

## 📚 Keywords

Digital Twin · Cyber-Physical Systems · ESP32 · MQTT · Node-RED · Predictive Maintenance · IoT

---

**This lab demonstrates a complete CPS loop:**
> *Physical → Cyber → Cognition → Control*

