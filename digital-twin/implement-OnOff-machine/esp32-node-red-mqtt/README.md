# 🧪 Laboratory
## Digital Twin for On–Off Machines using ESP32, Node-RED, and MQTT

---

## 📌 Introduction

A **Digital Twin** is a virtual representation of a physical system that enables
real-time monitoring, control, and intelligent decision-making.

This laboratory implements a **Digital Twin for an On–Off Machine**, where an
**ESP32 controls two LEDs**:
- **Primary LED (Main Machine)**
- **Backup LED (Secondary Machine)** activated automatically when the primary fails

The system integrates:
- ESP32 (Arduino framework)
- MQTT communication
- Node-RED Dashboard for visualization and control

This lab is suitable for **Cyber-Physical Systems (CPS), IoT, Smart Manufacturing,
and Predictive Maintenance** courses.

---

## 🎯 Learning Objectives

Students will be able to:

- Design an ESP32-based on–off controller
- Implement failure detection logic
- Use MQTT for real-time data exchange
- Build a Node-RED dashboard as a Digital Twin
- Synchronize physical and virtual machine states
- Demonstrate basic fault-tolerant CPS behavior

---

## 🧩 System Architecture

### Hardware Components
- ESP32 microcontroller
- Main LED (Primary Machine)
- Backup LED (Secondary Machine)
- Physical push button / switch
- Wi-Fi network

### Software Components
- Arduino IDE (ESP32 firmware)
- MQTT Broker (Mosquitto / HiveMQ)
- Node-RED
- Node-RED Dashboard

---

## ⚙️ Working Principle

1. ESP32 connects to Wi-Fi and MQTT broker  
2. Node-RED Dashboard provides remote ON/OFF control  
3. ESP32 monitors the main LED status  
4. If the main LED fails, the backup LED is activated automatically  
5. Physical switch allows manual control  
6. Digital Twin stays synchronized with the physical system  

---

## 🧪 ESP32 Firmware (Summary)

The ESP32 firmware performs:
- Wi-Fi and MQTT connection
- LED control via MQTT messages
- Failure detection of the primary LED
- Automatic activation of backup LED
- Publishing LED states to MQTT topics

---

## 🖥 Node-RED Dashboard

Node-RED provides:
- Real-time visualization of LED states
- Manual ON/OFF control
- Digital Twin representation of machines

### Basic Setup Steps
1. Install Node-RED  
2. Install MQTT broker  
3. Start Node-RED  
4. Open editor: http://localhost:1880  
5. Import provided MQTT flow  
6. Open dashboard: http://localhost:1880/ui  

---

## 📦 Deliverables

Students must submit:

1. ESP32 Arduino source code
2. Node-RED flow (JSON export)
3. Dashboard screenshots
4. Short explanation of the Digital Twin concept
5. Demonstration video or live demo (if required)

---

## 📝 Grading Criteria (Total: 100 Marks)

| Component | Description | Marks |
|---------|-------------|------:|
| ESP32 Control Logic | Correct LED control and switch handling | 15 |
| Failure Detection | Reliable detection and backup activation | 20 |
| MQTT Communication | Stable data exchange with Node-RED | 20 |
| Node-RED Dashboard | Clear visualization and control | 20 |
| Digital Twin Concept | Physical–virtual synchronization | 15 |
| Documentation & Presentation | Code quality, clarity, screenshots | 10 |
| **Total** |  | **100** |

---

## 🎓 Applications

- Smart Homes (backup lighting)
- Industrial automation
- Fault-tolerant CPS
- Predictive maintenance
- IoT monitoring systems

---

## ✅ Conclusion

This laboratory demonstrates a complete **Cyber-Physical System loop**:
**Physical Device → IoT → Digital Twin → Control Action**

Students gain hands-on experience with **ESP32, MQTT, Node-RED**, and
**Digital Twin concepts**, preparing them for modern CPS and Industry 4.0/5.0 applications.

---

**Keywords:** Digital Twin, ESP32, Node-RED, MQTT, CPS, IoT, Failure Detection
