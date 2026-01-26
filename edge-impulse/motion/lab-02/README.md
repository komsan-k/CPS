# Lab 1: Motion Classification with Edge Impulse

## Overview
This laboratory focuses on **motion classification using Edge Impulse** based on data collected from smartphone motion sensors (accelerometer and gyroscope).

Students design a complete **machine learning pipeline**—from raw sensor data to trained classifier—and evaluate model performance using validation and live testing. This lab emphasizes **data quality, feature extraction, and model behavior**, which are critical before embedded deployment.

---

## Learning Objectives
After completing this lab, students will be able to:

- Explain the motion classification problem formulation
- Prepare and label motion datasets in Edge Impulse
- Design a signal-processing + ML pipeline
- Train and evaluate a motion classification model
- Interpret accuracy, loss, and confusion matrix results

---

## CPS Context
This lab implements the **cyber intelligence layer** of a Cyber-Physical System.

| CPS Element | Implementation |
|------------|----------------|
| Physical World | Human / device motion |
| Sensor | Smartphone accelerometer & gyroscope |
| Cyber | Edge Impulse ML pipeline |
| Decision | Motion class prediction |
| Actuation | Not included in this lab |

---

## Tools and Requirements
### Hardware
- Smartphone (Android or iOS)

### Software
- Edge Impulse account (free)
- Edge Impulse Mobile App
- Web browser (Chrome / Firefox recommended)

---

## Step 1: Dataset Review and Organization
1. Log in to **Edge Impulse Studio**
2. Open your project created in **Lab 0**
3. Verify dataset balance:
   - Similar number of samples per class
   - Correct labels
4. Remove noisy or incorrect samples if needed

---

## Step 2: Motion Windowing
In **Create Impulse**:

- Window size: 1000–2000 ms
- Window increase: 50%
- Frequency: default (from sensor)

This defines how motion signals are segmented into learning samples.

---

## Step 3: Feature Extraction
Select one processing block:

### Option A: Spectral Features (Recommended)
- Captures frequency-domain motion patterns
- Robust for walking and repetitive motion

### Option B: Flatten (Beginner-Friendly)
- Uses raw time-domain values
- Easier to understand, lower accuracy

---

## Step 4: Classifier Design
Select **Neural Network (Keras)** as the learning block.

Typical configuration:
- Dense layers: 1–2
- Neurons: 20–50
- Activation: ReLU
- Output: Softmax

Train for:
- 20–50 epochs

---

## Step 5: Model Training
1. Click **Train**
2. Observe:
   - Training accuracy
   - Validation accuracy
   - Loss curves
3. Inspect **Confusion Matrix** to identify misclassified motions

---

## Step 6: Model Evaluation
Evaluate the model using:
- Test dataset
- Live Classification

Record:
- Overall accuracy
- Per-class accuracy
- Stability over repeated motion

---

## Step 7: Live Classification Test
1. Open **Live Classification**
2. Perform each motion type:
   - no_motion
   - walking
   - shake
   - pickup
3. Observe prediction confidence in real time

---

## Expected Results
- Overall accuracy ≥ 80%
- Clear separation between motion classes
- Stable predictions for repeated motions

---

## Deliverables
Students must submit:
- Screenshot of training results
- Screenshot of confusion matrix
- Screenshot of live classification
- Short report (1–2 pages):
  - Feature selection
  - Model parameters
  - Observed limitations

---

## Reflection Questions
1. Which motion classes are hardest to distinguish?
2. How does window size affect accuracy?
3. Why is feature extraction critical in motion classification?

---

## Extension (Next Lab)
This lab prepares students for:

- **Lab 2:** Deploying the motion model to ESP32 (TinyML)
- **Lab 3:** Motion-triggered actuation
- **Lab 4:** Motion data streaming via MQTT


