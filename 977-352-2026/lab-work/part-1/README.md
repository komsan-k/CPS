# 📝 Evaluation Scheme (10 Marks)
## Digital Twin for an On–Off Machine (ESP32 + Node-RED + MQTT)

This document defines the **evaluation rubric (10 marks)** for assessing student understanding and implementation of a **Digital Twin–based Cyber-Physical System (CPS)** using **ESP32, MQTT, and Node-RED**.

---

## 📊 Evaluation Rubric (Total: 10 Marks)

| No. | Evaluation Component | Criteria | Marks |
|----:|---------------------|----------|:----:|
| 1 | **Physical System Control** | ESP32 interrupt correctly toggles LED; manual on–off works reliably | 2 |
| 2 | **Perception & Fault Detection** | LDR correctly senses LED state and detects abnormal conditions | 2 |
| 3 | **IoT Communication** | ESP32 publishes data via MQTT and Node-RED receives it correctly | 2 |
| 4 | **Digital Twin Visualization** | Node-RED Dashboard reflects real-time LED/LDR status (virtual = physical) | 2 |
| 5 | **Cognition / Control Logic** | Rule-based prediction or backup activation is demonstrated | 2 |
| **Total** |  |  | **10** |

---

## 🎓 Instructor Notes

- Award **partial marks** if functionality is demonstrated but unstable.
- Cognition may be **simple rule-based logic** (machine learning is *not* required).
- Emphasis is on **CPS understanding and architecture**, not code complexity.

---

## ✅ Passing Benchmark

- **≥ 6 / 10** → Functional CPS with Digital Twin  
- **≥ 8 / 10** → Well-integrated CPS with clear Digital Twin  
- **10 / 10** → Complete closed-loop CPS with cognition and autonomy  

---

## 📚 Keywords
Digital Twin · Cyber-Physical Systems · ESP32 · MQTT · Node-RED · IoT · CPS Evaluation
