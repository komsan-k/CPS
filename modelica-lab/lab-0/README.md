# 🧩 Lab 0: Getting Started with Modelica and Your First Simulation

### 🎯 Objective
Students will learn to:
1. Set up and run their first simulation in OpenModelica.
2. Understand how Modelica expresses physical laws using differential equations.
3. Visualize simulation results for a basic RC charging circuit.

---

## ⚙️ 1. Introduction to Modelica
Modelica is a high-level, declarative language used to model multi-domain systems using equations rather than procedural steps.

💡 Example domains:
- Mechanical systems (mass–spring–damper)
- Electrical systems (RC, RLC circuits)
- Mechatronics and robotics
- Cyber-Physical Systems (CPS)

Modelica focuses on **what** the system does, not **how** it does it.

---

## ⚙️ 2. Creating Your First Model

Let's create a simple RC Circuit model.

---

### 🧠 Background
An RC circuit charges or discharges exponentially according to:

C * dv(t)/dt + v(t)/R = 1

where:
- R is the resistance (Ω)
- C is the capacitance (F)
- v(t) is the capacitor voltage (V)

The voltage across the capacitor increases over time as it charges toward 1 V.

---

## 🧩 Step 1: Define the Model
Paste this into a new code cell in OpenModelica Notebook:

```modelica
model RC_Circuit
  parameter Real R = 10;   // Resistance in ohms
  parameter Real C = 0.1;  // Capacitance in farads
  Real v(start=0);         // Capacitor voltage
equation
  der(v) = (1 - v)/(R*C);  // Differential equation
end RC_Circuit;
```

---

## 🧩 Step 2: Simulate and Plot

Run:

```modelica
simulate(RC_Circuit, stopTime=5)
plot(v)
```

✅ You’ll see an exponential charging curve approaching 1 V.

---

## 📊 3. Interpretation

The analytical solution of the voltage is:

v(t) = 1 - exp(-t/(R*C))

- The time constant τ = R * C = 1 s.
- At t = τ, v(t) ≈ 0.63 V (63% of final value).

This demonstrates the exponential nature of first-order systems.

---

## 🧠 4. Exercises
1. Change R to 5 Ω and observe how the charging speed changes.
2. Change C to 0.5 F and compare the response.
3. Add a discharge stage by setting voltage source = 0 after t = 2.
4. Plot current by adding `plot(der(v))` to observe the exponential decay.

---

## 💡 5. Learning Outcomes
After completing this lab, students will be able to:
- Write and simulate a simple Modelica model.
- Understand the concept of differential equations in Modelica.
- Visualize and interpret RC circuit dynamics.
- Prepare for hierarchical modeling in Lab 1.

---

## 🧭 Next Lab
**Lab 1:** Introduction to Cyber-Physical Systems and Modelica Basics.
