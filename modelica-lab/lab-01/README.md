# 🧩 Lab 1: Introduction to Cyber-Physical Systems (CPS) and Modelica Basics

### 🎯 Objective
Students will learn to:
1. Understand the concept and structure of a Cyber-Physical System (CPS).
2. Explore Modelica as a modeling language for physical systems.
3. Simulate basic physical dynamics and interpret results.

---

## 1. Prerequisites
- Familiarity with differential equations and basic control concepts.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org) or OpenModelica desktop IDE.

---

## 2. Background
A **Cyber-Physical System (CPS)** integrates *physical dynamics* (mechanics, electronics) with *cyber components* (control, computation, communication).

| Layer | Description | Example |
|--------|-------------|----------|
| Physical | Real-world process | Mass–spring–damper system |
| Cyber | Sensing, control, computation | PID controller |
| Communication | Data exchange | Network delay, sampling |

### CPS Abstraction
```
[Controller] → [Actuator] → [Physical Process] → [Sensor] → (Feedback Loop)
```

---

## 3. Getting Started with Modelica
### Basic Syntax
```modelica
model Example
  parameter Real k = 2 "Spring constant";
  Real x(start=0), v(start=0);
equation
  der(x) = v;
  der(v) = -k*x;
end Example;
```

Run simulation:
```modelica
simulate(Example, stopTime=10)
plot(x)
```

✅ Observe sinusoidal oscillations.

---

## 4. CPS Concept Demonstration: Mass–Spring–Damper
```modelica
model MassSpringDamper
  parameter Real m = 1;
  parameter Real k = 2;
  parameter Real c = 0.5;
  Real x(start=0), v(start=0);
  input Real F "External force";
equation
  der(x) = v;
  m*der(v) = F - c*v - k*x;
end MassSpringDamper;
```

Simulate a free response (F = 0):
```modelica
simulate(MassSpringDamper, stopTime=10)
plot(x)
```

---

## 5. Adding an External Force
```modelica
model ForcedOscillator
  parameter Real m = 1;
  parameter Real k = 2;
  parameter Real c = 0.5;
  Real x(start=0), v(start=0);
  Real F;
equation
  der(x) = v;
  m*der(v) = F - c*v - k*x;
  F = sin(2*time);
end ForcedOscillator;
```

✅ Shows forced vibration response.

---

## 6. Visualizing Energy Exchange
```modelica
model EnergyDemo
  MassSpringDamper sys(m=1, k=2, c=0.5);
  Real E_kin, E_pot;
equation
  E_kin = 0.5*sys.m*sys.v^2;
  E_pot = 0.5*sys.k*sys.x^2;
end EnergyDemo;
```

Run and plot:
```modelica
simulate(EnergyDemo, stopTime=10)
plot({E_kin, E_pot})
```

✅ Observe kinetic and potential energy exchange.

---

## 7. CPS System Representation
| Domain | Variable | Description |
|---------|-----------|-------------|
| Mechanical | x | Displacement |
| Cyber | F | Control signal |
| Communication | time | Synchronization |

---

## 8. Exercises
1. Change damping `c` from 0.5 → 0.1 and observe oscillations.
2. Add external force: `F = 0.5*sin(time)`.
3. Plot velocity vs. displacement (phase plot).
4. Add parameter for gravity and simulate free fall.

---

## 9. Learning Outcomes
- Explain CPS components and feedback structure.
- Simulate dynamic systems using Modelica.
- Analyze simple mechanical systems and energy flow.
- Prepare for hierarchical modeling in Lab 2.

---

## 10. Next Lab
**Lab 2:** Hierarchical Modeling of Cyber-Physical Systems.
