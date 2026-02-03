# 🧪 CPS / IoT Lab (Student Version)
## Implementing a Digital Twin in Node-RED for On–Off LED Control Using ESP32

> **You will build a Digital Twin** that can **control** and **monitor** a physical LED connected to an ESP32 using **MQTT + Node-RED Dashboard**.

---

## 1) Learning Outcomes
By the end of this lab, you can:
- Explain what a **Digital Twin** is (in CPS/IoT context)
- Wire and program an **ESP32** to control an LED
- Use **MQTT** for bidirectional communication
- Build a **Node-RED Dashboard** as a Digital Twin UI
- (Optional extension) Add **failure detection** and **backup activation**

---

## 2) Required Materials
### Hardware
- ESP32 Dev board
- 1× LED + 220Ω resistor (Main LED)
- (Optional) 1× LED + 220Ω resistor (Backup LED)
- (Optional) LDR + 10kΩ resistor (for failure detection)
- Breadboard + jumper wires

### Software
- Arduino IDE (ESP32 board support installed)
- Node-RED
- Node-RED Dashboard (`node-red-dashboard`)
- MQTT Broker (Mosquitto recommended; public broker OK for demo)

---

## 3) System Concept
**Digital Twin loop**:

```
Node-RED Dashboard  ⇄  MQTT Broker  ⇄  ESP32  ⇄  LED (Physical)
```

---

## 4) Hardware Wiring
### Main LED (required)
- LED anode (+) → **GPIO 2**
- LED cathode (–) → **220Ω** → **GND**

### Backup LED (optional extension)
- Backup LED anode (+) → **GPIO 12**
- Backup LED cathode (–) → **220Ω** → **GND**

### LDR (optional extension)
Use a voltage divider:
- **3.3V → LDR → (GPIO36 ADC) → 10kΩ → GND**

---

## 5) MQTT Topics (Use These Exactly)
### Commands (Node-RED → ESP32)
- `led/control` : `"ON"` or `"OFF"`

### Feedback (ESP32 → Node-RED) *(recommended)*
- `led/state` : `"ON"` / `"OFF"`

### Optional extension topics
- `dt/backup/state`
- `dt/fault`
- `dt/ldr`

---

## 6) ESP32 Task (Student)
### Task A — Connect to Wi‑Fi
Fill in your Wi‑Fi SSID/password.

### Task B — Subscribe to MQTT command topic
Subscribe to:
- `led/control`

When message is:
- `"ON"` → LED HIGH
- `"OFF"` → LED LOW

### Task C — Publish LED state *(recommended)*
Publish to:
- `led/state`

#### Skeleton Code (Fill the TODOs)
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

// TODO: Wi-Fi
const char* ssid = "__________";
const char* password = "__________";

// TODO: MQTT broker
const char* mqtt_server = "__________";   // e.g., broker.hivemq.com or your local broker IP
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = 2;

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  // TODO: handle ON / OFF
  // if (message == "ON")  { ... }
  // if (message == "OFF") { ... }

  // TODO (recommended): publish led/state
  // client.publish("led/state", "...");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // TODO: connect Wi-Fi
  // WiFi.begin(...);

  // TODO: connect MQTT
  // client.setServer(...);
  // client.setCallback(callback);

  // TODO: subscribe "led/control"
}

void loop() {
  // TODO: keep MQTT alive
  client.loop();
}
```

✅ **Evidence to show**: serial monitor output + LED reacts to MQTT commands.

---

## 7) Node-RED Task (Student)
### Task D — Build Digital Twin Dashboard
Create a flow with:
- **UI Button** (ON) → MQTT OUT (`led/control`) payload `"ON"`
- **UI Button** (OFF) → MQTT OUT (`led/control`) payload `"OFF"`
- **MQTT IN** (`led/state`) → **UI Text** to show current state *(if you implemented feedback)*

### Task E — Deploy & Test
- Deploy
- Open dashboard: `http://127.0.0.1:1880/ui`
- Verify: pressing UI buttons controls the LED

✅ **Evidence to show**: dashboard screenshot + short demo video (optional).

---

## 8) Optional Extension (Bonus)
Implement one (or more):
1. **Backup LED** activates when main fails
2. **LDR perception** detects “LED ON but brightness low”
3. Publish `dt/fault` and show fault indicator on dashboard

---

## 9) Deliverables (Submit)
1. `esp32_*.ino` (your Arduino code)
2. `node_red_flow.json` (exported Node-RED flow)
3. Screenshot of dashboard
4. Short report (1–2 pages): Digital Twin explanation + architecture + results

---

## 10) Grading (100 Marks)
| Item | Description | Marks |
|---|---|---:|
| ESP32 Setup & LED Control | Correct wiring + correct ON/OFF control | 20 |
| MQTT Communication | Reliable subscribe/publish + stable connection | 20 |
| Node-RED Dashboard | Functional UI (ON/OFF) + clean layout | 25 |
| Digital Twin Concept | Clear explanation + sync between physical & digital | 20 |
| Documentation | Clear report + screenshots + flow export | 15 |
| **Total** |  | **100** |

---

## 11) Common Troubleshooting
- Dashboard only shows “Welcome” → you have **no ui nodes deployed** OR wrong Node.js version
- MQTT not connecting → wrong broker IP/port, firewall, or topic mismatch
- LED doesn’t change → check GPIO pin, resistor, and GND wiring
