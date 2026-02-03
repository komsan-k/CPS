# 🧪 IoT : Controling LED
## Digital Twin for On–Off LED Control using ESP32 and Node-RED

---

## 📌 Overview

This baseline demonstrates how to implement a **Digital Twin** for a simple **on–off LED control system** using **ESP32** and **Node-RED**.

A Digital Twin is a **virtual replica of a physical system** that remains synchronized with the real system in real time. In this lab, the physical LED connected to an ESP32 is mirrored by a **Node-RED Dashboard**, enabling remote monitoring and control.

This experiment serves as a foundation for more advanced **IoT, CPS, and Digital Twin applications**.

---

## 🎯 Objectives

By completing this lab, students will be able to:

- Explain the concept of a Digital Twin
- Control a physical LED using ESP32
- Use MQTT for IoT communication
- Design a Node-RED dashboard for remote control
- Synchronize physical and digital system states

---

## 🧩 System Components

### Hardware
- **ESP32 Microcontroller**
- **LED** (with 220 Ω resistor)
- **Breadboard and jumper wires**
- **Wi-Fi network**

### Software
- **Arduino IDE**
- **Node-RED**
- **Node-RED Dashboard**
- **MQTT Broker** (Mosquitto / HiveMQ)

---

## ⚙️ System Architecture

```
Node-RED Dashboard
        ↓ MQTT
      ESP32
        ↓
       LED
```

---

## 🔌 Hardware Setup

1. Connect LED **anode (+)** to **ESP32 GPIO 2**
2. Connect LED **cathode (–)** to **GND through 220 Ω resistor**
3. Power ESP32 via USB

---

## 💻 ESP32 Programming (Arduino)

### Required Libraries
- `WiFi.h`
- `PubSubClient.h`

### Functional Description
- ESP32 connects to Wi-Fi
- Subscribes to MQTT topic `led/control`
- Receives `"ON"` / `"OFF"` commands
- Controls LED accordingly

---

## 🖥 Node-RED Digital Twin

### Dashboard Features
- ON button → turns LED ON
- OFF button → turns LED OFF
- Text indicator → shows LED state (optional)

### Node-RED Steps
1. Install Node-RED and Dashboard
2. Add **MQTT Out** node → topic `led/control`
3. Add **UI Button** nodes for ON / OFF
4. Deploy the flow
5. Open dashboard:  
   `http://localhost:1880/ui`

---

## 📦 Deliverables

Students must submit:

1. ESP32 Arduino code
2. Node-RED flow (JSON export)
3. Screenshot of dashboard
4. Short explanation of Digital Twin concept

---


## 📈 Results

The Digital Twin successfully:
- Controls the physical LED remotely
- Reflects system state in real time
- Demonstrates CPS and IoT integration

---

## 🧠 Conclusion

This lab provides hands-on experience with **Digital Twins**, **ESP32**, and **Node-RED**.  
It forms a strong basis for extending into:
- Multiple actuators
- Sensors and perception
- Predictive maintenance
- Smart CPS applications




**Keywords:** Digital Twin, ESP32, Node-RED, MQTT, CPS, IoT

