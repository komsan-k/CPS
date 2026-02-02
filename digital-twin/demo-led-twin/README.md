# 🌐 Lab: Digital Twin for ESP32 LED Toggle with Button Interrupt (Node-RED Integration)

## 🧩 1. Objective
This lab demonstrates how to build a **Digital Twin** for the physical ESP32 LED system using **Node-RED**.  
Students will learn to create a **real-time dashboard** that mirrors the ESP32 LED’s physical state and allows remote control through MQTT.

You will:
- Visualize the LED’s ON/OFF state in Node-RED.
- Mirror button presses as events.
- Control the LED virtually through a dashboard switch.
- Synchronize physical and digital states over MQTT.

---

## ⚙️ 2. System Overview

The setup involves two connected parts:

| **Component** | **Description** |
|----------------|------------------|
| ESP32 Board | Publishes LED state and button events; subscribes to LED command topic |
| Node-RED | Acts as the **digital twin**, visualizing LED state and allowing remote control |

---

## 🧠 3. MQTT Topic Design

| **Topic** | **Direction** | **Purpose** |
|------------|----------------|--------------|
| `iot/lab/led/state` | ESP32 → Node-RED | Publishes current LED state (`ON` / `OFF`) |
| `iot/lab/button/event` | ESP32 → Node-RED | Publishes button press events |
| `iot/lab/led/cmd` | Node-RED → ESP32 | Sends control commands (`ON` / `OFF`) |

> 🗝️ Use consistent naming for topics and avoid uppercase/lowercase mismatches.

---

## 🧰 4. Required Components
- ESP32 board (with LED + button circuit from previous lab)
- Node-RED installed (recommended via [https://nodered.org](https://nodered.org))
- MQTT broker (e.g., **Mosquitto**, running on PC or LAN)

---

## 🔌 5. Node-RED Flow Setup

### Step 1: Download and Import
Download the ready-to-use Node-RED flow file:

📦 [esp32_led_digital_twin_nodered.json](https://github.com/komsan-k/977-220/blob/main/iot/esp32/led-toggle-button/digital-twin/esp32_led_digital_twin_nodered.json)

Then import it into Node-RED:
1. Open Node-RED editor (`http://localhost:1880`)
2. Click **Menu → Import → Upload** the JSON file
3. Open the **“ESP32 LED Digital Twin”** tab
4. Edit the **MQTT Broker** node to match your host (e.g., `192.168.x.x` or `localhost`)
5. Click **Deploy**

---

### Step 2: Flow Architecture

| **Node** | **Function** |
|-----------|--------------|
| **MQTT In (iot/lab/led/state)** | Reads LED ON/OFF state |
| **MQTT In (iot/lab/button/event)** | Reads button press activity |
| **MQTT Out (iot/lab/led/cmd)** | Sends LED ON/OFF command from dashboard |
| **UI Switch** | Sends LED command (`ON` / `OFF`) |
| **UI Text & Chart** | Displays LED state and history |
| **UI Text (Button Log)** | Shows time-stamped button events |

---

### Step 3: Dashboard View
After deploying, open the dashboard via:
```
http://localhost:1880/ui
```
You’ll see:
- LED status text: *“LED State: ON/OFF”*  
- Command switch: *“LED Command”*  
- State chart: 0 = OFF, 1 = ON  
- Button log with timestamp  

---

## 💻 6. ESP32 Firmware Integration

Update your existing **“LED Toggle with Interrupt and Debounce”** sketch to include MQTT publishing/subscription logic.

### Add the following:
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "broker.hivemq.com";
//const char* MQTT_HOST = "192.168.1.10";   // Your MQTT broker IP
const int   MQTT_PORT = 1883;

const char* TOPIC_STATE = "iot/lab/led/state";
const char* TOPIC_CMD   = "iot/lab/led/cmd";
const char* TOPIC_BTN   = "iot/lab/button/event";

WiFiClient espClient;
PubSubClient mqtt(espClient);
```

### Add to Setup:
```cpp
void setupTwin() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMsg);
}

void setup() {
  // Existing setup
  setupTwin();
}
```

### Add Loop & Functions:
```cpp
void ensureMqtt() {
  while (!mqtt.connected()) {
    if (mqtt.connect("ESP32_LED_Twin")) {
      mqtt.subscribe(TOPIC_CMD);
    } else delay(1000);
  }
}

void loopTwin() {
  if (!mqtt.connected()) ensureMqtt();
  mqtt.loop();
}

void onMqttMsg(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.toUpperCase();
  if (String(topic) == TOPIC_CMD) {
    bool cmd = (msg == "ON" || msg == "1");
    digitalWrite(12, cmd);
    mqtt.publish(TOPIC_STATE, cmd ? "ON" : "OFF");
  }
}
```

Call `loopTwin();` inside your main `loop()` function.

---

Download the ready-to-use Arduino file:  
📄 [ESP32_LED_Digital_Twin.ino](https://github.com/komsan-k/977-220/blob/main/iot/esp32/led-toggle-button/digital-twin/ESP32_LED_Digital_Twin.ino)

This firmware combines **button interrupt with debounce** and **MQTT integration** to communicate with the Node-RED digital twin dashboard.

---

## 🔍 7. Testing Procedure

1. Power the ESP32 and connect to Wi-Fi.
2. Verify in Serial Monitor:
   ```
   Wi-Fi connected
   MQTT connected
   LED initial state: OFF
   ```
3. Open Node-RED dashboard and observe:
   - LED state changes in real-time when button pressed.
   - Toggle switch sends MQTT commands to ESP32.
   - Charts update automatically.

---

## 📊 8. Example Dashboard

| Component | Description |
|------------|-------------|
| 🟢 LED State | Shows current LED status (ON/OFF) |
| 🔘 LED Command Switch | Send ON/OFF command |
| 📈 Chart | Displays LED ON/OFF history |
| 🕹️ Button Log | Shows press events from ESP32 |

---

## 🧠 9. Discussion
- Node-RED creates a **real-time twin** of the ESP32 device.  
- The digital dashboard visualizes both **state feedback** and **control commands**.  
- MQTT ensures **bidirectional communication** with minimal overhead.  
- This design can be extended to **IoT twins**, **remote diagnostics**, or **cloud integration** (AWS IoT / Azure IoT Hub).

---

## 🧩 10. Exercises

1. Modify the dashboard to include an **RSSI strength indicator** from the ESP32.  
2. Add **timestamped message logging** using Node-RED “file out” node.  
3. Create an **alert** when button pressed more than 3 times in 10 seconds.  
4. Integrate **email or Telegram notification** for state changes.  
5. Extend the same digital twin model to multiple ESP32 devices.

---

## 📎 11. References
1. Node-RED Official Docs – [https://nodered.org/docs/](https://nodered.org/docs/)  
2. Eclipse Mosquitto MQTT Broker – [https://mosquitto.org](https://mosquitto.org)  
3. Espressif Systems – *ESP32 Wi-Fi & MQTT Examples*   
