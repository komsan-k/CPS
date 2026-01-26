# Lab 1: Motion Data Collection Using Smartphone Sensors

## Overview
This laboratory focuses on **systematic motion data collection** using a **smartphone as a sensing device** and **Edge Impulse** as the data acquisition platform.

Students learn how to collect, label, and manage high-quality motion datasets using built-in smartphone sensors (accelerometer and gyroscope). This lab emphasizes **data quality, repeatability, and labeling**, which are critical for reliable machine learning models in later labs.

---

## Learning Objectives
After completing this lab, students will be able to:

- Explain how smartphone motion sensors work
- Use a smartphone as a CPS sensing node
- Collect labeled motion data in Edge Impulse
- Apply best practices for data quality and consistency
- Prepare datasets suitable for motion classification

---

## CPS Context
This lab implements the **physical sensing and data acquisition stage** of a Cyber-Physical System.

| CPS Element | Implementation |
|------------|----------------|
| Physical World | Human / device motion |
| Sensor | Smartphone accelerometer & gyroscope |
| Cyber | Edge Impulse data ingestion |
| Decision | Not included in this lab |
| Actuation | Not included in this lab |

---

## Tools and Requirements
### Hardware
- Smartphone (Android or iOS)

### Software
- Edge Impulse account (free)
- Edge Impulse Mobile App
- Internet connection

---

## Step 1: Create or Open Edge Impulse Project
1. Go to https://studio.edgeimpulse.com
2. Create a new project or open an existing one
3. Project type:
   - **Motion / Accelerometer data**
4. Recommended project name:
   ```
   smartphone-motion-dataset
   ```

---

## Step 2: Install Edge Impulse Mobile App
- Download **Edge Impulse** from:
  - Google Play Store (Android)
  - Apple App Store (iOS)
- Log in using your Edge Impulse account

Your smartphone now functions as a **wireless motion sensing device**.

---

## Step 3: Understand Motion Sensors
Smartphones typically provide:
- **Accelerometer (ax, ay, az)** – linear motion
- **Gyroscope (gx, gy, gz)** – rotational motion

These signals are streamed directly into Edge Impulse.

---

## Step 4: Define Motion Classes
Before recording, define clear motion labels.

### Recommended Motion Classes
| Label | Description |
|------|------------|
| no_motion | Phone placed still on a flat surface |
| walking | Walking while holding the phone |
| shake | Shaking the phone by hand |
| pickup | Picking up the phone from rest |

Consistency in labels is essential.

---

## Step 5: Data Recording Procedure
Using the Edge Impulse mobile app:

- Recording length: **3–5 seconds**
- Samples per class: **10–20**
- Hold phone orientation consistently
- Perform one motion type per recording

Repeat recordings under similar conditions.

---

## Step 6: Data Review and Cleaning
In Edge Impulse Studio:

1. Open **Data Acquisition**
2. Inspect each sample:
   - Correct label
   - No unexpected motion
3. Delete incorrect or noisy samples if needed

Balanced datasets improve model performance.

---

## Expected Results
- Well-labeled motion dataset
- Similar number of samples per class
- Clear motion patterns in signal plots

---

## Deliverables
Students must submit:
- Screenshot of dataset overview
- Screenshot of raw signal plots
- Short report (1 page):
  - Motion classes used
  - Number of samples per class
  - Data collection challenges

---

## Common Mistakes to Avoid
- Mixing motion types in one recording
- Inconsistent phone orientation
- Too few samples per class
- Recording while labels are incorrect

---

## Extension (Next Labs)
This lab prepares students for:
- **Lab 2:** Motion Classification with Edge Impulse
- **Lab 3:** Deploying Motion Model to ESP32
- **Lab 4:** Motion-Based Actuation

---

## Key Takeaway
High-quality data collection is the **foundation of successful TinyML and CPS systems**.  
Good models start with good data.


