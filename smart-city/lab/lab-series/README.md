# 🌆 Smart City CPS — 6-Lab Series (ESP32 + Node-RED + MQTT)

This series expands a Smart City CPS lab into **6 progressive labs**, moving from basic sensing/actuation to digital twins and predictive maintenance.

## Labs
1. **Lab 01 — Street Light Digital Twin (LDR + LED)**
2. **Lab 02 — Smart Traffic Counter (IR/Ultrasonic)**
3. **Lab 03 — Air Quality CPS Node (MQ Sensor + Alert)**
4. **Lab 04 — Flood Monitoring CPS Node (Water Level + Pump)**
5. **Lab 05 — Smart Energy CPS Node (Current Monitor + Load Control)**
6. **Lab 06 — Predictive Maintenance (Vibration RMS + Anomaly Alert)**

## Common Requirements
- ESP32 + Arduino IDE + PubSubClient
- Node-RED + node-red-dashboard
- MQTT broker (Mosquitto recommended)

## How to Use (repeat for each lab)
1. Open the lab folder (e.g., `Lab01_StreetLight_DT/`)
2. Upload the `.ino` to ESP32 (set Wi-Fi + broker)
3. Import the Node-RED `.json` flow
4. Set broker host in the MQTT config node, Deploy
5. Open dashboard at `http://<node-red-host>:1880/ui`

## Topics Convention
Each lab uses its own topic prefix: `smartcity/<domain>/...`
