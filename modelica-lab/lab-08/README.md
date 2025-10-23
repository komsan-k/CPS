# 🧩 Lab 8: Cyber-Physical Fault Detection and Self-Healing Control

### 🎯 Objective
Students will learn to:
1. Inject and detect faults in CPS components.
2. Implement fault monitoring and self-healing mechanisms.
3. Evaluate system resilience under fault conditions.

---

## 1. Prerequisites
- Completion of Lab 7: Predictive Maintenance.
- Use [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
Self-healing CPS can detect, isolate, and recover from component faults.

| Fault Type | Example | Effect |
|-------------|----------|--------|
| Sensor fault | Biased or stuck reading | Wrong feedback |
| Actuator fault | Weak force | Low response |
| Structural fault | Stiffness loss | Reduced dynamics |
| Communication fault | Delay/loss | Timing issues |

---

## 3. Base Plant Model
```modelica
model BasePlant
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.5);
  ForceActuator act;
  Real x, v;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  m1.F = act.F + s1.F + d1.F;
  act.u = 0;
  x = m1.x;
  v = m1.v;
end BasePlant;
```

---

## 4. Fault Injection
```modelica
model FaultInjectedCPS
  BasePlant plant;
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  Real x_ref = 1;
  Real x_meas;
  Real u_cmd;
  Boolean fault_sensor, fault_actuator;
equation
  fault_sensor = time > 4;
  x_meas = if fault_sensor then plant.x + 0.3 else plant.x;
  fault_actuator = time > 6;
  u_cmd = pid.u * (if fault_actuator then 0.6 else 1.0);
  pid.x_ref = x_ref;
  pid.x_meas = x_meas;
  plant.act.u = u_cmd;
end FaultInjectedCPS;
```

Run:
```modelica
simulate(FaultInjectedCPS, stopTime=10)
plot({plant.x, pid.x_ref})
```

---

## 5. Fault Detector
```modelica
model FaultDetector
  input Real x_meas, x_pred;
  output Boolean fault_detected;
  parameter Real threshold = 0.1;
equation
  fault_detected = abs(x_meas - x_pred) > threshold;
end FaultDetector;
```

---

## 6. Self-Healing CPS
```modelica
model SelfHealingCPS
  FaultInjectedCPS sys;
  IdealTwin twin(k_nom=2, c_nom=0.5, m_nom=1);
  FaultDetector fd;
  parameter Real Kp_normal = 3, Kp_backup = 5;
equation
  twin.F = sys.plant.act.F;
  fd.x_meas = sys.x_meas;
  fd.x_pred = twin.x;
  sys.pid.Kp = if fd.fault_detected then Kp_backup else Kp_normal;
end SelfHealingCPS;
```

Run:
```modelica
simulate(SelfHealingCPS, stopTime=10)
plot({sys.plant.x, sys.pid.x_ref})
```

---

## 7. Event-Based Reconfiguration
```modelica
when fd.fault_detected then
  reinit(sys.pid.Ki, sys.pid.Ki * 1.5);
  reinit(sys.pid.Kd, sys.pid.Kd * 0.8);
end when;
```

✅ Demonstrates controller retuning upon fault detection.

---

## 8. Communication Delay Example
```modelica
model NetworkDelay
  input Real inSig;
  output Real outSig;
  parameter Real delay = 0.2;
equation
  outSig = delay(inSig, delay);
end NetworkDelay;
```

---

## 9. Performance Metrics
| Metric | Description | Formula |
|---------|--------------|----------|
| Residual | Fault indicator | r = |x_meas - x_pred| |
| Recovery time | Time to regain stability | Δt |
| Resilience index | Robustness | R = 1 - r_max/x_ref |

---

## 10. Example Results
| Time (s) | Fault | PID Kp | Residual | Comment |
|-----------|--------|---------|-----------|----------|
| 0–4 | None | 3.0 | 0.00 | Normal |
| 4–6 | Sensor bias | 3.0 | 0.32 | Detected |
| >6 | Actuator loss | 5.0 | 0.11 | Healed |

---

## 11. Exercises
1. Add a stuck sensor fault: `x_meas = if time > 7 then pre(x_meas) else plant.x;`
2. Adjust threshold to 0.05 or 0.2.
3. Adapt integral gain in reconfiguration.
4. Combine delay and fault for analysis.
5. Log detection timestamps to CSV.

---

## 12. Learning Outcomes
- Simulate CPS faults in Modelica.
- Detect abnormal behavior using residuals.
- Implement adaptive self-healing control.
- Measure resilience and recovery time.
- Relate to real IoT fault-tolerant systems.

---

## 13. Next Lab
**Lab 9:** Multi-Agent Coordination in CPS.
