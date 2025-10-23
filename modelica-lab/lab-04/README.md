# 🧩 Lab 4: Networked Cyber-Physical Systems (CPS) and Data Communication

### 🎯 Objective
Students will learn to:
1. Simulate networked control between distributed CPS components.
2. Introduce communication delay, packet loss, and sampling effects.
3. Analyze how network conditions affect CPS stability and performance.

---

## 1. Prerequisites
- Completion of Lab 3: Sensing and Control in CPS.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
In networked CPS, sensors, controllers, and actuators communicate over a network.
This introduces:
- Delay (τ)
- Packet loss (p)
- Sampling period (Ts)

---

## 3. Basic Network Channel
```modelica
model NetworkChannel
  input Real u_in;
  output Real u_out;
equation
  u_out = u_in;
end NetworkChannel;
```

---

## 4. Channel with Delay and Packet Loss
```modelica
model DelayedChannel
  input Real u_in;
  output Real u_out;
  parameter Real delay = 0.05;
  parameter Real lossProb = 0.0;
  discrete Real buffer;
initial equation
  buffer = u_in;
equation
  when sample(0, delay) then
    if random(0) > lossProb then
      buffer = u_in;
    end if;
  end when;
  u_out = pre(buffer);
end DelayedChannel;
```

---

## 5. Networked CPS Model
```modelica
model NetworkedCPS
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.5);
  PositionSensor sensor(noise=0.02);
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  ForceActuator act;
  DelayedChannel ch_sensor(delay=0.05, lossProb=0.0);
  DelayedChannel ch_actuator(delay=0.05, lossProb=0.0);
  Real x_ref = 1;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  sensor.x_true = m1.x;
  ch_sensor.u_in = sensor.x_meas;
  pid.x_ref = x_ref;
  pid.x_meas = ch_sensor.u_out;
  ch_actuator.u_in = pid.u;
  act.u = ch_actuator.u_out;
  m1.F = act.F + s1.F + d1.F;
end NetworkedCPS;
```

Run:
```modelica
simulate(NetworkedCPS, stopTime=10)
plot(m1.x)
```

✅ Observe the delayed response due to communication lag.

---

## 6. Delay and Packet Loss Exploration
### Increase Delay
```modelica
DelayedChannel ch_sensor(delay=0.2);
DelayedChannel ch_actuator(delay=0.2);
```
→ Larger overshoot and slower response.

### Add Packet Loss
```modelica
DelayedChannel ch_sensor(lossProb=0.3);
```
→ Controller receives intermittent data, causing oscillations.

---

## 7. Sampling Effects
```modelica
when sample(0, 0.1) then
  pid.x_meas = ch_sensor.u_out;
end when;
```

✅ Models discrete controller updates.

---

## 8. CPS Timing Illustration
| Stage | Domain | Delay Source | Typical Value |
|--------|---------|--------------|----------------|
| Sensor → Network | Physical→Cyber | ADC + Transmission | 5–50 ms |
| Network → Controller | Cyber | Routing/Queuing | 10–100 ms |
| Controller → Actuator | Cyber→Physical | Serialization | 5–20 ms |

Total delay τ = τ₁ + τ₂ + τ₃ affects stability.

---

## 9. Exercises
1. Increase delay to 0.5 s and check stability.
2. Set lossProb = 0.2 and observe control effort.
3. Change sample rate to 0.05 and 0.2 s.
4. Add another actuator node with separate delay channel.

---

## 10. Learning Outcomes
- Model communication delay and loss in CPS.
- Analyze latency impact on feedback control.
- Understand timing constraints for real-time control.
- Use `delay()` and `sample()` for discrete-event simulation.

---

## 11. Next Lab
**Lab 5:** Digital Twin and Real-Time Visualization of CPS.
