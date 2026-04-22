# Lab Report Template  
## Unsupervised Learning for an LDR Light Sensor  
### ESP32 + MQTT + Python K-means + Node-RED Digital Twin Lab

---

## 1. Student Information

- **Course / Subject:** __________________________
- **Lab Title:** Unsupervised Learning for an LDR Light Sensor
- **Student Name:** __________________________
- **Student ID:** __________________________

---

## 2. Abstract

Write a brief summary of the lab in 150–250 words.

Suggested points:
- What system was built
- What sensor and platform were used
- What unsupervised learning method was used
- What outputs were obtained
- What the main result or conclusion was

**Abstract:**  
................................................................................  
................................................................................  
................................................................................  
................................................................................

---

## 3. Objective

State the purpose of the experiment.


**Your objectives:**  
1. ..............................................................................  
2. ..............................................................................  
3. ..............................................................................  
4. ..............................................................................  

---

## 4. Background / Theory

Explain the concepts related to the lab.

### 4.1 LDR Sensor


**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.2 Unsupervised Learning
Describe:
- what unsupervised learning is
- how it differs from supervised learning
- why it is useful when labels are unavailable

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.3 K-means Clustering
Describe:
- the principle of clustering
- why `k = 3` was chosen
- how clusters can represent DARK, NORMAL, and BRIGHT states

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 4.4 Cyber-Physical System Perspective
Explain the CPS pipeline in this lab.

**Suggested flow:**  
LDR Sensor → ESP32 ADC → Data Collection → Python K-means → Threshold Extraction → ESP32 Classification → Node-RED Dashboard

**Discussion:**  
................................................................................  
................................................................................  
................................................................................

---

## 5. Hardware and Software Requirements

### 5.1 Hardware
Fill in the components used.



### 5.2 Software
Fill in the tools used.

---

## 6. Circuit Design

### LDR Voltage Divider

Document the circuit used.

**LED pin used:** __________________________



### 6.4 Explanation
Explain how the voltage divider converts light intensity into a measurable ADC value.

................................................................................  
................................................................................  
................................................................................

---


## 7. Experimental Procedure

### 7.1 Phase 1: ESP32 Data Acquisition
Describe how raw LDR data were collected.

Include:
- ESP32 sketch used
- serial or MQTT collection method
- sampling interval
- lighting conditions tested

**Conditions tested:**
1. __________________________________
2. __________________________________
3. __________________________________
4. __________________________________
5. __________________________________

**Sampling interval:** __________________________  
**Number of samples collected:** __________________________

### 7.2 Phase 2: Dataset Preparation
Explain:
- file name used
- CSV structure
- whether missing/outlier values were checked

**CSV filename:** __________________________  
**Columns in dataset:** __________________________

**Preview of collected data:**

| Sample No. | ADC Value |
|---:|---:|
| 1 | ______ |
| 2 | ______ |
| 3 | ______ |
| 4 | ______ |
| 5 | ______ |

### 7.3 Phase 3: Python K-means Training
Describe the training process.

Include:
- normalization method
- clustering method
- number of clusters
- random state / repeat settings
- output files generated

**Write here:**  
................................................................................  
................................................................................  
................................................................................

### 7.4 Phase 4: Threshold Extraction
Record the learned cluster centroids and thresholds.

| Parameter | Value |
|---|---:|
| Centroid 1 | __________ |
| Centroid 2 | __________ |
| Centroid 3 | __________ |
| Threshold 1 | __________ |
| Threshold 2 | __________ |
| ADC minimum | __________ |
| ADC maximum | __________ |
| Threshold 1 in ADC | __________ |
| Threshold 2 in ADC | __________ |

### 7.5 Phase 5: Deployment to ESP32
Explain how learned thresholds were inserted into the ESP32 sketch for real-time classification.

................................................................................  
................................................................................  
................................................................................


---

## 9. Program Code Summary

Do not paste unnecessary full code unless required. Summarize the important parts.

### 9.1 ESP32 Data Collection Code
Describe:
- ADC reading
- publish or serial print
- timing

................................................................................

### 9.2 Python Clustering Code
Describe:
- CSV loading
- scaling
- K-means fitting
- centroid sorting
- threshold calculation
- plotting

................................................................................

### 9.3 ESP32 Deployment Code
Describe:
- normalization
- threshold comparison
- state labeling
- MQTT publishing
- LED control if used

................................................................................

---

## 9. Results

### 9.1 Raw Data Characteristics
Summarize the data collected.

| Metric | Value |
|---|---:|
| Minimum ADC | __________ |
| Maximum ADC | __________ |
| Mean ADC | __________ |
| Standard deviation | __________ |
| Total samples | __________ |

### 9.2 Clustering Result
Attach the clustering figure and explain the separation among groups.

**Figure . K-means clustering result**

[Insert figure here]

**Observation:**  
................................................................................  
................................................................................  
................................................................................

### 10.3 Derived Light-State Interpretation

| Range | Interpreted State |
|---|---|
| Below Threshold 1 | __________________ |
| Threshold 1 to Threshold 2 | __________________ |
| Above Threshold 2 | __________________ |

### 10.4 Real-Time Classification Test
Record several live tests.

| Test No. | Lighting Condition | ADC Value | Predicted State | Expected State | Correct? |
|---:|---|---:|---|---|---|
| 1 | __________________ | ______ | ______ | ______ | ______ |
| 2 | __________________ | ______ | ______ | ______ | ______ |
| 3 | __________________ | ______ | ______ | ______ | ______ |
| 4 | __________________ | ______ | ______ | ______ | ______ |
| 5 | __________________ | ______ | ______ | ______ | ______ |

---

## 11. Discussion

Discuss the meaning of the results.

Suggested points:
- Were the clusters well separated?
- Did the learned thresholds reflect real lighting conditions?
- Was the three-state interpretation reasonable?
- How accurate was the real-time deployment?
- What caused misclassification, if any?
- How did the digital twin help monitoring and control?

**Write your discussion here:**  
................................................................................  
................................................................................  
................................................................................  
................................................................................  
................................................................................  
................................................................................

---



## 12. Improvements and Future Work

Suggest ways to improve the lab.

**Write here:**  
1. ..............................................................................  
2. ..............................................................................  
3. ..............................................................................  
4. ..............................................................................  

---

## 13. Conclusion

Summarize the final outcome of the experiment in one paragraph.

................................................................................  
................................................................................  
................................................................................  
................................................................................

---

