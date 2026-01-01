# Lab 3 – Signal Processing and Feature Extraction (C2: Conversion)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand the **Conversion (C2) layer** of the 5C Cyber-Physical System (CPS) model
- Transform raw sensor data into meaningful information
- Apply **digital signal processing (DSP)** techniques for CPS data
- Configure feature extraction blocks in **Edge Impulse**
- Prepare optimized features for embedded intelligence

---

## 🧠 Background Theory

### 1. Conversion Layer in CPS (C2)

The **Conversion layer (C2)** converts raw sensor data into **information-rich representations** suitable for analysis and intelligence.
It bridges low-level sensing (C1) and higher-level cognition (C4).

Typical functions include:
- Filtering and normalization
- Windowing and segmentation
- Feature extraction

---

### 2. Why Signal Processing Is Essential in CPS

Raw sensor signals are often:
- Noisy
- High-dimensional
- Redundant

Signal processing improves:
- Signal-to-noise ratio
- Computational efficiency
- Model accuracy

---

### 3. Feature Extraction Concepts

Features are compact representations of signals.
Common CPS features:
- Time-domain statistics (mean, RMS, variance)
- Frequency-domain features (FFT, spectral energy)
- Time–frequency features (spectrograms)

---

### 4. Edge Impulse Conversion Blocks

Edge Impulse provides DSP blocks such as:
- Raw signal
- Spectral analysis
- MFCC (audio)
- Flattened statistical features

These blocks are optimized for embedded systems.

---

## 🧱 CPS Focus of This Lab

- **C2 – Conversion:** Primary focus  
- C1 – Connection: Data source  
- C3–C5: Not yet implemented  

---

## ⚙️ Edge Impulse Focus

- Designing the impulse pipeline
- Selecting DSP blocks
- Tuning window size and overlap
- Evaluating feature quality

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor from Lab 2 (IMU / temperature / vibration / microphone)
- USB cable

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse CLI
- Edge Impulse Studio (Web)

---

## 🔧 Experimental Procedure

### Step 1: Load Dataset from Lab 2
1. Open the Edge Impulse project.
2. Navigate to **Data Acquisition**.
3. Verify that labeled sensor data exists.

---

### Step 2: Create an Impulse
1. Go to **Create Impulse**.
2. Set window size (e.g., 1000–2000 ms).
3. Set window overlap (e.g., 50%).
4. Select appropriate DSP block.
5. Save the impulse.

---

### Step 3: Configure DSP Parameters
1. Open the DSP block.
2. Adjust parameters (FFT length, axes selection).
3. Generate features.
4. Observe feature visualizations.

---

### Step 4: Feature Evaluation
1. Use feature explorer.
2. Check class separation.
3. Identify redundant or noisy features.

---

## 📊 Results and Observations

Students should observe:
- Reduced data dimensionality
- Clear feature patterns
- Improved class separability

---

## 📝 Discussion Questions

1. Why is feature extraction critical in CPS?
2. How does window size affect features?
3. What trade-offs exist between accuracy and computation?
4. How does C2 influence C4 cognition?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Explain the role of the Conversion layer
- Apply DSP techniques to CPS data
- Configure feature extraction pipelines
- Evaluate feature quality for embedded ML

---

## 🧾 Conclusion

This lab demonstrated how raw CPS sensor data is converted into meaningful features using Edge Impulse. These features form the foundation for machine learning–based cognition in subsequent labs.

---

## 📚 References

1. Edge Impulse Documentation  
2. Digital Signal Processing Fundamentals  
3. CPS Architecture Literature  

---

### ▶️ Next Lab

**Lab 4 – Classification Models for CPS Cognition (C4)**
