# Instructor Solution Guide

## Physical AI Laboratory Series (Lab 0--Lab 5)

This guide supports instructors delivering the Physical AI lab sequence
aligned with:

-   CPS 5C Architecture
-   Physical AI Closed-Loop Model
-   TinyML + ESP32
-   Node-RED + MQTT
-   Digital Twin integration

Physical AI Loop:

Sense → Represent → Predict → Decide → Act → Learn

------------------------------------------------------------------------

# Lab 0 -- Sensor Connection & Real-Time Acquisition

## Objectives

-   Establish hardware connections
-   Acquire real-time sensor data
-   Normalize signals
-   Publish via MQTT
-   Visualize in Node-RED

## Expected Output

-   Stable real-time sensor readings
-   Dashboard updating at 1--2 Hz
-   MQTT topics: cps/lab0/temperature\
    cps/lab0/lux\
    cps/lab0/vibration

## Common Errors & Fixes

  Problem             Cause                        Fix
  ------------------- ---------------------------- -------------------
  Fluctuating LDR     Missing pull-down resistor   Add 10kΩ
  ESP32 reset loop    Power instability            Use stable USB
  MQTT failure        Incorrect broker IP          Verify IP
  No dashboard data   Topic mismatch               Match topic names

------------------------------------------------------------------------

# Lab 1 -- Feature Extraction & Representation

## Objectives

-   Compute RMS and slope
-   Implement autoencoder (Python)
-   Generate latent space

## Key Equations

RMS: sqrt((1/N) Σ x_i²)

Autoencoder: z = fθ(x)\
x̂ = gφ(z)

Loss: L = \|\|x − x̂\|\|²

## Evaluation Criteria

-   Correct math implementation (30%)
-   Latent visualization (30%)
-   Interpretation quality (40%)

------------------------------------------------------------------------

# Lab 2 -- Predictive Modeling (TinyML)

## Objectives

-   Train small MLP
-   Convert to TensorFlow Lite
-   Deploy to ESP32
-   Measure inference performance

## Example Model

1 → 8 → 3\
Total parameters = 43

## Performance Targets

  Metric           Target
  ---------------- -----------
  Accuracy         \> 85%
  Inference time   \< 10 ms
  Flash usage      \< 150 KB
  RAM usage        \< 200 KB

## Common Issues

  Issue          Cause
  -------------- ------------------------
  Model crash    Tensor arena too small
  Low accuracy   Improper scaling
  Boot failure   Memory overflow

------------------------------------------------------------------------

# Lab 3 -- Decision Intelligence

## Objectives

-   Implement anomaly detection
-   Compare rule-based vs NN-based
-   Generate MQTT alerts

## Decision Rule

Normal if L(x) \< τ\
Anomaly if L(x) ≥ τ

## Target Metrics

  Metric      Target
  ----------- ---------
  Precision   \> 0.85
  Recall      \> 0.85
  F1-score    \> 0.85

Instructor Tip: Implement hysteresis thresholds to prevent alert
oscillation.

------------------------------------------------------------------------

# Lab 4 -- Reinforcement Learning Control

## Objectives

-   Define state, action, reward
-   Implement Q-learning
-   Observe convergence

## RL Objective

Max E\[ Σ γ\^t r_t \]

Q-learning update:

Q(s,a) ← Q(s,a) + α\[r + γ max Q(s',a') − Q(s,a)\]

## Evaluation Criteria

-   Correct RL setup (30%)
-   Policy convergence (30%)
-   Performance analysis (40%)

------------------------------------------------------------------------

# Lab 5 -- Full Closed-Loop Integration

## Objectives

Integrate Labs 0--4 into full CPS 5C Physical AI system.

Closed-loop model:

x\_{t+1} = Φ(x_t, u_t)\
u_t = π(f(x_t))

## Performance Targets

  Metric               Minimum
  -------------------- ----------------
  Latency              \< 100 ms
  Accuracy             \> 90%
  Stability            No oscillation
  Reward convergence   Stable

------------------------------------------------------------------------

# Final Project Grading Rubric

  Component           Weight
  ------------------- --------
  Labs 0--2           20%
  Lab 3               15%
  Lab 4               20%
  Lab 5 Integration   25%
  Technical Report    20%

------------------------------------------------------------------------

# Conceptual Mastery Questions

1.  Explain CPS 5C alignment with Physical AI.
2.  Why is representation learning critical?
3.  Compare rule-based vs NN-based decision systems.
4.  What ensures stability in RL-based control?
5.  Why is edge inference necessary?

------------------------------------------------------------------------

Prepared for CPS / Physical AI / TinyML course deployment.
