# Chapter 2: CPS Architectures and Models

## Chapter Overview
This chapter explores the **architectural frameworks** and **system models** that underpin Cyber-Physical Systems (CPS).  

Building on the foundation from Chapter 1, we examine:
- Reference architectures: **5C, IIRA, RAMI 4.0**  
- Layered abstractions  
- Discrete-event and hybrid models  
- Co-design principles (hardware, software, communication)  

Hands-on: modeling CPS data flows in **Node-RED** using data injection, transformation, and visualization (no physical hardware required).

---

## Learning Outcomes
By the end of this chapter, students will be able to:

1. Describe layered CPS architectures and their functional decomposition.  
2. Explain the **5C architecture** (Connection, Conversion, Cyber, Cognition, Configuration).  
3. Contrast **IIRA (Industrial Internet Reference Architecture)** with **RAMI 4.0**.  
4. Model CPS processes using discrete-event and hybrid system representations.  
5. Build a simple data flow model in **Node-RED** using inject, function, and dashboard nodes.  

---

## Key Terms
`5C architecture, IIRA, RAMI 4.0, discrete-event system (DES), hybrid system, layered CPS model, co-simulation, Node-RED dataflow`

---

## Reference Architectures

### The 5C Architecture
(Lee et al., 2015) – widely used in smart manufacturing:

1. **Connection** – sensors/devices collect raw data.  
2. **Conversion** – raw data processed into information.  
3. **Cyber** – information aggregated, analyzed, stored.  
4. **Cognition** – decision-making and knowledge generation.  
5. **Configuration** – adaptive feedback to physical systems.  

---

### IIRA (Industrial Internet Reference Architecture)
Defined by the Industrial Internet Consortium:

- **Business Viewpoint** – stakeholders, values, ROI, compliance.  
- **Usage Viewpoint** – functional requirements, workflows, scenarios.  
- **Functional Viewpoint** – capabilities grouped into control, operations, information, and business.  

---

### RAMI 4.0 (Reference Architectural Model for Industry 4.0)
Three-dimensional cube:

- **Hierarchy levels** – field devices → enterprise.  
- **Layers** – asset, integration, communication, information, functional, business.  
- **Lifecycle/Value stream** – from design (type) to runtime (instance).  

Focus: lifecycle integration and standardized interfaces.

---

## Layered CPS Models
A simplified layered model:

1. **Human Layer** – operators, dashboards, alarms.  
2. **Application Layer** – analytics, decision support, digital twins.  
3. **Cyber Layer** – estimation, control, coordination, computation.  
4. **Communication Layer** – networks (MQTT, OPC UA, security).  
5. **Perception Layer** – sensors, signal processing.  
6. **Actuation Layer** – actuators, motors, drivers.  

*Flow: perception → communication → cyber → application → human → actuation*

---

## System Models
- **DES (Discrete-Event Systems):** packet arrivals, threshold crossings, alarms.  
- **Continuous Models:** differential equations for physical plants.  
- **Hybrid Models:** combine continuous + discrete (e.g., traffic lights).  

---

## CPS Co-Design Principles
1. **Hardware–software co-design** – partition functions across MCU, edge, cloud.  
2. **Timing-aware design** – allocate latency budgets.  
3. **Safety/Security first** – invariants and fail-safes built early.  
4. **Scalable architecture** – anticipate growth in nodes and data volume.  

---

## Case Study: Traffic Light CPS
- **Sensors** – inductive loops, cameras detect vehicles.  
- **Controllers** – compute phases from arrival rates.  
- **Actuators** – traffic lights.  
- **Communication** – intersections exchange load info.  
- **Model** – hybrid: continuous traffic flow + discrete FSM for lights.  

---

## Node-RED Lab 2: Modeling CPS Data Flow

### Objectives
- Model CPS pipeline without hardware.  
- Use **inject nodes** as simulated sensors.  
- Transform data using function nodes.  
- Display actuator states on dashboard.  

### Required Software
- Node-RED with `node-red-dashboard` package.  

### Procedure
1. Add two **inject nodes**: `Sensor A` and `Sensor B`.  
2. Connect both to a **function node** (Processing):  

```js
let v = Number(msg.payload);
msg.payload = {source: msg.topic, value: v, score: v*2};
return msg;
```

3. Connect function node to:  
   - a **gauge** (dashboard)  
   - a **debug** node  

4. Deploy, trigger injects, observe dashboard.

### Expected Output
- Sensor A injects `42` → Gauge shows `84`.  
- Sensor B injects `17` → Gauge shows `34`.  
- Debug panel prints JSON `{source, value, score}`.  

### Assessment Rubric (10 points)
- [2] Inject nodes with correct topics.  
- [3] Function node transforms payload.  
- [3] Gauge displays processed score.  
- [2] Debug messages consistent with input.  

### Starter Flow JSON
```json
[{"id":"tab2","type":"tab","label":"Lab2 - CPS Model"},
{"id":"injA","type":"inject","z":"tab2","name":"Sensor A","props":[{"p":"payload"},{"p":"topic","vt":"str"}],
"topic":"sensor/A","payload":"42","payloadType":"num","x":130,"y":100,"wires":[["fn2"]]},
{"id":"injB","type":"inject","z":"tab2","name":"Sensor B","props":[{"p":"payload"},{"p":"topic","vt":"str"}],
"topic":"sensor/B","payload":"17","payloadType":"num","x":130,"y":160,"wires":[["fn2"]]},
{"id":"fn2","type":"function","z":"tab2","name":"Processing","func":"let v = Number(msg.payload);\nmsg.payload = {source: msg.topic, value: v, score: v*2};\nreturn msg;","outputs":1,"noerr":0,"x":330,"y":130,"wires":[["uiGauge","dbg2"]]},
{"id":"uiGauge","type":"ui_gauge","z":"tab2","group":"uigrp2","order":1,"gtype":"gage","title":"Actuator Level","label":"units","format":"{{msg.payload.score}}","min":0,"max":"100","x":530,"y":130,"wires":[]},
{"id":"dbg2","type":"debug","z":"tab2","name":"debug","active":true,"tosidebar":true,"complete":"true","x":530,"y":90,"wires":[]},
{"id":"uigrp2","type":"ui_group","name":"Lab2","tab":"uitab2","order":1,"disp":true,"width":"6","collapse":false},
{"id":"uitab2","type":"ui_tab","name":"CPS Labs","icon":"dashboard","disabled":false,"hidden":false}]
```

---

## Summary
- CPS architectures organize sensing, communication, computation, and actuation.  
- Reference models (**5C, IIRA, RAMI 4.0**) give systematic guidance.  
- System models: discrete, continuous, hybrid.  
- Node-RED enables rapid prototyping of CPS dataflows.  

---

## Review Questions
1. What are the five layers of the 5C architecture? Give examples.  
2. How do IIRA viewpoints differ from the RAMI 4.0 cube?  
3. Distinguish between DES, continuous, and hybrid models.  
4. Why is hardware–software co-design critical in CPS?  

---

## Exercises
1. Extend Lab 2: add a third inject node (Sensor C), process with a new rule (e.g., square), show on a new gauge.  
2. Sketch RAMI 4.0 cube and map a smart factory use case.  
3. Model a traffic light CPS as a hybrid system (continuous flow + discrete FSM).  

---

## Further Reading
- L. Monostori, *Cyber-Physical Production Systems: Roots, Expectations and R&D Challenges*, 2014.  
- Industrial Internet Consortium, *Industrial Internet Reference Architecture (IIRA)*, 2019.  
- Plattform Industrie 4.0, *RAMI 4.0 Reference Model*, 2015.  
- P. Derler, E.A. Lee, A. Sangiovanni-Vincentelli, *Modeling Cyber-Physical Systems*, Proc. IEEE, 2012.  

