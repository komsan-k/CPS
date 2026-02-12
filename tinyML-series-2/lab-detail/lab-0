# 🔹 Lab 0 --- CPS Communication Foundation

## ESP32 + MQTT + Node-RED Dashboard (Digital Twin Skeleton)

This lab establishes the **CPS communication backbone**. You will
publish system status from an ESP32 over MQTT and visualize it in a
**Node-RED Dashboard** (Digital Twin UI) with **sensor placeholders**
for future labs.

------------------------------------------------------------------------

## ✅ Learning Objectives

By the end of this lab, students can:

-   Connect ESP32 to Wi‑Fi and MQTT
-   Publish **system status telemetry**: RSSI and uptime
-   Build a Node-RED **Digital Twin Dashboard**
-   Display **placeholder fields** for sensors (Lux, Temperature, Accel)

------------------------------------------------------------------------

## 🧰 Hardware / Software

### Hardware

-   ESP32 DevKit (any ESP32 board)

### Software

-   Arduino IDE
-   Node-RED + Dashboard
-   MQTT Broker (Mosquitto recommended)

------------------------------------------------------------------------

## 📡 MQTT Topics (Lab 0)

  -------------------------------------------------------------------------
  Direction               Topic                     Payload
  ----------------------- ------------------------- -----------------------
  ESP32 → Node-RED        `tinymlcps/lab0/status`   JSON telemetry

  Node-RED → ESP32        `tinymlcps/lab0/cmd`      (reserved for future
  (optional)                                        labs)
  -------------------------------------------------------------------------

------------------------------------------------------------------------

## 📦 Telemetry JSON Format (Published by ESP32)

Example payload:

``` json
{
  "device_id": "ESP32_TinyMLCPS_01",
  "ts_ms": 123456,
  "uptime_s": 123,
  "rssi_dbm": -52,
  "free_heap": 201344,
  "placeholders": {
    "lux": null,
    "temp_c": null,
    "ax_g": null,
    "ay_g": null,
    "az_g": null
  }
}
```

> ✅ In Lab 0, sensor values are **null** placeholders.\
> ✅ In Lab 1+, you will replace placeholders with real sensor readings.

------------------------------------------------------------------------

# Part A --- ESP32 Arduino Setup

## 1) Install Arduino Libraries

In **Arduino IDE → Library Manager**, install:

-   **PubSubClient** (Nick O'Leary)
-   **ArduinoJson** (Benoit Blanchon)

(ESP32 WiFi and Wire are built-in.)

## 2) Configure `secrets.h`

Open `esp32_lab0/secrets.h` and set: - Wi‑Fi SSID / Password - MQTT
broker IP (e.g., your PC running Mosquitto) - Topics (default OK)

## 3) Upload `ESP32_Lab0_Status.ino`

-   Board: **ESP32 Dev Module**
-   Port: your COM port
-   Upload and open Serial Monitor at **115200 baud**

You should see JSON lines printed every second.

------------------------------------------------------------------------

# Part B --- Node-RED Dashboard (Digital Twin)

## 1) Install Nodes (if needed)

In Node-RED → **Menu → Manage palette** - `node-red-dashboard`

## 2) Import the Flow

Import file: `node-red/Lab0_DigitalTwin_Dashboard.json`

## 3) Configure MQTT Broker in Node-RED

Open the MQTT node configuration: - Server: `localhost` (if Mosquitto
runs on the same machine as Node-RED)\
or your broker IP (e.g., `192.168.1.10`) - Port: `1883`

## 4) Run & View Dashboard

-   Deploy flow
-   Open Dashboard: `http://localhost:1880/ui`

Dashboard shows: - Device ID - Uptime (s) - RSSI (dBm) - Free heap -
Sensor placeholders: Lux, Temp, ax/ay/az

------------------------------------------------------------------------

# ✅ Checkpoints (What to Demonstrate)

1.  ESP32 prints JSON in Serial Monitor (uptime + rssi updates)
2.  Node-RED receives MQTT messages
3.  Dashboard updates live
4.  Placeholder sensor fields show "---" (not real values yet)

------------------------------------------------------------------------

# 📝 Report Template (Short)

Include in your submission:

-   Screenshot of Arduino Serial Monitor output
-   Screenshot of Node-RED Dashboard
-   Short explanation (3--5 sentences) of how this lab maps to CPS 5C:
    -   **Connection:** Wi‑Fi + MQTT telemetry
    -   **Cyber:** Digital Twin dashboard

------------------------------------------------------------------------

## ✅ Deliverables

Submit a ZIP or folder containing: - `README.md` -
`esp32_lab0/ESP32_Lab0_Status.ino` - `esp32_lab0/secrets.h` -
`node-red/Lab0_DigitalTwin_Dashboard.json`
