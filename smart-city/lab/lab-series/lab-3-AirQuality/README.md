# Lab 03 — Air Quality CPS Node (MQ Sensor + Alert)

**Goal:** Monitor air quality and trigger an alert/control action.

**Sensor:** MQ-series gas sensor (analog) on **GPIO34** (example)  
**Actuator:** LED or relay as a "fan/ventilation" control

## MQTT Topics
- Telemetry: `smartcity/air/telemetry` (aqi proxy)
- State (retained): `smartcity/air/state` (fan, quality)
- Command: `smartcity/air/cmd` (ON/OFF)

## Dashboard
- Gauge + chart for AQI proxy
- Text: quality class (GOOD/MODERATE/POOR)
- Switch: fan ON/OFF

> This lab uses an **AQI proxy** (not calibrated). For real AQI, calibration is required.
