# 🧩 Lab 10: CPS Co-Design and System Integration

### 🎯 Objective
Students will learn to:
1. Integrate sensing, control, communication, digital-twin, and cloud layers into a unified CPS model.
2. Apply co-design principles—joint optimization of cyber and physical subsystems.
3. Evaluate performance trade-offs in real-time, multi-agent CPS environments.

---

## 1. Prerequisites
Review Labs 3–9 and use [OpenModelica Notebook](https://notebook.openmodelica.org) or the desktop IDE.

---

## 2. Background
CPS Co-Design means the simultaneous design of hardware (physical), software (cyber), and network (communication) components.

| Layer | Role | Components |
|-------|------|-------------|
| Physical | Dynamics, actuation | Mass–spring–damper |
| Cyber | Control, computation | PID, AI logic |
| Network | Communication | Delay, MQTT |
| Digital Twin | State estimation | Predictive model |
| Cloud | Visualization | CSV, Node-RED |

---

## 3. System Overview
Integrated CPS with two agents, shared twin, and cloud logger.

```
[Agent 1] <-> [Agent 2]
    ↓             ↓
 [Digital Twin Monitor]
            ↓
     [Cloud Logger]
```

---

## 4. Smart Agent Model
```modelica
model SmartAgent
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.4);
  ForceActuator act;
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  Real x_ref;
  Real x_meas;
  Boolean fault;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  fault = time > 6;
  x_meas = if fault then m1.x + 0.2 else m1.x;
  pid.x_ref = x_ref;
  pid.x_meas = x_meas;
  act.u = pid.u;
  m1.F = act.F + s1.F + d1.F;
end SmartAgent;
```

---

## 5. Coupled Multi-Agent Network
```modelica
model IntegratedCPS
  SmartAgent A1;
  SmartAgent A2;
  parameter Real k_net = 0.6;
  Real x_mean;
equation
  A1.x_ref = A1.m1.x + k_net*(A2.m1.x - A1.m1.x);
  A2.x_ref = A2.m1.x + k_net*(A1.m1.x - A2.m1.x);
  x_mean = (A1.m1.x + A2.m1.x)/2;
end IntegratedCPS;
```

---

## 6. Digital Twin Monitor
```modelica
model TwinMonitor
  IntegratedCPS sys;
  Real residual1, residual2;
  Real health1, health2;
  Real system_health;
equation
  residual1 = abs(sys.A1.m1.x - sys.A2.m1.x);
  residual2 = abs(sys.A1.m1.x - sys.x_mean);
  health1 = exp(-residual1);
  health2 = exp(-residual2);
  system_health = (health1 + health2)/2;
end TwinMonitor;
```

---

## 7. Cloud Data Logger
```modelica
model CloudLogger
  TwinMonitor tm;
  parameter Real Ts = 0.5;
  String filename = "cps_co_design.csv";
  Integer fileID;
initial algorithm
  fileID := OpenModelica.Scripting.openFile(filename,"w");
  OpenModelica.Scripting.writeFile(fileID,"time,health,A1_x,A2_x\n");
equation
  when sample(0, Ts) then
    OpenModelica.Scripting.writeFile(fileID,
      String(time) + "," + String(tm.system_health) + "," +
      String(tm.sys.A1.m1.x) + "," + String(tm.sys.A2.m1.x) + "\n");
  end when;
end CloudLogger;
```

---

## 8. Full Co-Design Model
```modelica
model CPS_CoDesign
  CloudLogger cl;
  parameter Real delay_net = 0.1;
  parameter Real gain_adapt = 1.5;
equation
  cl.tm.sys.A1.pid.x_ref = delay(cl.tm.sys.A1.x_ref, delay_net);
  cl.tm.sys.A2.pid.x_ref = delay(cl.tm.sys.A2.x_ref, delay_net);
  when cl.tm.system_health < 0.8 then
    reinit(cl.tm.sys.A1.pid.Kp, cl.tm.sys.A1.pid.Kp * gain_adapt);
    reinit(cl.tm.sys.A2.pid.Kp, cl.tm.sys.A2.pid.Kp * gain_adapt);
  end when;
end CPS_CoDesign;
```

---

## 9. Performance Metrics
| Metric | Description | Formula |
|---------|-------------|----------|
| Tracking error | Difference to ref | e = |x_ref - x| |
| Health index | Twin residual | h = exp(-|x1 - x2|) |
| Recovery time | Duration to stability | Δt |
| Resilience index | Robustness | R = 1 - e_max/x_ref |

---

## 10. Example Results
| Time (s) | A1 | A2 | Health | Comment |
|-----------|----|----|---------|----------|
| 0–5 | Stable | Stable | 0.99 | Normal |
| 6 | Fault | – | 0.72 | Detected |
| 7 | Adaptive active | – | 0.88 | Recovering |
| 10 | Resync | – | 0.95 | Healed |

---

## 11. Exercises
1. Change delay_net to 0.5 s and analyze performance.
2. Add third agent for scalability.
3. Use continuous adaptation: `Kp = 3 + (1 - health)*2`.
4. Connect MQTT cloud publisher.
5. Compare co-design vs. static control.

---

## 12. Learning Outcomes
- Combine sensing, control, and cloud in one Modelica CPS.
- Apply co-design and adaptive optimization.
- Use digital twin for resilience and monitoring.
- Evaluate trade-offs between delay, control, and communication.

---

## 13. Course Recap
| Lab | Focus | Outcome |
|------|--------|----------|
| 1 | Modelica Basics | Equation modeling |
| 2 | Hierarchical Systems | Modular design |
| 3 | Sensing & Control | Feedback loops |
| 4 | Networked CPS | Delay & loss |
| 5 | Digital Twin | Synchronization |
| 6 | Cloud Integration | IoT data streams |
| 7 | Predictive Maintenance | Health monitoring |
| 8 | Self-Healing | Adaptive control |
| 9 | Multi-Agent | Consensus formation |
| 10 | Co-Design | Full integration |

---

## 14. Evaluation
| Component | Weight | Method |
|------------|---------|--------|
| Model accuracy | 20% | Simulation |
| Controller adaptation | 20% | Response plots |
| Health monitoring | 20% | Twin metrics |
| Cloud logging | 20% | Data export |
| Report | 20% | Analysis & discussion |
