# Node-RED Dashboard: Beam ID vs RSSI Plot (Twin-Control + ESP32)

This dashboard visualizes **Beam ID vs RSSI (dBm)** for an RSSI-based Twin-Control beam-steering lab.

## What you get
- **Bar chart**: Beam ID (0–15) vs RSSI (median over a rolling window)
- **Table**: per-beam RSSI median values
- **Gauges**: Best beam ID and best RSSI

---

## 1) Prerequisites
1. Node-RED installed and running
2. Install these Node-RED palettes:
   - `node-red-dashboard`
   - `node-red-node-ui-table` (for the table widget)
   - `node-red-node-mqtt` (usually already installed)

> In Node-RED: Menu → Manage palette → Install

---

## 2) Import the flow
1. Open Node-RED editor
2. Menu → Import
3. Paste the JSON from `TwinControl_Beam_RSSI_Dashboard.json`
4. Deploy

---

## 3) MQTT topics used
This flow listens to two MQTT topics:

### A) RSSI samples
Topic: `twincontrol/rssi`

Payload JSON examples:
```json
{"beam": 7, "rssi": -62}
```
or
```json
{"beam": 7, "avg": -62.5}
```

### B) Best beam selection
Topic: `twincontrol/best`

Payload JSON examples:
```json
{"best": 7, "rssi": -61}
```
or
```json
{"beam": 7, "rssi": -61}
```

---

## 4) Broker setting
The flow is configured for a **local MQTT broker**:
- Host: `127.0.0.1`
- Port: `1883`

If your broker is elsewhere (e.g., Mosquitto on another PC/RPi), edit the **Local MQTT** broker node.

---

## 5) Open the dashboard
After Deploy, open:
- `http://localhost:1880/ui`

---

## 6) Notes / tuning
- Default beams: **16** (0–15). You can change this in the function node:
  - `const beams = 16;`
- Rolling window size per beam:
  - `const windowN = 15;`
- Chart y-axis range is set to **-100 to -20 dBm**.

---

## 7) ESP32 publishing tips
From your ESP32-RX, publish RSSI samples like:
- Topic: `twincontrol/rssi`
- Payload: `{"beam":<id>,"rssi":<dbm>}`

And publish best beam after each scan:
- Topic: `twincontrol/best`
- Payload: `{"best":<id>,"rssi":<dbm>}`
