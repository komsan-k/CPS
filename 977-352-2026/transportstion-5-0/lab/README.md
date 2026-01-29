# Transportation 5.0 CPS Lab: Human-Centered Smart Intersection (ESP32 + Node-RED)

This lab builds a **closed-loop Cyber-Physical System (CPS)** for a **smart intersection** aligned with **Transportation 5.0**:
- **Human-centered** (pedestrian priority + manual policy toggles)
- **Sustainable** (eco mode reduces waste / excessive switching)
- **Explainable** (dashboard shows *why* each phase was chosen)
- **Deterministic & safe** (ESP32 executes only predefined light phases)

---

## 1) What You Will Build

### CPS loop (5C)
- **Connection:** ESP32 senses lane demand (LDR) + pedestrian request (button)
- **Conversion:** Node-RED normalizes signals into demand scores
- **Cyber:** Node-RED maintains a **Digital Twin state** (phase, timers, rolling stats)
- **Cognition:** Policy-aware controller (rules + scoring)
- **Configuration:** Node-RED sends deterministic phase commands to ESP32

---

## 2) Hardware (ESP32)

### Components
- ESP32 DevKit
- LDR + 10k resistor (voltage divider)
- Push button (pedestrian request)
- 6 LEDs + resistors (Lane A: R/Y/G, Lane B: R/Y/G)
- (Optional) 1 LED for pedestrian walk signal

### Wiring (example pins)
- **LDR**: ADC `GPIO34`
- **Ped button**: `GPIO27` to GND (uses `INPUT_PULLUP`)
- **Lane A LEDs**: R=`GPIO25`, Y=`GPIO26`, G=`GPIO33`
- **Lane B LEDs**: R=`GPIO14`, Y=`GPIO12`, G=`GPIO13`
- **Ped LED**: `GPIO32`

> If your board uses different pins, edit the constants in the Arduino code.

---

## 3) Software Requirements

- Arduino IDE + ESP32 board support
- Libraries:
  - **PubSubClient**
  - **ArduinoJson**
- Node-RED with Dashboard:
  - `node-red-dashboard`
- MQTT broker (recommended): Mosquitto

---

## 4) MQTT Topics + Payloads

### Telemetry (ESP32 → Node-RED)
- Topic: `t50/intersection/telemetry`
- JSON example:
```json
{
  "device":"esp32-intersection-01",
  "ts":1700000000,
  "laneA": 620,
  "laneB": 410,
  "ped": 1
}
```

### Commands (Node-RED → ESP32)
- Topic: `t50/intersection/cmd`
- JSON example:
```json
{
  "phase":"A_GREEN",
  "duration_ms":12000,
  "reason":"Lane A demand high; no ped request; eco mode ON",
  "policy":{"eco":true,"ped_priority":false,"max_green_ms":15000}
}
```

### Phase names (deterministic)
- `A_GREEN`, `A_YELLOW`, `B_GREEN`, `B_YELLOW`, `PED_CROSS`, `ALL_RED`

---

## 5) Node-RED Dashboard Controls

- **Eco Mode** (switch): caps green time and reduces frequent switching
- **Pedestrian Priority** (switch): serves pedestrian requests earlier
- **Phase display** (text): current phase
- **Reason** (text): explanation of decision
- **Charts**: lane A/B trend

---

## 6) Run the Lab (Step-by-Step)

1. Start MQTT broker (Mosquitto).
2. Open Node-RED.
3. Import the provided flow: `node_red_transportation5_intersection.json`
4. In Node-RED, open MQTT broker config and set:
   - Server: your broker IP (e.g., `localhost` or `192.168.x.x`)
   - Port: `1883`
5. Deploy the flow.
6. Open Dashboard:
   - `http://<node-red-host>:1880/ui`
7. Flash ESP32 with: `esp32_transportation5_intersection.ino`
8. Verify:
   - Dashboard shows live values
   - Phase changes occur
   - LEDs match commanded phases
   - Reason text updates each decision

---

## 7) Experiments (Transportation 5.0 Evaluation)

Run each mode for ~3–5 minutes and record metrics:
1) **Baseline:** Eco OFF, Ped priority OFF  
2) **Eco only:** Eco ON, Ped priority OFF  
3) **Human-centered:** Eco ON, Ped priority ON  

**Suggested metrics (proxy)**
- Pedestrian wait time (seconds)
- Phase switching frequency (switches/min)
- Unserved demand time (how long demand stays high)
- Eco proxy: fewer unnecessary greens

---

## 8) Notes on Safety & Determinism

- ESP32 **never “decides”** based on AI/LLMs.
- Node-RED controller is **rule-based + bounded** (max green, yellow/all-red).
- Every command is a **known phase** with a **fixed duration**.

---

## Files
- `node_red_transportation5_intersection.json` (Node-RED flow)
- `esp32_transportation5_intersection.ino` (ESP32 sketch)
