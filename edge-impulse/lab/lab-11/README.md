# Lab 11 – Adaptive CPS: Model Update and Online Learning (Lifecycle & Adaptation)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand **adaptation and learning** in Cyber-Physical Systems (CPS)
- Manage the **model lifecycle** for edge intelligence
- Perform **dataset updates, retraining, and redeployment** using Edge Impulse
- Implement **over-the-air (OTA) model updates** on ESP32
- Evaluate CPS performance before and after adaptation

---

## 🧠 Background Theory

### 1. Adaptation in CPS

Adaptive CPS can modify their behavior in response to:
- Environmental changes
- System aging and drift
- New operating conditions

Adaptation improves robustness, reliability, and long-term performance.

---

### 2. Model Lifecycle in Edge Intelligence

An edge ML lifecycle includes:
1. Data collection
2. Model training
3. Deployment
4. Monitoring
5. Update and redeployment

Managing this lifecycle is essential for real-world CPS.

---

### 3. Concept Drift and Dataset Evolution

**Concept drift** occurs when data distributions change over time.
Symptoms include:
- Reduced accuracy
- Increased anomaly scores
- Misclassification

Updating datasets and retraining models mitigates drift.

---

### 4. OTA Updates in CPS

OTA updates enable:
- Remote model updates
- Reduced maintenance cost
- Scalable CPS deployment

In this lab, OTA is demonstrated at a conceptual and practical level.

---

## 🧱 CPS Focus of This Lab

- **C5 – Configuration:** System adaptation  
- Cross-layer lifecycle management (C1–C4)

---

## ⚙️ Edge Impulse Focus

- Dataset augmentation
- Model retraining
- Version comparison
- Redeployment to ESP32

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor and actuator from previous labs
- USB cable
- Wi-Fi network

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse Studio
- Edge Impulse inference library
- OTA update support (basic)

---

## 🔧 Experimental Procedure

### Step 1: Collect New Data
1. Identify new operating conditions.
2. Acquire additional sensor samples.
3. Label and upload data to Edge Impulse.

---

### Step 2: Retrain the Model
1. Retrain classification or anomaly model.
2. Compare metrics with previous version.
3. Select improved model.

---

### Step 3: Redeploy to ESP32
1. Export updated model.
2. Upload firmware to ESP32.
3. Verify successful update.

---

### Step 4: Evaluate Adaptation
1. Run inference with updated model.
2. Compare performance before and after update.
3. Observe reduction in errors or anomalies.

---

## 📊 Results and Observations

Students should observe:
- Improved accuracy or stability
- Reduced anomaly scores under new conditions
- Successful model version transition

---

## 📝 Discussion Questions

1. Why is adaptation essential for long-term CPS deployment?
2. How does concept drift affect CPS reliability?
3. What are the risks of OTA updates?
4. How frequently should CPS models be updated?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain CPS adaptation and lifecycle concepts
- Update datasets and retrain models
- Redeploy ML models to ESP32
- Evaluate adaptive CPS performance

---

## 🧾 Conclusion

This lab demonstrated **adaptive CPS behavior** through model update and lifecycle management using Edge Impulse. The CPS can now evolve with changing environments and requirements.

---

## 📚 References

1. Edge Impulse Documentation  
2. Adaptive CPS Literature  
3. OTA Update Best Practices  

---

### ▶️ Next Lab

**Lab 12 – Capstone Mini-Project: End-to-End CPS with Edge Intelligence**
