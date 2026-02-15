# Lab 02 — Smart Traffic Counter (IR/Ultrasonic)

**Goal:** Measure traffic density and visualize it as a Smart City CPS node.

**Sensor Options**
- IR beam break sensor (simple vehicle count)
- Ultrasonic (HC-SR04) as a proximity counter at a gate lane

**Actuator (optional)**
- LED indicator for congestion alert

## MQTT Topics
- Telemetry: `smartcity/traffic/telemetry` (count, rate)
- State (retained): `smartcity/traffic/state` (mode, level)
- Command: `smartcity/traffic/cmd` (AUTO / MANUAL)

## Dashboard
- Gauge: traffic rate (veh/min proxy)
- Chart: rate trend
- Text: congestion level (LOW/MED/HIGH)

## Alert Rule (example)
If rate ≥ threshold → **HIGH** congestion.
