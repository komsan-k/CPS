# Lab 4: Train and Deploy a TinyML Classifier on ESP32  
## From Features to On-Device Intelligence  
### CPS Focus: C4 — Cognition

---

## 1. Lab Overview

This laboratory completes the **end-to-end TinyML pipeline** for the first time.

Students will use the **feature vectors generated in Lab 3** to:
- Train a lightweight classifier on a PC  
- Convert the trained model into **C/C++ code**  
- Deploy the model on an **ESP32** for real-time, on-device inference  

Lab 4 corresponds to **CPS Level C4 (Cognition)**, where learned models make intelligent decisions based on processed sensor data.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Train a small ML classifier using feature data  
- Evaluate model accuracy and confusion matrix  
- Convert a trained model into embedded **C/C++ form**  
- Integrate the model with ESP32 firmware  
- Perform real-time inference and control an actuator  

---

## 3. Background Theory

### 3.1 Why Feature-Based TinyML Models?

On microcontrollers:

- Raw signals are expensive to process  
- Feature vectors are compact and informative  
- Simple models often outperform complex ones  

**TinyML-friendly models include:**
- Logistic regression  
- Small multilayer perceptron (MLP)  
- Decision tree  

In this lab, a **simple multi-class classifier** is used to balance accuracy and efficiency.

---

### 3.2 CPS Perspective (C4: Cognition)

| CPS Layer | Role |
|---------|------|
| Physical | Sensor signals |
| Conversion | ADC |
| Cyber | Feature extraction |
| Cognition | TinyML classifier |
| Actuation | LED / indicator |

---

## 4. Dataset Preparation (PC Side)

### 4.1 Dataset Structure

From **Lab 3**, each feature vector consists of:

```
mean, min, max, variance, rms, slope, label
```

**Example labels (LDR-based):**
- `0` → Dark  
- `1` → Normal  
- `2` → Bright  

---

### 4.2 Training (Conceptual)

Students may use **Python (scikit-learn)** or **Edge Impulse**.

Typical training steps:
1. Load CSV dataset  
2. Normalize features  
3. Train classifier  
4. Evaluate accuracy and confusion matrix  

**Target accuracy:** ≥ 85% (demonstration level)

---

## 5. Model Conversion for ESP32

After training, model parameters are converted into **C arrays**.

### 5.1 Example Model Header

```cpp
// your_model_inferencing.h

#ifndef TINYML_MODEL_H
#define TINYML_MODEL_H

#define INPUT_SIZE 6
#define NUM_CLASSES 3

extern const float weights[NUM_CLASSES][INPUT_SIZE];
extern const float bias[NUM_CLASSES];

int tinyml_predict(float *features);

#endif
```

---

### 5.2 Inference Function

```cpp
int tinyml_predict(float *x) {
  float scores[NUM_CLASSES];

  for (int i = 0; i < NUM_CLASSES; i++) {
    scores[i] = bias[i];
    for (int j = 0; j < INPUT_SIZE; j++) {
      scores[i] += weights[i][j] * x[j];
    }
  }

  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (scores[i] > scores[best]) best = i;

  return best;
}
```

This implements a **lightweight linear classifier** suitable for ESP32.

---

## 6. ESP32 System Architecture

```
[Analog Sensor]
       ↓
     ADC
       ↓
 Feature Extraction
       ↓
 TinyML Classifier
       ↓
  Prediction Label
       ↓
    Actuator (LED)
```

This forms a **complete TinyML CPS loop**.

---

## 7. Lab Tasks

### Task 1: Integrate Feature Vector

```cpp
float features[6];
features[0] = mean;
features[1] = minVal;
features[2] = maxVal;
features[3] = variance;
features[4] = rms;
features[5] = slope;
```

---

### Task 2: Run Inference

```cpp
int prediction = tinyml_predict(features);
```

---

### Task 3: Actuator Control Based on Prediction

```cpp
void controlOutput(int label) {
  if (label == 0) digitalWrite(LED_PIN, LOW);       // Dark
  else if (label == 1) digitalWrite(LED_PIN, HIGH); // Normal
  else digitalWrite(LED_PIN, HIGH);                  // Bright (blink optional)
}
```

---

### Task 4: Serial Monitoring

```cpp
Serial.print("Prediction: ");
Serial.println(prediction);
```

Students observe **live classification** as lighting conditions change.

---

## 8. Observations and Discussion

Students should evaluate:
- Accuracy vs real environment conditions  
- Prediction stability  
- Delay from sensing → decision  

**Key Insight:**  
> TinyML does not need large models — good features and simple classifiers are sufficient.

---

## 9. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Dataset | Correct feature–label mapping | 20% |
| Model | Trained and validated | 20% |
| Integration | Model runs on ESP32 | 30% |
| Inference | Correct classification | 20% |
| Explanation | CPS & TinyML understanding | 10% |

---

## 10. Student Reflection Questions

- Why is a simple model sufficient for TinyML?  
- What causes misclassification in real deployment?  
- How would quantization affect this model?

---

## 11. Preparation for Next Lab

In **Lab 5**, students will:
- Apply **INT8 quantization**  
- Measure **memory and latency improvements**  
- Compare **accuracy vs efficiency**

---

## 12. Key Takeaway

**Lab 4 delivers real intelligence:**  
> Trained ML models can run fully offline on ESP32 and make real-time decisions.
