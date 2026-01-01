# Lab 4 – Classification Models for CPS Cognition (C4)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand the **Cognition (C4) layer** of the 5C Cyber-Physical System (CPS) model
- Design and train **classification models** for CPS perception
- Evaluate classification performance using Edge Impulse tools
- Deploy trained models to an **ESP32 edge device**
- Perform real-time on-device inference

---

## 🧠 Background Theory

### 1. Cognition Layer in CPS (C4)

The **Cognition layer (C4)** is responsible for transforming information into **knowledge and decisions**.
In CPS, cognition enables the system to:
- Recognize system states
- Classify events or behaviors
- Detect abnormal conditions
- Support decision-making

Machine learning models are central to implementing cognition in modern CPS.

---

### 2. Classification in Cyber-Physical Systems

Classification assigns sensor data to predefined categories.
Common CPS classification tasks include:
- Human activity recognition
- Machine operating state detection
- Gesture recognition
- Event detection

Classification outputs are typically probability scores that represent system confidence.

---

### 3. Edge Impulse Classification Models

Edge Impulse supports several embedded-friendly classifiers:
- Neural network classifiers
- Lightweight dense models
- DSP + NN hybrid pipelines

These models are optimized for:
- Low latency
- Low memory usage
- Deterministic execution on microcontrollers

---

### 4. Performance Metrics for CPS Cognition

Key metrics include:
- Accuracy
- Precision and recall
- Confusion matrix
- Latency and memory footprint

In CPS, model reliability is as important as raw accuracy.

---

## 🧱 CPS Focus of This Lab

- **C4 – Cognition:** Primary focus  
- C1 – Connection: Data source  
- C2 – Conversion: Feature extraction  
- C3 & C5: Not yet implemented  

---

## ⚙️ Edge Impulse Focus

- Neural network model design
- Training and validation
- Performance evaluation
- Model deployment to ESP32

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor used in Labs 2–3
- USB cable

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse CLI
- Edge Impulse Studio

---

## 🔧 Experimental Procedure

### Step 1: Verify Feature Set
1. Open the Edge Impulse project.
2. Navigate to **Impulse Design**.
3. Confirm DSP block from Lab 3 is configured.
4. Regenerate features if necessary.

---

### Step 2: Configure Classification Model
1. Go to **Classifier** tab.
2. Select neural network classifier.
3. Configure model parameters:
   - Number of layers
   - Neurons per layer
   - Learning rate
4. Start training.

---

### Step 3: Model Evaluation
1. Observe training and validation accuracy.
2. Examine the confusion matrix.
3. Identify misclassified classes.

---

### Step 4: Model Optimization
1. Enable quantization if available.
2. Review RAM and Flash usage.
3. Adjust model size if required.

---

### Step 5: Deploy Model to ESP32
1. Build firmware for ESP32.
2. Upload firmware using Arduino IDE.
3. Open Serial Monitor.
4. Observe real-time inference results.

---

## 📊 Results and Observations

Students should obtain:
- A trained classification model
- Accuracy and confusion matrix results
- Successful on-device inference
- Real-time classification outputs

---

## 📝 Discussion Questions

1. Why is cognition essential in CPS?
2. How does classification accuracy affect CPS decisions?
3. What trade-offs exist between model size and accuracy?
4. Why is on-device inference preferred in CPS?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain the role of the Cognition layer
- Train classification models using Edge Impulse
- Evaluate ML model performance
- Deploy ML models to ESP32
- Interpret real-time CPS inference results

---

## 🧾 Conclusion

This lab implemented the Cognition layer of a CPS by training and deploying classification models using Edge Impulse on ESP32. The CPS can now perceive and classify real-world states in real time.

---

## 📚 References

1. Edge Impulse Documentation  
2. Embedded Machine Learning Literature  
3. CPS Architecture References  

---

### ▶️ Next Lab

**Lab 5 – Anomaly Detection for CPS Monitoring**
