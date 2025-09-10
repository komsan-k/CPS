# Chapter 1: Foundations of Cyber-Physical Systems (CPS)

## Chapter Overview
Cyber-Physical Systems (CPS) integrate computation, communication, and control with physical processes.  
This chapter builds the conceptual foundation for the course, clarifies how CPS differs from embedded systems and IoT, introduces representative applications (smart cities, healthcare, energy, transportation, manufacturing), and frames the analytical lenses we will use across the textbook:

- Discrete-event/hybrid modeling  
- Sensing and actuation chains  
- Real-time constraints  
- Safety and security  

---

## Learning Outcomes
After completing this chapter, students will be able to:

1. Define CPS and explain the interplay among sensing, computation, control, actuation, and communication.  
2. Differentiate CPS from embedded systems, IoT solutions, and digital twins.  
3. Describe the CPS layered architecture (perception → network → cyber → actuation → human).  
4. Identify key design constraints: sampling, latency, jitter, reliability, and safety/security.  
5. Explain the role of discrete control algorithms and hybrid models in CPS.  
6. Set up a basic Node-RED environment and implement a “Hello CPS” dashboard flow.  

---

## Prerequisites
- Basic programming  
- Introductory signals and systems  
- Familiarity with microcontrollers (Arduino, ESP32, etc.)  

---

## Key Terms
`Cyber-Physical System, sensing chain, actuator, control loop, discrete-event system (DES), hybrid system, real-time constraint, latency budget, jitter, reliability, availability, safety, security, digital twin, MQTT, Node-RED`

---

## What Is a Cyber-Physical System?
A CPS is a feedback system where computational elements observe and influence physical processes:  

- **Sensors** capture plant state.  
- **Computation** produces decisions (estimation, control, scheduling).  
- **Actuators** apply inputs back to the plant.  
- **Networks** connect distributed components across space and organizations.  

### CPS vs. Embedded Systems
An embedded system is a computer inside a device.  
A CPS extends this with *tight feedback coupling* to physical dynamics, often distributed over networks, with explicit timing, safety, and resilience requirements.

### CPS vs. Internet of Things (IoT)
- IoT emphasizes connectivity and data exchange.  
- CPS emphasizes *control* and *closed-loop behavior*.  
- IoT devices become part of CPS when orchestrated to meet control/timing requirements.

### CPS vs. Digital Twins
A digital twin is a synchronized virtual model of a physical asset/process.  
- Enriches CPS with prediction, estimation, and what-if analysis.  
- It is a component/technique, not a CPS by itself.  

---

## CPS Application Domains
- **Smart Manufacturing:** robotic cells, predictive maintenance, inspection.  
- **Energy Systems:** microgrids, demand response, inverter/fault detection.  
- **Transportation/ITS:** traffic coordination, V2I safety.  
- **Healthcare:** patient monitoring, insulin delivery, rehabilitation robotics.  
- **Smart Cities/Buildings:** HVAC optimization, occupancy-aware lighting, alarms.  

---

## CPS Layered Reference Architecture
A canonical layered view of CPS:

1. **Human Layer:** dashboards, alarms, visualization.  
2. **Cyber Layer:** estimation, control, analytics, scheduling, edge/cloud.  
3. **Communication Layer:** MQTT, CoAP, OPC UA; Wi-Fi, BLE, LoRa; QoS, security.  
4. **Perception Layer:** sensors, signal conditioning, sampling, time-stamping.  
5. **Actuation Layer:** actuators, drivers, power electronics, safety interlocks.  

*Closed-loop: sensors → network → computation → actuators (timed, reliable, secure).*  

---

## Timing, Sampling, and Real-Time Constraints
- Sampling period: `T_s`  
- End-to-end deadline:  

```math
D = d_sense + d_net + d_compute + d_act
```

- Stability/performance require:  

```math
T_s ≤ D   and   T_s ≤ (1/10) * T_dyn
```

- Minimize jitter with priority scheduling, QoS, edge placement.  

**Reliability/Availability:** many CPS target >99.9%.  
- Redundancy  
- Watchdogs  
- Fail-safes  
- Degraded modes  

---

## Modeling Perspectives
- **DES (Discrete-Event Systems):** arrivals, switches.  
- **Continuous-time:** ODEs, physical dynamics.  
- **Hybrid systems:** combination (e.g., thermostat).  

**Example: Room Temperature Control**

```math
u(t) =
  HEAT, if x(t) < x_set - Δ
  OFF,  if x(t) > x_set + Δ
```

Controller executes every `T_s`; requires `D ≤ T_s`.

---

## CPS Toolchain and Platforms
- **Edge Hardware:** ESP32/Arduino, Raspberry Pi, PLCs, FPGAs.  
- **Middleware:** Node-RED, MQTT, OPC UA, containers.  
- **Modeling/Simulation:** Modelica, Simulink, Python/Matlab, digital twins.  

---

## Security and Safety at a Glance
Minimum practices:  
1. Authentication and least-privilege.  
2. Transport security (TLS) and topic-level authorization.  
3. Data validation (bounds, rate limits, plausibility).  
4. Safety interlocks and emergency stop paths.  

---

## Case Study: Smart Room Controller
Monitors temperature (T), humidity (H), occupancy (O), controls fan/light.  

- Update: 1 Hz  
- Latency: ≤ 500 ms  
- Energy saving when unoccupied  
- Fail-safe: defaults + alerts if no data in 5s  

---

## Node-RED Lab 1: Hello CPS World

### Objectives
- Install Node-RED and create a basic dataflow.  
- Understand messages, topics, and deploy cycle.  

### Required Software
- Node.js + Node-RED  
- Web browser  
- (Optional) `node-red-dashboard` package  

### Procedure
1. Inject → Function → Debug.  
2. Function code:

```js
msg.payload = "Hello CPS: " + new Date().toISOString();
return msg;
```

3. Deploy and test.  
4. Add dashboard `ui_text` node.  
5. Optional: repeating inject for streaming.  

### Expected Output
```
Hello CPS: 2025-09-10T00:00:00.000Z
```

### Assessment Rubric (10 points)
- [2] Node-RED installed/working  
- [3] Flow inject → function → debug  
- [3] Dashboard output correct  
- [2] Clear screenshot + explanation  

### Starter Flow JSON
```json
[{"id":"tab1","type":"tab","label":"Lab1 - Hello CPS"},
{"id":"inj1","type":"inject","z":"tab1","name":"tick","props":[{"p":"payload"}],
"repeat":"","crontab":"","once":false,"onceDelay":0.1,"topic":"",
"payloadType":"date","x":140,"y":120,"wires":[["fn1"]]},
{"id":"fn1","type":"function","z":"tab1","name":"format",
"func":"msg.payload = \"Hello CPS: \" + new Date(msg.payload).toISOString();\nreturn msg;",
"outputs":1,"noerr":0,"initialize":"","finalize":"","libs":[],
"x":330,"y":120,"wires":[["dbg1","ui1"]]},
{"id":"dbg1","type":"debug","z":"tab1","name":"debug","active":true,"tosidebar":true,
"console":false,"tostatus":false,"complete":"payload","statusVal":"",
"statusType":"auto","x":520,"y":90,"wires":[]},
{"id":"ui1","type":"ui_text","z":"tab1","group":"uigrp1","order":1,"width":0,"height":0,
"name":"","label":"Hello","format":"{{msg.payload}}","layout":"row-spread",
"x":520,"y":150,"wires":[]},
{"id":"uigrp1","type":"ui_group","name":"Lab1","tab":"uitab1","order":1,"disp":true,
"width":"6","collapse":false},
{"id":"uitab1","type":"ui_tab","name":"CPS Labs","icon":"dashboard",
"disabled":false,"hidden":false}]
```

---

## Design Constraints and Trade-offs
- **Performance:** rise time, throughput.  
- **Resources:** CPU, memory, bandwidth, energy.  
- **Timing:** latency vs. deadlines.  
- **Safety/Security:** invariants, authentication, fail-safe.  

---

## From Prototype to Infrastructure-Scale CPS
Challenges include:  
- Heterogeneity  
- Orchestration  
- Observability  
- Governance  

---

## Summary
CPS merge computation with real-world physics.  
Design requires attention to timing, reliability, safety/security.  
Node-RED provides a rapid way to prototype CPS before scaling to hardware or infrastructure systems.  

---

## Review Questions
1. Define CPS and list core subsystems.  
2. Differentiate CPS from embedded systems and IoT.  
3. What is a latency budget and why does jitter matter?  
4. Give an example of a hybrid system and its discrete logic.  
5. What security measures are essential for CPS in public infrastructure?  

---

## Exercises
1. (Modeling) Given `T_s=1s`, compute permissible delays with 20% margin.  
2. (Design) Draft an MQTT topic hierarchy for a smart-room CPS.  
3. (Prototype) Extend Lab 1 with periodic inject (1 Hz) and a dashboard chart.  

---

## Further Reading
- E. A. Lee, *Cyber Physical Systems: Design Challenges*, 2008.  
- R. Rajkumar et al., *Cyber-Physical Systems: The Next Computing Revolution*, 2010.  
- P. Tabuada, *Verification and Control of Hybrid Systems*, 2009.  
- OPC Foundation, *OPC UA Specifications*, latest edition.  

