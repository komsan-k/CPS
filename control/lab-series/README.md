# 6-Lab Series: Control Theory for CPS with ESP32 + Node-RED

This lab series introduces **control theory concepts** through hands-on **Cyber-Physical Systems (CPS)** experiments using **ESP32**, **MQTT**, and **Node-RED dashboards**. The labs progress from basic feedback control to networked and constraint-aware control.

---

## Common Setup (All Labs)

### Hardware (Minimum)
- ESP32 DevKit  
- 1 × LED + 220 Ω resistor  
- 1 × Potentiometer (10 kΩ) **or** LDR with resistor divider  

**Optional**
- DHT22 / DS18B20 temperature sensor  
- Servo motor (SG90)

### Software
- Arduino IDE or PlatformIO (ESP32)  
- Node-RED + Dashboard  
- MQTT Broker (Mosquitto – local or public)

### MQTT Topics (Recommended)
- **Sensor publish:** `cps/ctrl/sensor`  
- **Setpoint command:** `cps/ctrl/setpoint`  
- **Actuator command:** `cps/ctrl/act`  
- **Status / telemetry:** `cps/ctrl/status`

---

## Lab 1 — Basic Closed-Loop CPS: Setpoint Tracking (On/Off)

**Control concepts:** feedback loop, setpoint, error, closed-loop vs. open-loop  

**System:**  
Sensor (pot/LDR) → threshold controller → LED

### ESP32 Tasks
- Read analog sensor  
- Subscribe to setpoint via MQTT  
- Apply bang-bang control  
- Publish sensor value and actuator state  

### Node-RED Flow
- Dashboard slider → `cps/ctrl/setpoint`  
- Sensor chart and gauge  
- Optional status display

### Deliverables
- Dashboard screenshot  
- Sensor vs. LED state plot  
- Short discussion on closed-loop robustness  

---

## Lab 2 — Modeling & Dynamics: First-Order Plant + Step Response

**Control concepts:** plant dynamics, step response, time constant, sampling  

**System:**  
PWM LED + software low-pass filter (digital twin lite)

Plant model:
```
y[k] = α y[k−1] + (1 − α) u[k]
```

### ESP32 Tasks
- Apply PWM (0–255)  
- Compute filtered output y  
- Publish u and y  

### Node-RED Tasks
- Inject step inputs  
- Plot u and y  
- Estimate time constant

### Deliverables
- Step-response plots  
- Estimated time constant (τ) and sampling period (Ts)

---

## Lab 3 — Proportional Control (P)

**Control concepts:** proportional gain, overshoot, steady-state error  

Control law:
```
u = Kp (r − y)
```

### ESP32 Tasks
- Receive setpoint  
- Compute error and control signal  
- Saturate PWM output  
- Publish r, y, e, u  

### Node-RED
- Sliders: setpoint, Kp  
- Charts: y vs r, control effort, error  

### Deliverables
- Comparison of low / medium / high Kp  
- Discussion: speed vs. oscillation trade-off  

---

## Lab 4 — PI Control with Anti-Windup

**Control concepts:** integral action, windup, anti-windup  

PI control:
```
u[k] = Kp e[k] + Ki Σ e[k]
```

### ESP32 Tasks
- Add integral term  
- Clamp integral when saturated  
- Publish integral state  

### Node-RED
- Sliders: setpoint, Kp, Ki  
- Disturbance injection button  
- Charts including integral term  

### Deliverables
- Evidence of zero steady-state error  
- Demonstration of windup and anti-windup  

---

## Lab 5 — Networked Control Systems (NCS)

**Control concepts:** delay, jitter, packet loss, network effects  

**Architecture:**  
Controller in Node-RED, plant on ESP32

### Node-RED Flow
- Delay and jitter injection  
- Optional packet loss  
- PI controller block  
- Dashboard for network parameters  

### Deliverables
- Experiments with:
  - No delay  
  - Fixed 200 ms delay  
  - Random delay + 10% packet loss  
- Discussion on stability degradation  

---

## Lab 6 — Constraint-Aware Control (MPC Intuition)

**Control concepts:** constraints, prediction, receding horizon  

Candidate control set:
```
u ∈ {0, 64, 128, 192, 255}
```

Cost function:
```
J = (r − ŷ)² + λ u²
```

### Node-RED
- Predict next output  
- Evaluate cost for each candidate  
- Apply optimal u  

### ESP32
- Publish current output  
- Apply received PWM  
- Publish updated output  

### Deliverables
- Comparison of λ values  
- Explanation of MPC advantages over PI  

---

## Assessment Rubric (Suggested)

| Component | Weight |
|--------|--------|
| Lab completion & demo | 40% |
| Plots & analysis | 30% |
| Control reasoning | 20% |
| Code quality & documentation | 10% |

---

## Learning Outcome
By completing this lab series, students will understand **how control theory is implemented in real CPS**, including sensing, actuation, networking, delays, and constraint handling using modern IoT tools.
