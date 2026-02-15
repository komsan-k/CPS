# Lab 04 — Flood Monitoring CPS Node (Water Level + Pump)

**Goal:** Measure water level (distance) and trigger flood warnings + pump control.

**Sensor:** Ultrasonic (HC-SR04) measuring distance to water surface  
**Actuator:** LED/relay as pump control

## MQTT Topics
- Telemetry: `smartcity/flood/telemetry` (level_cm)
- State (retained): `smartcity/flood/state` (pump, risk)
- Command: `smartcity/flood/cmd` (ON/OFF)

## Risk Rule (example)
- level < 10cm → **HIGH** (near overflow)
- 10–20cm → **MED**
- > 20cm → **LOW**
