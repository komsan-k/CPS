# Lab 12 – Capstone Mini-Project: End-to-End CPS with Edge Intelligence

## 🎯 Objective

The objectives of this capstone laboratory are to:

- Integrate **all CPS layers (C1–C5)** into a single end-to-end system
- Design and implement a **complete CPS** using ESP32 and Edge Impulse
- Demonstrate sensing, conversion, cognition, cyber integration, and configuration
- Validate system performance, robustness, and adaptability
- Present and document a **real-world CPS application**

---

## 🧠 Background Theory

### 1. End-to-End CPS Integration

A complete Cyber-Physical System integrates:
- **C1 – Connection:** sensing and data acquisition
- **C2 – Conversion:** signal processing and feature extraction
- **C3 – Cyber:** communication and system-level coordination
- **C4 – Cognition:** machine learning–based perception
- **C5 – Configuration:** control, adaptation, and optimization

This capstone lab synthesizes all previous labs into a unified CPS.

---

### 2. Role of Edge Intelligence in CPS

Edge intelligence enables:
- Real-time decision-making
- Reduced latency and bandwidth usage
- Privacy-preserving analytics
- Resilient operation during network disruptions

**Edge Impulse** serves as the embedded intelligence platform throughout the CPS lifecycle.

---

## 🧱 CPS Focus of This Lab

- **C1–C5:** Full-stack CPS integration  
- Lifecycle management and system validation

---

## ⚙️ Edge Impulse Focus

- End-to-end pipeline usage
- Model selection (classification or anomaly detection)
- Performance optimization
- Versioning and deployment

---

## 🧰 Hardware Requirements

- ESP32 development board
- At least one sensor (IMU, temperature, vibration, microphone)
- At least one actuator (LED, relay, motor, servo)
- USB cable
- Wi-Fi network

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse Studio
- Edge Impulse inference library
- MQTT broker (optional)
- Dashboard tool (optional)

---

## 🧪 Project Options (Choose One)

Students may choose one of the following CPS scenarios or propose their own:

1. **Predictive Maintenance CPS**  
   - Sensor: vibration  
   - Intelligence: anomaly detection  
   - Action: alarm or shutdown  

2. **Human Activity Recognition CPS**  
   - Sensor: IMU  
   - Intelligence: classification  
   - Action: mode switching  

3. **Smart Environment CPS**  
   - Sensor: temperature/light  
   - Intelligence: trend detection  
   - Action: relay or fan control  

---

## 🔧 System Architecture

Students must design a CPS architecture diagram showing:
- Sensor interfaces
- Edge Impulse pipeline
- Decision logic
- Cyber connectivity (if used)
- Actuation and feedback

---

## 🧪 Experimental Procedure

### Step 1: CPS Design
1. Select CPS application scenario.
2. Identify sensors, actuators, and CPS objectives.
3. Map design to 5C CPS layers.

---

### Step 2: Implementation
1. Interface sensors with ESP32.
2. Configure Edge Impulse pipeline.
3. Train and deploy ML model.
4. Implement decision and control logic.

---

### Step 3: Cyber Integration (Optional)
1. Publish CPS data via MQTT.
2. Visualize system state on dashboard.

---

### Step 4: Validation and Testing
1. Test under normal conditions.
2. Introduce disturbances or anomalies.
3. Evaluate CPS response and robustness.

---

## 📊 Evaluation Metrics

Students should evaluate:
- ML accuracy or anomaly score behavior
- Inference latency
- Resource usage (RAM, Flash)
- System responsiveness
- Energy considerations

---

## 📝 Report & Presentation Requirements

Each team must submit:
- CPS architecture diagram
- Description of each CPS layer
- Experimental results and analysis
- Discussion of limitations and improvements
- Live or recorded demonstration

---

## 🎓 Learning Outcomes

After completing this capstone lab, students will be able to:
- Design and implement a full CPS
- Integrate edge intelligence into real systems
- Analyze CPS performance holistically
- Communicate CPS designs and results professionally

---

## 🧾 Conclusion

This capstone laboratory demonstrated the design and realization of a **complete Cyber-Physical System with edge intelligence**. It consolidates all CPS concepts and skills developed throughout the course.

---

## 📚 References

1. Edge Impulse Documentation  
2. CPS Architecture Literature  
3. TinyML and Edge AI Resources  

---

### 🎉 End of CPS Laboratory Series
