# Lab 06 — Predictive Maintenance (Vibration RMS + Anomaly Alert)

**Goal:** Create a maintenance CPS node that detects abnormal vibration trends (TinyML-lite).

**Sensor:** MPU6050 (I2C) — compute RMS acceleration magnitude proxy  
**Output:** Status NORMAL/WARNING/FAULT

## MQTT Topics
- Telemetry: `smartcity/maint/telemetry` (rms, avg)
- State (retained): `smartcity/maint/state` (mode, status)
- Command: `smartcity/maint/cmd` (BASELINE / ACTIVE)

## Algorithm (Simple)
- Maintain moving average baseline
- If RMS exceeds baseline by thresholds → WARNING/FAULT
