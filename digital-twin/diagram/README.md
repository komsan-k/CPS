# Digital Twin Diagram

## Overview
This resource provides a **clean, textbook-quality Digital Twin diagram** designed for
**Cyber-Physical Systems (CPS)** courses, textbooks, journal papers, and standards-aligned
technical documents.

The diagram emphasizes:
- Clear separation between **Physical Space** and **Cyber Space**
- Explicit **closed-loop feedback**
- Compatibility with **ISO/IEC CPS and IoT reference architectures**
- Suitability for **lecture slides, textbooks, and lab manuals**

---

## Conceptual Definition
A **Digital Twin** is a virtual, continuously synchronized representation of a physical
system that enables monitoring, analysis, prediction, and closed-loop control.

In CPS terms:

```
Physical System ⇄ Sensors/Data ⇄ Digital Twin ⇄ Decision ⇄ Actuators
```

---

## Diagram Description

### Physical Space
- Sensors (temperature, light, vibration, etc.)
- Actuators (LEDs, motors, valves)
- Real-world assets (machines, devices, environments)

### Cyber Space (Digital Twin Core)
- Digital models (physics-based, data-driven, or hybrid)
- Analytics and simulation
- Decision and control logic

### Synchronization Layer
- Time alignment
- State update
- Data validation

### Key Feature
The diagram explicitly illustrates the **closed CPS loop**:

> **Sense → Synchronize → Model → Predict → Decide → Act**

This distinguishes a **true Digital Twin–enabled CPS** from offline simulation
or dashboard-only monitoring systems.

---

## Standards Alignment

The diagram is compatible with:
- ISO/IEC 30141 (IoT Reference Architecture)
- ISO/IEC/IEEE 42010 (System Architecture Description)
- ISO 23247 (Digital Twin Framework)
- RAMI 4.0 (Industry 4.0 Architecture Model)

---

## Intended Use
This diagram can be used for:
- CPS and IoT textbooks
- Lecture slides (undergraduate and graduate)
- CPS laboratory manuals
- Journal papers and technical reports
- Industry 4.0 / 5.0 system documentation

---

## Suggested Figure Caption
**Figure X.** Digital Twin architecture in a Cyber-Physical System (CPS). The physical system
is continuously synchronized with its digital counterpart through sensor data and a data
synchronization layer, enabling analysis, prediction, and closed-loop control.

---

## License / Usage
This diagram and README are intended for **educational and academic use**.
You may adapt or redraw the figure for teaching, research, and publication
with appropriate citation.

---

## Next Extensions (Optional)
- ISO/IEC 30141–mapped Digital Twin diagram
- 5C Architecture–based Digital Twin
- ESP32 + MQTT + Node-RED Digital Twin lab
- TikZ / LaTeX version for journals

---

**Author use-case**: CPS education, Digital Twin textbooks, Industry 4.0 laboratories
