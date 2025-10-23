# 🧩 Lab 3: Sensing and Control in Cyber-Physical Systems (CPS)

### 🎯 Objective
Students will learn to:
1. Model sensors and actuators in CPS using Modelica.
2. Implement feedback control with sensor measurements.
3. Visualize closed-loop performance and control response.

---

## 1. Prerequisites
- Completion of Lab 2: Hierarchical Modeling.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
Cyber-Physical Systems integrate sensing, computation, and actuation.  
This lab adds virtual sensors and controllers to the physical model.

```
[Sensor] → [Controller] → [Actuator] → [Physical System]
           ↑______________________________↓
                 Feedback Control Loop
```

---

## 3. Sensor, Actuator, and Controller Models

### Position Sensor
```modelica
model PositionSensor
  input Real x_true;
  output Real x_meas;
  parameter Real noise = 0.0;
equation
  x_meas = x_true + noise * sin(10*time);
end PositionSensor;
```

### Force Actuator
```modelica
model ForceActuator
  input Real u;
  output Real F;
  parameter Real gain = 1.0;
equation
  F = gain * u;
end ForceActuator;
```

### PID Controller
```modelica
model PIDController
  parameter Real Kp = 3;
  parameter Real Ki = 0.5;
  parameter Real Kd = 0.2;
  input Real x_ref;
  input Real x_meas;
  output Real u;
  Real e(start=0);
  Real ie(start=0);
equation
  e = x_ref - x_meas;
  der(ie) = e;
  u = Kp*e + Ki*ie + Kd*der(e);
end PIDController;
```

---

## 4. Closed-Loop CPS Model
```modelica
model CPS_MassSpringSystem
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.5);
  PositionSensor sensor(noise=0.05);
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  ForceActuator actuator(gain=1);
  Real x_ref = 1;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  sensor.x_true = m1.x;
  pid.x_ref = x_ref;
  pid.x_meas = sensor.x_meas;
  actuator.u = pid.u;
  m1.F = actuator.F + s1.F + d1.F;
end CPS_MassSpringSystem;
```

Run:
```modelica
simulate(CPS_MassSpringSystem, stopTime=10)
plot(m1.x)
```

✅ Observe position tracking of the mass to reference x_ref.

---

## 5. Sensor and Control Visualization
```modelica
plot({m1.x, sensor.x_meas})
plot(pid.u)
```

✅ Compare true vs. measured displacement and control signal.

---

## 6. CPS Component Roles
| Component | Domain | Function |
|------------|---------|-----------|
| PositionSensor | Physical→Cyber | Converts position to signal |
| PIDController | Cyber | Computes control effort |
| ForceActuator | Cyber→Physical | Converts control to force |
| Mass–Spring–Damper | Physical | Produces motion response |

---

## 7. Exercises
1. Increase noise to 0.1 and analyze system response.
2. Change controller gains (Kp, Ki, Kd) for faster or slower tracking.
3. Add a second actuator in parallel.
4. Log x_meas and u into a CSV file for analysis.

---

## 8. Learning Outcomes
- Implement a full CPS feedback loop in Modelica.
- Model sensors and actuators.
- Simulate closed-loop behavior.
- Analyze performance under noise and gain changes.

---

## 9. Next Lab
**Lab 4:** Networked Cyber-Physical Systems and Data Communication.
