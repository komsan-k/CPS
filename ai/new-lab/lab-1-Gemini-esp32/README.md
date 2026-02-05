# 🧪 Lab-1: Gemini + ESP32 + MQTT (Node-RED Digital Twin + AI Cognition)
**From Lab-0 (Cloud AI) → Lab-1 (AI + IoT + CPS Loop)**

---

## 🎯 Lab Objective
Build a CPS mini-system where:

- **ESP32** publishes sensor data via **MQTT**
- **Node-RED** visualizes the system (Digital Twin UI)
- **Gemini** provides **cognition** (explain state, diagnose faults, recommend actions)
- **Node-RED** sends control commands back to ESP32 via MQTT (Configuration)

✅ Outcome: *ESP32 → MQTT → Node-RED → Gemini → Node-RED → MQTT → ESP32*

---

## 🧩 Architecture (CPS 5C mapping)
- **Connection:** ESP32 (sensor + actuator) + Wi‑Fi
- **Conversion:** MQTT + JSON parsing
- **Cyber:** Node-RED flow + dashboard
- **Cognition:** Gemini explanation & recommendation
- **Configuration:** Node-RED sends LED command back to ESP32

---

## 🛠️ Requirements
### Hardware
- ESP32 DevKit (any ESP32 board)
- LED + 220Ω resistor (or onboard LED if available)
- LDR + resistor (voltage divider) *(optional but recommended)*

### Software
- Arduino IDE (or PlatformIO)
- Node-RED + **node-red-dashboard**
- MQTT broker (Mosquitto recommended)

---

## 🔑 Keys & Credentials
### Gemini API Key
Create at: https://aistudio.google.com/app/apikey

### MQTT Broker
Choose one:
- **Local Mosquitto** (recommended): `localhost:1883`
- **Public broker** (quick demo): `broker.hivemq.com:1883` *(no password; not private)*

---

## 🧾 MQTT Topics (standard for this lab)
**Telemetry (ESP32 → Node-RED)**
- `lab1/esp32/telemetry`

**Command (Node-RED → ESP32)**
- `lab1/esp32/cmd/led`  (payload: `0` or `1`)

---

## 1) ESP32 Wiring
### Option A: Use external LED
- LED anode → GPIO **2** (changeable)
- LED cathode → 220Ω → GND

### Option B: Use onboard LED
Many ESP32 boards use GPIO2 as onboard LED.

### LDR (optional)
Create a voltage divider:
- 3.3V → LDR → (ADC pin) → resistor (10k) → GND
- ADC pin example: GPIO **34** (input-only ADC)

---

## 2) ESP32 Firmware
Open `ESP32_Lab1_Gemini_MQTT.ino` and set:
- Wi‑Fi SSID/PASS
- MQTT broker host
- (Optional) static client ID

Upload to ESP32. Open Serial Monitor at 115200.

ESP32 will:
- Read LDR (ADC)
- Publish JSON telemetry
- Subscribe to LED command topic

---

## 3) Node-RED Flow
Import the provided flow:
- Node-RED → Menu → **Import → Clipboard**
- Paste `flow_lab1_gemini_esp32_mqtt.json`
- Deploy

Open Dashboard:
- http://localhost:1880/ui

### Configure two nodes before running
1) **MQTT Broker** config node (host/port)
2) **Build Gemini Request** function node: put your API key OR env var

---

## 4) What Students Do (Tasks)
1. Confirm telemetry arrives on dashboard (LDR + LED status)
2. Toggle LED from dashboard (manual command → ESP32)
3. Click **“Ask Gemini”** to generate:
   - system summary
   - fault hints (e.g., LED ON but LDR too low)
   - recommended action
4. (Optional) enable rule-based fallback:
   - If LDR indicates mismatch, auto suggest turning LED off/on

---

## ✅ Expected Output
- Live telemetry shown (ADC value)
- LED responds to dashboard toggle
- Gemini returns a readable explanation and recommendation

---

## 📊 Evaluation (20 Marks)
| Component | Criteria | Marks |
|---|---|---:|
| ESP32 Telemetry | publishes JSON via MQTT reliably | 5 |
| Node-RED Receive/Parse | correct parsing + debug visibility | 4 |
| Dashboard Digital Twin | shows live status + control | 4 |
| MQTT Control Loop | LED command works both ways | 4 |
| Gemini Cognition | meaningful analysis/recommendation | 3 |
| **Total** |  | **20** |

---

## 🔐 Security Notes
- Do **not** upload API keys to GitHub.
- Prefer environment variables:
  - `GEMINI_API_KEY`
- Avoid public MQTT brokers for sensitive data.

---

## 🚀 Extensions (Optional)
- Add logging to CSV/InfluxDB
- Add “fault injection” (disconnect sensor)
- Add auto-control mode (closed-loop with safe limits)
- Add second sensor (temperature) and multi-symptom diagnosis

---

## Files in this lab package
- `README.md`
- `ESP32_Lab1_Gemini_MQTT.ino`
- `flow_lab1_gemini_esp32_mqtt.json`
