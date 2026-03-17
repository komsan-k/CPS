# ESP32 + TFLM INT8 → MQTT (Node-RED Digital Twin)

This project connects an ESP32 TinyML model to a **Node-RED digital twin via MQTT**.

---

## 📦 Requirements

Place these files in your Arduino sketch folder:

- `model_data_int8.h` (INT8 model C array)

### Install Arduino Libraries:
- WiFi
- PubSubClient
- TensorFlowLite_ESP32 (or equivalent TFLM)

---

## 🚀 ESP32 Arduino Sketch

Save as: `esp32_tflm_ldr_mqtt.ino`

```cpp
// (code truncated for readability in README)
// Use the full version from your source (unchanged)
```

---

## ⚙️ Configuration

Edit these:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define MQTT_HOST "192.168.1.10"
#define MQTT_PORT 1883
```

---

## 📡 MQTT Topics

| Topic | Direction | Description |
|------|----------|------------|
| cps/lab/ldr/pred | ESP32 → Node-RED | JSON prediction |
| cps/lab/ldr/cmd  | Node-RED → ESP32 | control commands |

---

## 📤 Example JSON Output

```json
{
  "ts": 123456,
  "adc": 2500,
  "x": 0.61,
  "p": [0.1, 0.8, 0.1],
  "class": "normal",
  "mode": "auto",
  "lat_us": 420
}
```

---

## 🎛️ Commands (MQTT → ESP32)

Send to topic: `cps/lab/ldr/cmd`

```
mode:auto
mode:dark
mode:normal
mode:bright
rate:200
```

---

## 🧠 Node-RED Setup

### 1. MQTT Input
- Topic: `cps/lab/ldr/pred`

### 2. JSON Node
- Convert string → object

### 3. Dashboard Widgets

Display:

- `msg.payload.class`
- `msg.payload.x`
- `msg.payload.adc`
- `msg.payload.lat_us`

Add gauges:

- `p[0]`, `p[1]`, `p[2]`

---

## 🔁 Data Flow

```
LDR Sensor → ESP32 (TFLM INT8)
            → MQTT Publish
            → Node-RED
            → Dashboard (Digital Twin)
```

---

## ✅ Features

- Real-time TinyML inference
- MQTT streaming
- Digital twin visualization
- Remote control (mode + rate)

---

## 🚀 Use Cases

- Smart lighting CPS
- AI-enabled IoT nodes
- Digital twin labs
- Edge intelligence systems

---

## 📌 Notes

- Use real dataset for best accuracy
- Increase tensor arena if needed
- Ensure MQTT broker is reachable

---

## ✅ Summary

This system demonstrates a full **Physical AI pipeline**:

**Sensor → Edge AI → Network → Digital Twin → Control**
