# Closed-Loop CPS (ESP32 + MQTT + Node-RED + ChatGPT Explanation)

This part implements a **safe, deterministic CPS loop**:

**ESP32 sensor → MQTT → Node-RED rule controller → MQTT actuator → ESP32 LED**
and **ChatGPT (via OpenAI Responses API) provides explanations only**.

## Files
- `esp32_closed_loop_mqtt.ino` — ESP32 publishes LDR and subscribes LED commands
- `node_red_closed_loop_flow.json` — import-ready Node-RED flow
- `cps_log.jsonl` — generated at runtime by Node-RED (JSON Lines log)

## Hardware (recommended)
- ESP32 DevKit
- LDR + resistor (voltage divider) → GPIO 34 (ADC)
- LED + resistor → GPIO 2

## MQTT Topics
- Publish (ESP32 → Node-RED): `cps/esp32/sensor`
- Subscribe (ESP32 ← Node-RED): `cps/esp32/actuator`

## Step 1 — MQTT broker
Run a broker (Mosquitto), either:
- on the same machine as Node-RED (default in flow: `127.0.0.1:1883`)
- or on another machine (change broker IP in Node-RED MQTT broker config)

## Step 2 — Node-RED setup
1. Install Node-RED Dashboard (if needed): `node-red-dashboard`
2. Import the flow:
   - Menu → Import → Clipboard → paste `node_red_closed_loop_flow.json`
3. Set OpenAI key as an environment variable for Node-RED:

### Linux/macOS (example)
```bash
export OPENAI_API_KEY="YOUR_KEY"
node-red
```

### Docker (example)
Pass env var into the container:
```bash
docker run -it -p 1880:1880 -e OPENAI_API_KEY="YOUR_KEY" nodered/node-red
```

4. Open dashboard:
- `http://<node-red-host>:1880/ui`

## Step 3 — ESP32 setup
1. In `esp32_closed_loop_mqtt.ino`, set:
   - `WIFI_SSID`, `WIFI_PASSWORD`
   - `MQTT_BROKER` (IP of broker; often same as Node-RED host)
2. Upload to ESP32.
3. Watch Serial Monitor at 115200 baud.

## Control Logic (safe)
Node-RED uses **hysteresis** to avoid flicker:
- LED ON  if `ldr < 450`
- LED OFF if `ldr > 550`

## Safety Note
- **Node-RED rules** control the LED (deterministic).
- **ChatGPT does not control hardware.** It only explains decisions for learning and operator support.

## Troubleshooting
- If dashboard shows sensor but no LED action:
  - confirm ESP32 subscribes `cps/esp32/actuator`
  - confirm broker IP/port is correct
- If AI explanation is missing:
  - ensure `OPENAI_API_KEY` is set in Node-RED environment
  - check Node-RED debug sidebar for errors

Enjoy the lab!

