# 🧩 Lab 2: Hierarchical Modeling of Cyber-Physical Systems (CPS)

### 🎯 Objective
Students will learn to:
1. Build modular CPS models using hierarchical design in Modelica.
2. Understand system decomposition into mechanical and control subsystems.
3. Combine physical and cyber components into a single simulation.

---

## 1. Prerequisites
- Completion of Lab 1: Introduction to Modelica and CPS Basics.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
CPS modeling benefits from **hierarchical abstraction**, where systems are broken into reusable subsystems:

```
[Controller] → [Actuator] → [Physical System] → [Sensor] → (Feedback)
```

Each subsystem is modeled as a separate component for modularity.

---

## 3. Basic Components

### Mass Model
```modelica
model Mass
  parameter Real m = 1;
  Real x(start=0);
  Real v(start=0);
  input Real F;
equation
  der(x) = v;
  m*der(v) = F;
end Mass;
```

### Spring Model
```modelica
model Spring
  parameter Real k = 2;
  input Real x;
  output Real F;
equation
  F = -k*x;
end Spring;
```

### Damper Model
```modelica
model Damper
  parameter Real c = 0.5;
  input Real v;
  output Real F;
equation
  F = -c*v;
end Damper;
```

---

## 4. Hierarchical Subsystem Assembly
```modelica
model MechanicalSystem
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.5);
  input Real u;
  output Real x;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  m1.F = u + s1.F + d1.F;
  x = m1.x;
end MechanicalSystem;
```

---

## 5. Adding Control Component
```modelica
model Controller
  parameter Real Kp = 3;
  parameter Real Ki = 0.5;
  Real e(start=0);
  Real ie(start=0);
  input Real x_ref;
  input Real x_meas;
  output Real u;
equation
  e = x_ref - x_meas;
  der(ie) = e;
  u = Kp*e + Ki*ie;
end Controller;
```

---

## 6. Complete Hierarchical CPS Model
```modelica
model HierarchicalCPS
  MechanicalSystem mech;
  Controller ctrl(Kp=3, Ki=0.5);
  Real x_ref = 1;
equation
  ctrl.x_ref = x_ref;
  ctrl.x_meas = mech.x;
  mech.u = ctrl.u;
end HierarchicalCPS;
```

Run:
```modelica
simulate(HierarchicalCPS, stopTime=10)
plot(mech.x)
```

✅ Observe that the controlled system tracks the reference position.

---

## 7. Visualization and Component Hierarchy
```
HierarchicalCPS
 ├── Controller
 │   ├── Gains (Kp, Ki)
 │   └── Signals (x_ref, x_meas)
 └── MechanicalSystem
     ├── Mass
     ├── Spring
     └── Damper
```

---

## 8. Exercises
1. Modify `Kp` and `Ki` values and observe response changes.
2. Add friction term in `Damper` model: `F = -c*v - 0.1*sign(v)`.
3. Create a new subsystem `ForceSensor` to measure force output.
4. Replace `Controller` with proportional-only control and compare performance.

---

## 9. Learning Outcomes
- Develop hierarchical and modular CPS models in Modelica.
- Connect mechanical and control subsystems.
- Understand signal flow between physical and cyber layers.
- Evaluate closed-loop control performance.

---

## 10. Next Lab
**Lab 3:** Sensing and Control in Cyber-Physical Systems.
