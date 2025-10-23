# 📘 Course Overview: Cyber-Physical Systems (CPS) with Modelica

### 🏫 College of Computing, Prince of Songkla University  
### 🧑‍🏫 Instructor: Dr. Komsan Kanjanasit  
### 💻 Tools: [OpenModelica](https://openmodelica.org), [Modelica Standard Library](https://github.com/modelica/ModelicaStandardLibrary)

---

## 🎯 Course Objective
This laboratory course introduces Cyber-Physical Systems (CPS) through modeling and simulation using **Modelica**.  
Students learn to design, simulate, and integrate physical, cyber, and communication components into smart CPS prototypes.

---

## 🧩 Course Learning Outcomes
By the end of the course, students will be able to:
- Model mechanical, control, and network subsystems in Modelica.  
- Build hierarchical and modular CPS architectures.  
- Apply feedback, sensing, and actuation for control.  
- Simulate network delay, digital twins, and cloud connectivity.  
- Implement adaptive, fault-tolerant, and multi-agent CPS.

---

## 🧠 Laboratory Sequence Overview

| **Lab** | **Title** | **Key Focus** | **Learning Outcomes** |
|----------|------------|----------------|-------------------------|
| **1** | Introduction to CPS and Modelica Basics | CPS concept, feedback loop, Modelica syntax | Simulate dynamic systems and interpret results |
| **2** | Hierarchical Modeling of CPS | Modular subsystems and reusability | Connect mechanical and control layers |
| **3** | Sensing and Control in CPS | Sensors, actuators, PID feedback | Implement closed-loop CPS control |
| **4** | Networked CPS and Data Communication | Delay, packet loss, sampling | Study network latency effects |
| **5** | Digital Twin and Real-Time Visualization | Twin synchronization | Build and visualize digital twin models |
| **6** | Cloud-Connected CPS and IoT Integration | Data streaming and MQTT/REST APIs | Publish CPS data to dashboards |
| **7** | Digital Twin Synchronization and Predictive Maintenance | Health index and residuals | Predict degradation trends |
| **8** | Cyber-Physical Fault Detection and Self-Healing Control | Fault injection and recovery | Implement adaptive healing controllers |
| **9** | Multi-Agent Coordination in CPS | Consensus, formation control | Simulate cooperative agent systems |
| **10** | CPS Co-Design and System Integration | Full system integration | Combine sensing, control, and communication |

---

## 🔄 Progressive Skill Development
```
Lab 1–2 → Physical Modeling
Lab 3–4 → Feedback and Communication
Lab 5–7 → Digital Twin and Cloud
Lab 8–9 → Fault Tolerance and Multi-Agent CPS
Lab 10  → Co-Design Integration Project
```

---

## ⚙️ Software & Environment Setup

| Tool | Purpose | Notes |
|------|----------|--------|
| **OpenModelica** | Main simulation platform | Use Online Notebook or IDE |
| **Python / MQTT / Node-RED** | IoT connectivity | Required for Labs 6–10 |
| **CSV Logger** | Data export | Built-in scripting support |
| **ThingSpeak / HiveMQ / AWS IoT** | Cloud dashboards | Optional extensions |

---

## 📂 Repository Structure Example

```
CPS_Modelica_Labs/
│
├── Lab1_Introduction_to_CPS_README.md
├── Lab2_Hierarchical_Modeling_CPS_README.md
├── Lab3_Sensing_and_Control_CPS_README.md
├── Lab4_Networked_CPS_README.md
├── Lab5_DigitalTwin_CPS_README.md
├── Lab6_CloudConnected_CPS_README.md
├── Lab7_PredictiveMaintenance_CPS_README.md
├── Lab8_SelfHealing_CPS_README.md
├── Lab9_MultiAgent_CPS_README.md
├── Lab10_CoDesign_CPS_README.md
└── /assets (figures, models, CSV logs)
```

---

## 📊 Assessment Scheme

| Component | Weight | Description |
|------------|---------|-------------|
| Simulation Models | 30% | Correctness and completeness |
| Reports & Discussion | 30% | Technical explanation and insight |
| Cloud/IoT Integration | 20% | Real-time visualization and logging |
| Final Co-Design Project | 20% | Integrated CPS system (Lab 10) |

---

## 🧠 Capstone Project Suggestions
- **Smart Robotic Arm CPS** – feedback + predictive maintenance  
- **Connected Vehicle Platoon** – leader–follower coordination  
- **Smart Manufacturing Twin** – cloud-connected production CPS  
- **Digital Twin for HVAC Systems** – adaptive energy management  

---

## 📘 References
- Modelica Association. *Modelica Language Specification v3.6*  
- Peter Fritzson. *Introduction to Modeling and Simulation of Technical and Physical Systems with Modelica.* Wiley, 2011  
- Lee, E.A., et al. *Introduction to Cyber-Physical Systems.* MIT Press, 2020  
- Rajkumar, R., et al. *Cyber-Physical Systems: The Next Computing Revolution.* DAC, 2010  
