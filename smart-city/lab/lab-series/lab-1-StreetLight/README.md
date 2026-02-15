# Lab 01 — Street Light Digital Twin (LDR + LED)

**Goal:** Build a Smart City street-light CPS node with a simple **Digital Twin** dashboard.

**Sensor:** LDR on **GPIO36** (brightness proxy)  
**Actuator:** LED on **GPIO2** (street light)

## MQTT Topics
- Telemetry: `smartcity/streetlight/telemetry`
- State (retained): `smartcity/streetlight/state`
- Command: `smartcity/streetlight/cmd` (ON/OFF)

## Dashboard
- Gauge + trend chart for lux proxy
- Command switch (ON/OFF)
- Status text (NORMAL/FAIL)

## FAIL Rule (example)
If command is **ON** but lux stays below a threshold → **FAIL** (lamp fault or sensor mismatch).
