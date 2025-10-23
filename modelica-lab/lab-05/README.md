# 🧩 Lab 5: Digital Twin and Real-Time Visualization of Cyber-Physical Systems (CPS)

### 🎯 Objective
Students will learn to:
1. Build a digital twin of a physical CPS system.
2. Synchronize the digital twin with the physical model.
3. Visualize real-time responses and twin tracking in Modelica.

---

## 1. Prerequisites
- Completion of Lab 4: Networked CPS.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
A **Digital Twin** is a live, executable model that mirrors the behavior of a real system in real time.

| Domain | Element | Description |
|---------|----------|-------------|
| Physical | Plant | The actual dynamic process |
| Cyber | Twin | Digital model mirroring the plant |
| Connectivity | Data link | Sensor-to-model data synchronization |
| Visualization | Dashboard | Graphs or 3D animation of twin and plant |

---

## 3. Physical Plant Model
```modelica
model PhysicalPlant
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
  act.u = 1.0;
  x = m1.x;
  v = m1.v;
end PhysicalPlant;
```

---

## 4. Digital Twin Model
```modelica
model DigitalTwin
  input Real x_true;
  input Real v_true;
  output Real x_pred;
  output Real v_pred;
  parameter Real alpha = 0.9;
initial equation
  x_pred = 0;
  v_pred = 0;
equation
  der(x_pred) = alpha * (x_true - x_pred);
  der(v_pred) = alpha * (v_true - v_pred);
end DigitalTwin;
```

---

## 5. Physical–Digital Synchronization
```modelica
model CPS_DigitalTwin
  PhysicalPlant plant;
  DigitalTwin twin(alpha=2.0);
equation
  twin.x_true = plant.x;
  twin.v_true = plant.v;
end CPS_DigitalTwin;
```

Run:
```modelica
simulate(CPS_DigitalTwin, stopTime=10)
plot({plant.x, twin.x_pred})
```

✅ The digital twin tracks the physical system closely.

---

## 6. 3D Visualization (Desktop OpenModelica)
```modelica
model AnimatedTwin
  import Modelica.Mechanics.MultiBody.Visualizers.*;
  CPS_DigitalTwin sys;
  Shape massShape(shapeType="sphere", length=0.1, color={0,0,255});
equation
  massShape.r[1] = sys.plant.x;
  massShape.r[2] = 0;
  massShape.r[3] = 0;
end AnimatedTwin;
```

Run:
```modelica
simulate(AnimatedTwin, stopTime=10)
```

✅ Shows motion animation in 3D view.

---

## 7. Controlled Digital Twin
```modelica
model ControlledDigitalTwin
  import MyLibrary.*;
  PhysicalPlant plant;
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  DigitalTwin twin(alpha=3.0);
  Real x_ref = 1;
equation
  twin.x_true = plant.x;
  twin.v_true = plant.v;
  pid.x_ref = x_ref;
  pid.x_meas = twin.x_pred;
  plant.act.u = pid.u;
end ControlledDigitalTwin;
```

Run:
```modelica
simulate(ControlledDigitalTwin, stopTime=10)
plot({plant.x, twin.x_pred, pid.u})
```

✅ Demonstrates twin-based feedback control.

---

## 8. Digital Twin Architecture
| Layer | Function | Example |
|--------|-----------|----------|
| Physical | Plant dynamics | `PhysicalPlant` |
| Cyber Twin | Estimation/prediction | `DigitalTwin` |
| Visualization | Animation or plotting | `Shape` / `plot()` |
| Controller | Decision/control | `PIDController` |

---

## 9. Exercises
1. Add noise: `plant.x + 0.05*sin(10*time)` and test twin tracking.
2. Change spring stiffness at t > 5 s: `when time>5 then s1.k=3; end when;`
3. Predict `x_pred` ahead using derivative extrapolation.
4. Use `twin.x_pred` in controller feedback and compare results.

---

## 10. Learning Outcomes
- Build a digital twin model in Modelica.
- Synchronize physical and digital domains.
- Visualize twin tracking in real-time.
- Understand twin-based CPS feedback design.

---

## 11. Next Lab
**Lab 6:** Cloud-Connected CPS and IoT Integration.
