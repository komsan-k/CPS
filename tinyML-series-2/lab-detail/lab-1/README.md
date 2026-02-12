# 🔹 Lab 1 --- Multi-Sensor Data Acquisition

## ESP32 + MQTT + Node-RED Dashboard (LDR + LM73 + MPU6050)

In Lab 1, you will acquire **real sensor data** from three sensors and
stream it to Node-RED via MQTT.

Sensors: - **LDR** → light intensity (ADC) - **LM73** → temperature
(I²C) - **MPU6050** → acceleration (ax, ay, az) (I²C)

------------------------------------------------------------------------

## ✅ Learning Objectives

By the end of this lab, students can:

-   Read analog data from an LDR using ESP32 ADC
-   Read temperature from LM73 over I²C
-   Read acceleration from MPU6050 over I²C
-   Publish a **structured JSON payload** via MQTT
-   Visualize sensor data in a **Node-RED Dashboard**

------------------------------------------------------------------------

## 🧰 Hardware / Software

### Hardware

-   ESP32 DevKit
-   LDR + resistor (voltage divider)
-   LM73 (I²C temperature module)
-   MPU6050 (I²C motion module)
-   Jumper wires + breadboard

### Software

-   Arduino IDE
-   Node-RED + Dashboard
-   MQTT Broker (Mosquitto recommended)

------------------------------------------------------------------------

## 🔌 Wiring (Quick Guide)

### LDR (Analog)

-   Make a voltage divider: **3.3V → LDR → ADC pin → Resistor → GND**
-   Connect divider output to **GPIO34** (ADC1)

### LM73 (I²C)

-   VCC → 3.3V
-   GND → GND
-   SDA → GPIO21
-   SCL → GPIO22
-   Address often **0x48** (may vary)

### MPU6050 (I²C)

-   VCC → 3.3V
-   GND → GND
-   SDA → GPIO21
-   SCL → GPIO22
-   Address often **0x68** (0x69 if AD0=HIGH)

> ✅ If sensors don't respond, run the included **I2C Scanner** to
> confirm addresses.

------------------------------------------------------------------------

## 📡 MQTT Topics (Lab 1)

  Direction                     Topic                        Payload
  ----------------------------- ---------------------------- ----------------
  ESP32 → Node-RED              `tinymlcps/lab1/telemetry`   JSON telemetry
  Node-RED → ESP32 (optional)   `tinymlcps/lab1/cmd`         reserved

------------------------------------------------------------------------

## 📦 Telemetry JSON Format (Lab 1)

**Structured JSON Example (simplified):**

``` json
{
  "lux": 523,
  "temp": 29.4,
  "ax": 0.02,
  "ay": -0.01,
  "az": 0.98
}
```

**Actual payload used in this lab** includes metadata:

``` json
{
  "device_id": "ESP32_TinyMLCPS_01",
  "ts_ms": 123456,
  "lux_raw": 523,
  "temp_c": 29.4,
  "ax_g": 0.02,
  "ay_g": -0.01,
  "az_g": 0.98
}
```

------------------------------------------------------------------------

# Part A --- ESP32 Arduino Implementation

## 1) Install Arduino Libraries

Install from Arduino IDE → Library Manager:

-   **PubSubClient** (Nick O'Leary)
-   **ArduinoJson** (Benoit Blanchon)

## 2) Upload the Code

Open and upload: `esp32_lab1/ESP32_Lab1_Telemetry.ino`

Edit Wi-Fi + MQTT settings in: `esp32_lab1/secrets.h`

## 3) Verify Output

Open Serial Monitor (115200) and check JSON prints every 1 second.

------------------------------------------------------------------------

# Part B --- Node-RED Digital Twin Dashboard

## 1) Import the Flow

Import: `node-red/Lab1_Dashboard_Telemetry.json`

## 2) Configure MQTT Broker

Open the MQTT node and set: - Broker: `localhost` (or your broker IP) -
Port: `1883`

## 3) Deploy and View

Deploy and open: - `http://localhost:1880/ui`

Dashboard includes: - Lux (raw ADC) - Temperature (°C) - ax, ay, az
(g) - Optional live charts

------------------------------------------------------------------------

# ✅ Checkpoints (What to Demonstrate)

1.  LDR raw ADC changes when you cover/uncover the sensor
2.  LM73 temperature changes when you warm it slightly (finger touch)
3.  MPU6050 acceleration changes when you tilt the board
4.  Node-RED receives telemetry and dashboard updates live

------------------------------------------------------------------------

# 📝 Report Template (Short)

Include in your submission: - Wiring photo (or diagram) - Screenshot of
Serial Monitor output (JSON) - Screenshot of Node-RED dashboard with
live values - Short CPS mapping (3--5 sentences): - **Connection:**
multi-sensor acquisition + MQTT publish - **Cyber:** Digital Twin
dashboard visualization

------------------------------------------------------------------------

## ✅ Deliverables

Submit: - `README.md` - `esp32_lab1/ESP32_Lab1_Telemetry.ino` -
`esp32_lab1/secrets.h` - `esp32_lab1/I2C_Scanner.ino` -
`node-red/Lab1_Dashboard_Telemetry.json`
