# 🧠 Hysteresis Mechanism in a Predictive LED Controller (Rule-Based Threshold Explanation)

This document explains how the **hysteresis mechanism** enhances a
**rule-based threshold controller** used in a Cyber-Physical System
(CPS) predictive LED lab.

------------------------------------------------------------------------

# 📌 From Rule-Based Threshold → Hysteresis Control

## 1️⃣ Basic Rule-Based Threshold (No Hysteresis)

A traditional LED controller uses a **single decision threshold**:

    IF lux < 300  → LED ON
    ELSE          → LED OFF

### Problem: Rapid Switching (Chattering)

When light fluctuates near the threshold:

    299 → ON
    301 → OFF
    298 → ON
    302 → OFF

This causes unstable behavior called **chattering**.

------------------------------------------------------------------------

## 2️⃣ Adding Hysteresis (Dual Threshold Rules)

Hysteresis introduces **two thresholds** instead of one.

  Rule                 Condition               Action
  -------------------- ----------------------- ---------
  Turn ON Threshold    lux \< LOW_THRESHOLD    LED ON
  Turn OFF Threshold   lux \> HIGH_THRESHOLD   LED OFF

Example:

    LOW_THRESHOLD  = 280
    HIGH_THRESHOLD = 320

### New Rule-Based Logic

    IF lux < 280        → LED ON
    IF lux > 320        → LED OFF
    ELSE                → Keep previous state

The range between the two thresholds is called the **hysteresis
region**.

------------------------------------------------------------------------

## 3️⃣ Why This Is Still Rule-Based

Even with hysteresis, the controller remains:

-   Deterministic
-   Logic-based
-   No Machine Learning required

Decision depends on:

-   Current lux value
-   Previous LED state

👉 This is called **Stateful Rule-Based Thresholding**.

------------------------------------------------------------------------

## 4️⃣ Predictive LED Controller Integration

In a CPS predictive controller:

1.  Future lux is estimated from trends.
2.  Hysteresis rules stabilize switching.

Concept:

    predicted_lux = trend(lux_history)

    IF predicted_lux < LOW_THRESHOLD:
           LED = ON

    IF predicted_lux > HIGH_THRESHOLD:
           LED = OFF

Hysteresis improves stability without replacing rule logic.

------------------------------------------------------------------------

## 5️⃣ Physical Meaning of the Hysteresis Curves

Typical hysteresis diagrams show two curves:

-   Increasing light → Different switching point
-   Decreasing light → Different switching point

This separation prevents oscillation.

------------------------------------------------------------------------

## 6️⃣ CPS 5C Perspective

  CPS Layer       Function
  --------------- -----------------------
  Connection      LDR sensing
  Conversion      Lux filtering
  Cyber           Trend prediction
  Cognition       Rule-based hysteresis
  Configuration   LED control

Hysteresis belongs to the **Cognition Layer**.

------------------------------------------------------------------------

## 7️⃣ Example Pseudocode

    if led_state == OFF and lux < LOW_THRESHOLD:
            led_state = ON

    else if led_state == ON and lux > HIGH_THRESHOLD:
            led_state = OFF

Decision uses both measurement and system state.

------------------------------------------------------------------------

# ✅ Summary

Hysteresis is a **dual-threshold rule-based controller** that introduces
a stability zone between ON and OFF conditions.

Instead of:

    ONE threshold → unstable switching

We use:

    TWO thresholds + memory → stable predictive control
