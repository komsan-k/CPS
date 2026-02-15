# Lab 05 — Smart Energy CPS Node (Current Monitor + Load Control)

**Goal:** Monitor energy usage (current proxy) and control a load using a city policy signal.

**Sensor:** ACS712 (analog) or any current/voltage sensor (proxy)  
**Actuator:** Relay/LED as load control

## MQTT Topics
- Telemetry: `smartcity/energy/telemetry` (current_a proxy)
- State (retained): `smartcity/energy/state` (load, demand)
- Command: `smartcity/energy/cmd` (ON/OFF)

## Demand Rule (example)
- Current > threshold → demand HIGH (suggest shed load)
