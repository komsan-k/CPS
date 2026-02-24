# Physical AI Laboratory Framework (Lab 0--Lab 5)

This laboratory sequence is aligned with:

-   CPS 5C Architecture
-   Physical AI closed-loop model
-   TinyML + ESP32
-   Node-RED + MQTT
-   Digital Twin integration

The labs incrementally build the Physical AI loop:

Sense → Represent → Predict → Decide → Act → Learn

------------------------------------------------------------------------

## Overall CPS 5C Alignment

  Lab     CPS Layer        Physical AI Stage
  ------- ---------------- -------------------------
  Lab 0   Connection       Sense
  Lab 1   Conversion       Represent
  Lab 2   Cyber            Predict
  Lab 3   Cognition        Decide
  Lab 4   Configuration    Act & Adapt
  Lab 5   Full 5C System   Closed-Loop Physical AI

------------------------------------------------------------------------

# Lab 0 -- Sensor Connection & Real-Time Acquisition

## Objective

Implement real-time sensing and establish the CPS Connection layer.

## Hardware

-   ESP32
-   LDR
-   LM73 / DS18B20 temperature sensor
-   MPU6050 (optional)
-   Breadboard and resistors

## Tasks

1.  Read sensor values via ESP32
2.  Normalize signals
3.  Publish via MQTT
4.  Visualize using Node-RED

## Key Model

x(t) = s(t) + n(t)

Normalized signal: x̂(t) = (x(t) − μ) / σ

------------------------------------------------------------------------

# Lab 1 -- Feature Extraction & Representation

## Objective

Convert raw signals into structured features.

## Tasks

1.  Compute RMS, moving average, slope
2.  Implement autoencoder (Python side)
3.  Generate latent representation

## Mathematical Model

RMS = sqrt((1/N) Σ x_i\^2)

Autoencoder: z = fθ(x) x̂ = gφ(z)

Loss: L = \|\|x − x̂\|\|²

------------------------------------------------------------------------

# Lab 2 -- Predictive Modeling (TinyML)

## Objective

Deploy a predictive model to ESP32.

## Tasks

1.  Train small MLP in TensorFlow
2.  Convert to TensorFlow Lite
3.  Deploy to ESP32
4.  Perform inference locally

## Example Architecture

1 → 8 → 3

Parameter count: (1×8 + 8) + (8×3 + 3) = 43

------------------------------------------------------------------------

# Lab 3 -- Decision Intelligence

## Objective

Implement anomaly detection and decision logic.

## Tasks

1.  Define anomaly threshold
2.  Compare rule-based vs NN-based detection
3.  Implement MQTT alerts

Decision rule:

D(x) = Normal, if L(x) \< τ Anomaly, if L(x) ≥ τ

------------------------------------------------------------------------

# Lab 4 -- Reinforcement Learning Control

## Objective

Enable adaptive control.

## Tasks

1.  Define state, action, reward
2.  Implement Q-learning
3.  Observe policy evolution

Objective: Max E\[ Σ γ\^t r_t \]

Update rule: Q(s,a) ← Q(s,a) + α\[r + γ max Q(s',a') − Q(s,a)\]

------------------------------------------------------------------------

# Lab 5 -- Full Physical AI Closed-Loop Integration

## Objective

Integrate Labs 0--4 into a complete CPS 5C Physical AI system.

Architecture:

Sensor → Feature → Prediction → Decision → Control → Feedback

Closed-loop model:

x\_{t+1} = Φ(x_t, u_t) u_t = π(f(x_t))

------------------------------------------------------------------------

## Performance Metrics

  Metric      Target
  ----------- ---------------------
  Latency     \< 100 ms
  Accuracy    \> 90%
  Energy      Reduced vs baseline
  Stability   No oscillation

------------------------------------------------------------------------

## Learning Outcomes

After completing Lab 0--5, students will be able to:

-   Implement CPS 5C architecture
-   Deploy TinyML models on edge devices
-   Build anomaly detection systems
-   Design reinforcement learning controllers
-   Develop embodied Physical AI systems

------------------------------------------------------------------------

Prepared for CPS / Physical AI / TinyML laboratory course deployment.
