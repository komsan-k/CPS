# 🌆 Smart City CPS Lab Package (ESP32 + Node-RED + MQTT)

This package is a **hands-on Smart City CPS mini-project** using:

- **ESP32** (sensing + actuation)
- **MQTT** (city data bus)
- **Node-RED** (digital twin dashboard + logic)

It implements a simple urban scenario:

✅ **Street Light Digital Twin**  
- Sensor: **LDR** measures brightness (lux proxy)  
- Actuator: **LED** represents a street light  
- Dashboard: shows **lux**, **LED state**, and **health status**  
- Logic: detects **FAIL** if command says *ON* but lux stays low

---

## 1) System Architecture

```
[ESP32: LDR + LED]  →  MQTT Publish (telemetry)  →  [Node-RED Dashboard / DT]
[Node-RED Control]  →  MQTT Command (ON/OFF)     →  [ESP32 Actuator]
```

### MQTT Topics (Recommended)
- Telemetry (ESP32 → Node-RED): `smartcity/streetlight/telemetry`
- State (ESP32 → Node-RED): `smartcity/streetlight/state`
- Command (Node-RED → ESP32): `smartcity/streetlight/cmd`

Payloads are **JSON**.

---

## 2) Hardware Setup (ESP32)

### Wiring (typical)
- **LDR voltage divider**
  - LDR → 3.3V
  - 10k resistor → GND
  - Divider midpoint → **GPIO36 (ADC1_CH0)**
- **LED**
  - LED (+) → **GPIO2** (through ~220Ω resistor)
  - LED (-) → GND

> Notes  
> - GPIO36 is input-only (good for ADC).  
> - If you use a different ESP32 board, confirm ADC pins.

---

## 3) Software Requirements

### ESP32
- Arduino IDE
- Libraries:
  - `WiFi` (built-in)
  - `PubSubClient` by Nick O’Leary

### Node-RED
- Node-RED installed (local PC / server / Raspberry Pi)
- Dashboard nodes:
  - `node-red-dashboard`

### MQTT Broker
Use any of these:
- Mosquitto (local): `mqtt://localhost:1883`
- A broker on your LAN
- A public broker (for testing only)

---

## 4) Quick Start

### Step A — ESP32
1. Open **`ESP32_StreetLight_DT.ino`**
2. Edit these fields:
   - `WIFI_SSID`, `WIFI_PASS`
   - `MQTT_HOST` (broker IP / host)
3. Upload to ESP32
4. Open Serial Monitor (115200)

### Step B — Node-RED
1. Import **`NodeRED_StreetLight_DT_Flow.json`**
2. Double-click the MQTT broker config node and set your broker host/port
3. Deploy
4. Open Dashboard: `http://<node-red-host>:1880/ui`

---

## 5) How FAIL Detection Works

Node-RED keeps:
- `lastCmd` = ON/OFF from dashboard switch
- `lastLux` from ESP32 telemetry

Rule (simple):
- If `lastCmd == "ON"` and `lux < LUX_MIN_ON` for a short period → **FAIL**
- Otherwise → **NORMAL**

You can tune:
- `LUX_MIN_ON` in Node-RED function node
- Sampling rate in ESP32

---

## 6) Files in This Package

- `README.md`
- `ESP32_StreetLight_DT.ino`
- `NodeRED_StreetLight_DT_Flow.json`

---

## 7) Extensions (Optional Ideas)

1. **Smart Traffic Node**
   - Add IR sensor / ultrasonic for vehicle count
   - Show traffic density on dashboard

2. **Environmental Node**
   - Add MQ gas sensor or DHT22
   - Publish city air/temperature telemetry

3. **Prediction (CPS 4.0/5.0)**
   - Predict future lux trend (simple AR / moving average)
   - Add “predictive maintenance” alert


