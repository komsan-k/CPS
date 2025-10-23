# 🧩 Lab 9: Multi-Agent Coordination in Cyber-Physical Systems (CPS)

### 🎯 Objective
Students will learn to:
1. Model multiple CPS agents that interact through shared information.
2. Implement consensus and formation control using Modelica.
3. Explore synchronization, cooperation, and decentralized decision-making principles.

---

## 1. Prerequisites
- Completion of Lab 8: Self-Healing Control.
- Familiarity with control loops, delays, and digital twins.
- Use [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
Multi-agent CPSs consist of multiple autonomous nodes working together toward a global objective.

| Concept | Description |
|----------|--------------|
| Consensus | Agents agree on a shared variable (position, velocity). |
| Formation control | Maintain geometric formation (line, triangle). |
| Leader–Follower | One agent dictates the reference for others. |

---

## 3. Basic Agent Model
```modelica
model Agent
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=1);
  Damper d1(c=0.3);
  ForceActuator act;
  PIDController pid(Kp=2, Ki=0.2, Kd=0.1);
  input Real x_ref;
  output Real x_pos;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  pid.x_ref = x_ref;
  pid.x_meas = m1.x;
  act.u = pid.u;
  m1.F = act.F + s1.F + d1.F;
  x_pos = m1.x;
end Agent;
```

---

## 4. Two-Agent Consensus
```modelica
model TwoAgentConsensus
  Agent A1;
  Agent A2;
  parameter Real k_c = 0.5;
  Real x_mean;
equation
  A1.x_ref = A1.x_pos + k_c*(A2.x_pos - A1.x_pos);
  A2.x_ref = A2.x_pos + k_c*(A1.x_pos - A2.x_pos);
  x_mean = (A1.x_pos + A2.x_pos)/2;
end TwoAgentConsensus;
```

Run:
```modelica
simulate(TwoAgentConsensus, stopTime=10)
plot({A1.x_pos, A2.x_pos, x_mean})
```

✅ Both agents converge to consensus.

---

## 5. Leader–Follower Control
```modelica
model LeaderFollower
  Agent leader;
  Agent follower;
  parameter Real k_sync = 0.7;
  Real x_lead_ref;
equation
  x_lead_ref = sin(time);
  leader.x_ref = x_lead_ref;
  follower.x_ref = leader.x_pos + k_sync*(leader.x_pos - follower.x_pos);
end LeaderFollower;
```

Run:
```modelica
simulate(LeaderFollower, stopTime=10)
plot({leader.x_pos, follower.x_pos})
```

✅ The follower tracks the leader’s oscillations.

---

## 6. Formation Control (Three Agents)
```modelica
model FormationControl
  Agent A1;
  Agent A2;
  Agent A3;
  parameter Real k_form = 0.6;
  parameter Real d = 0.5 "Desired spacing";
equation
  A1.x_ref = sin(time);
  A2.x_ref = A1.x_pos - d + k_form*(A1.x_pos - A2.x_pos + d);
  A3.x_ref = A2.x_pos - d + k_form*(A2.x_pos - A3.x_pos + d);
end FormationControl;
```

Run:
```modelica
simulate(FormationControl, stopTime=10)
plot({A1.x_pos, A2.x_pos, A3.x_pos})
```

✅ Agents maintain constant relative spacing.

---

## 7. Communication Delay
```modelica
model DelayedLink
  input Real x_in;
  output Real x_out;
  parameter Real tau = 0.2;
equation
  x_out = delay(x_in, tau);
end DelayedLink;
```

Insert between agents to simulate latency.

---

## 8. Multi-Agent Digital Twin
```modelica
model MultiAgentTwin
  FormationControl sys;
  Real e12, e23, consensus_index;
equation
  e12 = abs(sys.A1.x_pos - sys.A2.x_pos);
  e23 = abs(sys.A2.x_pos - sys.A3.x_pos);
  consensus_index = exp(-0.5*(e12 + e23));
end MultiAgentTwin;
```

Run:
```modelica
simulate(MultiAgentTwin, stopTime=10)
plot(consensus_index)
```

✅ Higher consensus index = better synchronization.

---

## 9. Fault-Tolerant Coordination
```modelica
when time > 6 then
  A3.pid.x_meas = pre(A3.pid.x_meas) + 0.2; // sensor bias
end when;
```

✅ Consensus index drops but recovers — resilient coordination.

---

## 10. Theoretical Insight
Consensus dynamics:
\[ \dot{x}_i = -\sum_{j \in N_i} a_{ij}(x_i - x_j) \]

Network topology determines convergence speed.

---

## 11. Exercises
1. Add a third follower and test scalability.
2. Introduce random network delay: `tau = 0.1 + 0.1*sin(time)`.
3. Implement different coupling weights a12 ≠ a23.
4. Log consensus index to a file.
5. Extend to 2D formation with x, y coordinates.

---

## 12. Learning Outcomes
- Model multi-agent CPS in Modelica.
- Understand consensus and formation control.
- Analyze network delay and faults on coordination.
- Quantify performance via consensus index.
- Integrate with IoT dashboard visualization.

---

## 13. Next Lab
**Lab 10:** CPS Co-Design and System Integration.
