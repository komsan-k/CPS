# Lab Extension: MQTT + Node-RED Publishing (SensorML → ESP32 LDR)

This extension adds **MQTT publishing** from the ESP32 and a **Node-RED dashboard/flow** to visualize:
- **Sensor metadata** (SensorML subset)
- **Real-time sensor values** (self-describing JSON)
- (Optional) actuator commands back to ESP32

> Works with the base lab: **SensorML → ESP32 (LDR Self-Describing Sensor)**.

---

## 1) Architecture (Extended)

```
SensorML (subset) → ESP32 (ADC read + calibration)
               → JSON self-describing message
               → MQTT publish
               → Node-RED (subscribe + parse + dashboard)
```

---

## 2) MQTT Topics (Recommended)

Use one device ID (from SensorML) to form topics:

- Metadata (optional, send once at boot):  
  `device/ldr01/metadata`

- Real-time readings:  
  `device/ldr01/telemetry`

- (Optional) Command from Node-RED to ESP32 (actuator):  
  `device/ldr01/cmd`

> You may also use the SensorML `<identifier>` value directly as the topic suffix.

---

## 3) Message Formats

### 3.1 Metadata Message (send once)
Publish a compact “SensorML subset” JSON to help Node-RED/other systems understand the device.

Example (`device/ldr01/metadata`):
```json
{
  "id": "LDR_ESP32_01",
  "observedProperty": "LightLevel",
  "uom": "adc_counts",
  "samplingRateHz": 20,
  "calibration": { "scale": 1.0, "offset": 0.0 },
  "uncertainty": 0.05
}
```

### 3.2 Telemetry Message (publish periodically)
Example (`device/ldr01/telemetry`):
```json
{
  "id":"LDR_ESP32_01",
  "property":"LightLevel",
  "value": 1780.25,
  "uom":"adc_counts",
  "uncertainty":0.05,
  "ts_ms": 123456
}
```

---

## 4) ESP32 Arduino Code (MQTT Add-on)

### 4.1 Libraries
Install:
- **PubSubClient** (MQTT)
- ESP32 Wi-Fi is built-in (Arduino ESP32 core)

Arduino IDE → Library Manager → search **PubSubClient**.

### 4.2 Code Snippet (Drop-in Additions)

Add these includes:
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
```

Add configuration (edit to your network/broker):
```cpp
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";

// Example broker: Mosquitto on PC / VM / RPi
const char* MQTT_HOST = "192.168.1.10";
const int   MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

String TOPIC_META = "device/ldr01/metadata";
String TOPIC_TLM  = "device/ldr01/telemetry";
String TOPIC_CMD  = "device/ldr01/cmd";
```

Wi-Fi + MQTT connect helpers:
```cpp
void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] connected");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Optional: handle actuator command from Node-RED
  // Example payload: {"led":1}
}

void mqttConnect() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      Serial.println("[MQTT] connected");
      mqtt.subscribe(TOPIC_CMD.c_str());
    } else {
      Serial.print("[MQTT] failed, rc=");
      Serial.println(mqtt.state());
      delay(1000);
    }
  }
}
```

Publish metadata once (in `setup()` after parsing SensorML):
```cpp
String meta = "{";
meta += "\"id\":\"" + sensorId + "\",";
meta += "\"observedProperty\":\"" + observedProperty + "\",";
meta += "\"uom\":\"" + uom + "\",";
meta += "\"samplingRateHz\":" + String(samplingRateHz, 2) + ",";
meta += "\"calibration\":{\"scale\":" + String(scale, 4) + ",\"offset\":" + String(offset, 4) + "},";
meta += "\"uncertainty\":" + String(uncertainty, 4);
meta += "}";

mqtt.publish(TOPIC_META.c_str(), meta.c_str(), true); // retained
```

Publish telemetry (inside your sampling loop):
```cpp
String msg;
msg.reserve(160);
msg += "{\"id\":\"" + sensorId + "\",";
msg += "\"property\":\"" + observedProperty + "\",";
msg += "\"value\":" + String(calibrated, 2) + ",";
msg += "\"uom\":\"" + uom + "\",";
msg += "\"uncertainty\":" + String(uncertainty, 4) + ",";
msg += "\"ts_ms\":" + String(now);
msg += "}";

mqtt.publish(TOPIC_TLM.c_str(), msg.c_str());
```

Don’t forget to call these in `setup()`:
```cpp
wifiConnect();
mqttConnect();
```

And keep MQTT alive in `loop()`:
```cpp
if (!mqtt.connected()) mqttConnect();
mqtt.loop();
```

---

## 5) Node-RED Setup

### 5.1 Install Node-RED
Use:
- Node-RED Desktop / Node-RED on server / Docker

### 5.2 Install Dashboard (Optional)
In Node-RED: **Menu → Manage palette → Install**
- `node-red-dashboard`

### 5.3 Configure MQTT Broker
In Node-RED, set your broker host/port to match `MQTT_HOST` / `MQTT_PORT`.

---

## 6) Node-RED Flow (Import)

A ready-to-import flow JSON is provided with this lab:
- Subscribes to:
  - `device/ldr01/metadata`
  - `device/ldr01/telemetry`
- Parses JSON
- Shows values on a Dashboard (text + gauge + chart)

**Download the flow JSON file:**
- `NodeRED_Flow_SensorML_LDR_MQTT.json`

Import steps:
1. Node-RED → Menu → **Import**
2. Paste JSON (or upload the file)
3. Edit MQTT broker config (host/port)
4. Deploy

---

## 7) Testing Checklist

- ESP32 connects to Wi-Fi ✅
- ESP32 connects to MQTT broker ✅
- Node-RED receives **metadata (retained)** ✅
- Node-RED receives **telemetry stream** ✅
- Dashboard shows:
  - Sensor ID / unit / sampling rate
  - Real-time LDR value
  - Trend chart

---

## 8) Optional: Command Back to ESP32

Publish from Node-RED to:
- `device/ldr01/cmd`

Example payload:
```json
{ "led": 1 }
```

In ESP32 callback, parse and control LED/relay.

---

**Key Takeaway:**  
MQTT + Node-RED turns your SensorML-enabled ESP32 into a **plug-and-play CPS device** with discoverable metadata and real-time visualization.
