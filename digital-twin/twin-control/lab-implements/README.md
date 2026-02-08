# Twin-Control Beam Steering (ESP32 + MQTT + Node-RED Dashboard)

This lab implements an **RSSI-based Twin-Control loop** for beam steering using **two ESP32 boards**, an **MQTT broker (Mosquitto)**, and a **Node-RED Dashboard**.

## What you get
- **ESP32-TX**: Beam controller (SoftAP) + MQTT publish/subscribe  
- **ESP32-RX**: RSSI sensor (connects to SoftAP) + MQTT publisher  
- **Node-RED Dashboard**: RSSI visualization + optional best-beam decision + manual lock

---

## 1) System Architecture

```
         MQTT (Mosquitto) + Node-RED Dashboard (Wi-Fi LAN)
                 ┌──────────────────────────┐
                 │  Node-RED UI + Decision  │
                 └───────────┬──────────────┘
                             │ twin/beam/lock (command)
                             │
                 ┌───────────▼──────────────┐
                 │     ESP32-TX SoftAP      │
                 │  Beam control GPIO bits  │
                 │  Publishes beam active   │
                 └──────────────────────────┘
                             ▲
                             │  Wi-Fi RSSI (virtual sensor)
                 ┌───────────┴──────────────┐
                 │      ESP32-RX Station     │
                 │  Reads WiFi.RSSI()        │
                 │  Publishes RSSI stream    │
                 └──────────────────────────┘
```

---

## 2) MQTT Topics

| Topic | Direction | Payload |
|------|-----------|---------|
| `twin/beam/active` | TX → MQTT | `{"beam":n,"ts_ms":...}` (retained) |
| `twin/rssi` | RX → MQTT | `{"beam":n,"rssi":-55,"ts_ms":...}` |
| `twin/beam/best` | RX → MQTT (optional) | `{"beam":n,"avg_rssi":...}` |
| `twin/beam/lock` | Node-RED → TX | `"5"` (string) or `{"beam":5}` |
| `twin/status` | TX/RX → MQTT | status strings |

---

## 3) Requirements

### Hardware
- 2 × ESP32 DevKit (TX + RX)
- Beam control pins on TX (example: 4 pins → 16 beams)

### Software
- Arduino IDE (or PlatformIO)
- Node-RED + Dashboard (`node-red-dashboard`)
- Mosquitto broker (local PC / Raspberry Pi / LAN server)

### Arduino library
- **PubSubClient** (Library Manager)

---

## 4) Configure your MQTT broker IP

In both ESP32 sketches, set:

```cpp
const char* MQTT_HOST = "192.168.1.100";
```

Also open the Node-RED MQTT broker config node and set the same IP.

---

## 5) ESP32 Sketches

### A) TX (Beam Controller + MQTT)
**File:** `ESP32_TX_BeamController_MQTT.ino`

- Creates SoftAP: `BeamTwinAP`
- Sweeps beams (demo)
- Publishes `twin/beam/active` (retained)
- Subscribes `twin/beam/lock` to lock on a selected beam

### B) RX (RSSI Sensor + MQTT)
**File:** `ESP32_RX_RSSI_MQTT.ino`

- Connects to `BeamTwinAP`
- Subscribes to `twin/beam/active` to know beam ID
- Reads `WiFi.RSSI()` and publishes `twin/rssi`
- Optionally publishes `twin/beam/best` as a suggestion

---

## 6) Node-RED Dashboard

**File:** `NodeRED_Flow_TwinControl_Beam_RSSI_Dashboard.json`

### Import steps
Node-RED → Menu → **Import** → Clipboard → paste the JSON.

### What the dashboard shows
- Active beam
- RSSI gauge (dBm)
- RSSI chart (series per beam)
- Best beam (decision window)
- Manual “Lock Beam ID” input (0–15)

> Auto-lock is included but disabled by default inside the “Best beam” function node (uncomment to enable).

---

## 7) Where should “best beam” be computed?
Two valid teaching setups:

- **Option A (Edge decision):** ESP32-RX publishes best beam suggestion.
- **Option B (Cloud/Node decision):** Node-RED computes best beam from RSSI stream and locks TX.

**Recommended for teaching:** Option B (transparent + easy to modify).

---

## 8) Why this is Twin-Control
| Element | Twin-Control role |
|---|---|
| RSSI | Physical sensing (virtual sensor) |
| Beam ID | Control variable |
| Decision | Cognition layer |
| Lock command | Configuration layer |
| Digital twin | Beam↔RSSI performance model (table/ML/simulation-derived) |

---

## Files
- `ESP32_TX_BeamController_MQTT.ino`
- `ESP32_RX_RSSI_MQTT.ino`
- `NodeRED_Flow_TwinControl_Beam_RSSI_Dashboard.json`
- `README_TwinControl_BeamSteering_MQTT_NodeRED.md`
