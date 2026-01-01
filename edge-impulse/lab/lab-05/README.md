# Lab 5 – Anomaly Detection for CPS Monitoring (C4: Cognition)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand **anomaly detection** in Cyber-Physical Systems (CPS)
- Apply **unsupervised learning** for CPS condition monitoring
- Design an anomaly detection pipeline using **Edge Impulse**
- Deploy an anomaly detection model on an **ESP32 edge device**
- Interpret anomaly scores for CPS health monitoring

---

## 🧠 Background Theory

### 1. Anomaly Detection in CPS

In CPS, anomalies represent **unexpected or abnormal system behavior** that may indicate:
- Faults or failures
- Degradation of components
- Unsafe operating conditions

Unlike classification, anomaly detection often relies on **normal-operation data only**.

---

### 2. Cognition Layer and Health Monitoring

Within the **Cognition (C4)** layer, anomaly detection enables:
- System health awareness
- Early fault detection
- Predictive maintenance

Anomaly scores provide continuous insight rather than discrete labels.

---

### 3. Unsupervised Learning for CPS

Unsupervised models:
- Learn patterns of normal behavior
- Flag deviations as anomalies
- Are suitable when fault data is scarce

Edge Impulse provides lightweight anomaly detection blocks optimized for embedded devices.

---

### 4. Anomaly Scores and Thresholds

The anomaly detection model outputs:
- A numerical **anomaly score**
- Higher scores indicate deviation from normal behavior

Thresholds are used to trigger CPS actions such as alerts or shutdowns.

---

## 🧱 CPS Focus of This Lab

- **C4 – Cognition:** Primary focus  
- C1 – Connection: Sensor data  
- C2 – Conversion: Feature extraction  
- C3 & C5: Not yet implemented  

---

## ⚙️ Edge Impulse Focus

- Anomaly detection block
- Training with normal data
- Threshold configuration
- On-device anomaly scoring

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor used in previous labs (IMU, vibration, temperature)
- USB cable

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse CLI
- Edge Impulse Studio

---

## 🔧 Experimental Procedure

### Step 1: Prepare Normal Dataset
1. Open the Edge Impulse project.
2. Navigate to **Data Acquisition**.
3. Select only *normal* operation samples.
4. Ensure dataset consistency.

---

### Step 2: Configure Anomaly Detection Impulse
1. Go to **Create Impulse**.
2. Select DSP block from Lab 3.
3. Choose **Anomaly Detection** as the learning block.
4. Save the impulse.

---

### Step 3: Train Anomaly Detection Model
1. Navigate to **Anomaly** tab.
2. Start training using normal data.
3. Observe reconstruction or distance metrics.

---

### Step 4: Evaluate Anomaly Scores
1. Use test data.
2. Observe anomaly score distribution.
3. Determine suitable threshold values.

---

### Step 5: Deploy Model to ESP32
1. Build firmware for ESP32.
2. Upload to device.
3. Monitor anomaly scores via Serial Monitor.

---

## 📊 Results and Observations

Students should obtain:
- A trained anomaly detection model
- Real-time anomaly scores on ESP32
- Clear separation between normal and abnormal behavior

---

## 📝 Discussion Questions

1. Why is anomaly detection important in CPS?
2. How does it differ from classification?
3. How should anomaly thresholds be chosen?
4. What CPS actions can be triggered by anomalies?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain anomaly detection concepts in CPS
- Train unsupervised models using Edge Impulse
- Deploy anomaly detection models to ESP32
- Interpret CPS health indicators

---

## 🧾 Conclusion

This lab implemented anomaly-based cognition for CPS monitoring using Edge Impulse. The CPS can now detect abnormal behavior and assess system health in real time.

---

## 📚 References

1. Edge Impulse Documentation  
2. Predictive Maintenance Literature  
3. CPS Architecture References  

---

### ▶️ Next Lab

**Lab 6 – On-Device Inference and CPS Decision Logic**
