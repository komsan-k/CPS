# Final Course Project Submission Template
# Smart Traffic Light Controller Using ESP32 + TFLM + Node-RED

## Group Information
- **Course:** ______________________________________
- **Project Title:** ______________________________________
- **Number:** ______________________________________
- **Submission Date:** ______________________________________
---

## 1. Abstract
Provide a 150–250 word summary of the project.

---

## 2. Problem Statement
Describe the traffic control problem and objectives.

---

## 3. System Architecture
Insert the architecture diagram here.

**Figure 1. System Architecture**

---

## 4. Hardware Setup
List components used and functions.

---

## 5. Software Design
### 5.1 ESP32 Program
Explain `ESP32_TFLM_traffic.ino`

### 5.2 TinyML Training
Explain `train_model.py`

### 5.3 Node-RED Dashboard
Insert screenshots and explain the flow.

---

## 6. Experimental Results  
Include **at least 4 scenarios**.

### Scenario A: Major Road Heavy
- **Observation:**  
  East–West road shows high traffic density with frequent vehicle arrivals, while North–South traffic remains low.

- **Result:**  
  The controller maintains a longer green phase for the East–West direction to reduce queue buildup and average waiting time.

---

### Scenario B: Minor Road Congested
- **Observation:**  
  North–South road experiences a significant queue buildup, while East–West traffic decreases.

- **Result:**  
  The controller switches phase earlier to provide priority to the North–South road.

---

### Scenario C: Balanced Traffic
- **Observation:**  
  Both roads have similar traffic density and queue length.

- **Result:**  
  The controller performs adaptive phase switching with balanced green-time allocation.

---

### Scenario D: Pedestrian Crossing Request
- **Observation:**  
  A pedestrian push-button or pedestrian sensor is activated while the current vehicle phase is green.

- **Result:**  
  The controller waits until the minimum green time is satisfied, then safely transitions through:
  - yellow phase
  - all-red interval
  - pedestrian crossing phase (walk signal)

  After the pedestrian crossing interval, the controller resumes normal adaptive traffic operation.


---

## 7. Performance Metrics
| Metric | Value |
|---|---:|
| Average waiting time | |
| Queue length | |
| Inference latency | |

---

## 8. Discussion
Discuss system strengths, limitations, and possible improvements.

---

## 9. Conclusion
Summarize key outcomes and learning achievements.

---

## 10. Appendix
- screenshots

