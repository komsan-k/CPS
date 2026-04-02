# ESP32 + Agentic AI (Node-RED + LLM) + Physical Control Lab Manual

## 1. Overview

This lab demonstrates a complete **Cyber-Physical System (CPS)** integrating:

* **ESP32** for sensing and actuation
* **MQTT + Node-RED** for orchestration and dashboard
* **LLM-based Agentic AI** for reasoning and decision making
* **Physical control loop** using LED / optional servo

System loop:

**Sense → Analyze → Reason → Decide → Act**

---

## 2. Learning Objectives

* Understand CPS 5C architecture
* Build ESP32 sensor-to-actuator loop
* Use MQTT communication
* Develop Node-RED dashboard
* Integrate LLM for agentic reasoning
* Implement safe physical control

---

## 3. Hardware

* ESP32 DevKit
* LDR
* 10 kΩ resistor
* LED
* 220 Ω resistor
* Breadboard + jumper wires
* Optional push button / servo

---

## 4. Circuit

### LDR

```text
3.3V ---- LDR ----+---- GPIO34
                  |
                10kΩ
                  |
                 GND
```

### LED

```text
GPIO2 ---- 220Ω ---- LED ---- GND
```

---

## 5. MQTT Topics

### Telemetry

```json
{
  "device": "esp32_lab_01",
  "adc": 2100,
  "light_level": "medium",
  "led_state": "OFF",
  "mode": "AUTO"
}
```

Topic:

```text
cps/lab/telemetry
```

### Control

```json
{
  "action": "LED_ON"
}
```

Topic:

```text
cps/lab/cmd
```

---

## 6. Complete ESP32 MQTT Sketch

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// =============================
// Wi-Fi and MQTT configuration
// =============================
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_BROKER = "192.168.1.10";   // change to your broker IP
const int   MQTT_PORT = 1883;

const char* TOPIC_TELEMETRY = "cps/lab/telemetry";
const char* TOPIC_COMMAND   = "cps/lab/cmd";
const char* TOPIC_STATUS    = "cps/lab/status";

// =============================
// Pin configuration
// =============================
const int LDR_PIN = 34;      // analog input
const int LED_PIN = 2;       // built-in LED on many ESP32 boards
const int BUTTON_PIN = 14;   // optional button to toggle AUTO/MANUAL

// =============================
// System state
// =============================
String deviceId = "esp32_lab_01";
String mode = "AUTO";       // AUTO or MANUAL
String ledState = "OFF";    // ON or OFF
String lastAction = "BOOT";

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 2000;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// =============================
// Helper functions
// =============================
String classifyLight(int adc) {
  if (adc < 1200) return "dark";
  if (adc < 2800) return "medium";
  return "bright";
}

void setLed(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  ledState = on ? "ON" : "OFF";
}

void publishStatus(const String& message) {
  StaticJsonDocument<192> doc;
  doc["device"] = deviceId;
  doc["status"] = message;
  doc["mode"] = mode;
  doc["led_state"] = ledState;
  doc["ts"] = millis();

  char buffer[192];
  serializeJson(doc, buffer);
  mqttClient.publish(TOPIC_STATUS, buffer, true);
}

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wi-Fi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void subscribeTopics() {
  mqttClient.subscribe(TOPIC_COMMAND);
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");

    if (mqttClient.connect(deviceId.c_str())) {
      Serial.println("connected");
      subscribeTopics();
      publishStatus("MQTT_CONNECTED");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retry in 2 seconds");
      delay(2000);
    }
  }
}

void applyAction(const String& action) {
  if (action == "LED_ON") {
    setLed(true);
    lastAction = action;
  } else if (action == "LED_OFF") {
    setLed(false);
    lastAction = action;
  } else if (action == "MODE_AUTO") {
    mode = "AUTO";
    lastAction = action;
  } else if (action == "MODE_MANUAL") {
    mode = "MANUAL";
    lastAction = action;
  } else if (action == "HOLD") {
    lastAction = action;
  }
}

void handleCommandMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT message on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err) {
    Serial.println("Invalid JSON command");
    return;
  }

  String target = doc["target"] | "";
  String action = doc["action"] | "";

  // Ignore messages intended for another device if target is specified
  if (target.length() > 0 && target != deviceId) {
    return;
  }

  // Manual mode prevents agent from changing LED, except explicit mode changes
  if (mode == "MANUAL" && action != "MODE_AUTO" && action != "MODE_MANUAL") {
    Serial.println("Manual mode active: ignoring actuator command");
    return;
  }

  applyAction(action);
  publishStatus("CMD_APPLIED:" + action);
}

void publishTelemetry() {
  int adc = analogRead(LDR_PIN);
  String lightLevel = classifyLight(adc);

  StaticJsonDocument<256> doc;
  doc["device"] = deviceId;
  doc["ts"] = millis();
  doc["adc"] = adc;
  doc["light_level"] = lightLevel;
  doc["led_state"] = ledState;
  doc["mode"] = mode;
  doc["last_action"] = lastAction;
  doc["rssi"] = WiFi.RSSI();

  char buffer[256];
  serializeJson(doc, buffer);
  mqttClient.publish(TOPIC_TELEMETRY, buffer);

  Serial.print("Telemetry: ");
  Serial.println(buffer);
}

void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      if (stableButtonState == LOW) {
        if (mode == "AUTO") {
          mode = "MANUAL";
          lastAction = "BUTTON_MODE_MANUAL";
          publishStatus("BUTTON_SWITCHED_TO_MANUAL");
        } else {
          mode = "AUTO";
          lastAction = "BUTTON_MODE_AUTO";
          publishStatus("BUTTON_SWITCHED_TO_AUTO");
        }
      }
    }
  }

  lastButtonReading = reading;
}

// =============================
// Arduino setup and loop
// =============================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  setLed(false);

  analogReadResolution(12);   // ESP32 ADC: 0..4095

  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(handleCommandMessage);

  connectMQTT();
  publishStatus("BOOT_OK");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();
  handleButton();

  if (millis() - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = millis();
    publishTelemetry();
  }
}
```

### Notes

* Replace `YOUR_WIFI_NAME`, `YOUR_WIFI_PASSWORD`, and `192.168.1.10` with your actual network values.
* The sketch publishes telemetry every 2 seconds.
* It subscribes to `cps/lab/cmd` for agent or dashboard commands.
* The optional button on GPIO14 toggles between **AUTO** and **MANUAL** mode.
* In **MANUAL** mode, agent control commands for the LED are ignored until AUTO mode is restored.

---

## 7. Full Node-RED Flow JSON

Import the following flow into Node-RED. It provides:

* MQTT telemetry input
* dashboard gauge, chart, and status text
* manual LED ON / OFF buttons
* AUTO / MANUAL mode buttons
* LLM prompt construction
* HTTP request placeholder for an LLM API
* safe command filter
* MQTT command output back to ESP32

> Before deploying:
>
> * Replace `192.168.1.10` with your MQTT broker IP if needed.
> * Replace `YOUR_OPENAI_API_KEY` with your API key.
> * Adjust the HTTP request node if you use another LLM provider or a custom ChatGPT node.

```json
[
  {
    "id": "a1f0c001",
    "type": "tab",
    "label": "ESP32 Agentic AI Lab",
    "disabled": false,
    "info": "ESP32 + Node-RED + LLM + Physical Control"
  },
  {
    "id": "ui_tab_agentic",
    "type": "ui_tab",
    "name": "Agentic CPS Lab",
    "icon": "dashboard",
    "disabled": false,
    "hidden": false
  },
  {
    "id": "ui_group_status",
    "type": "ui_group",
    "name": "System Status",
    "tab": "ui_tab_agentic",
    "order": 1,
    "disp": true,
    "width": "12",
    "collapse": false
  },
  {
    "id": "ui_group_chart",
    "type": "ui_group",
    "name": "Telemetry",
    "tab": "ui_tab_agentic",
    "order": 2,
    "disp": true,
    "width": "12",
    "collapse": false
  },
  {
    "id": "ui_group_control",
    "type": "ui_group",
    "name": "Control",
    "tab": "ui_tab_agentic",
    "order": 3,
    "disp": true,
    "width": "12",
    "collapse": false
  },
  {
    "id": "mqtt_broker_agentic",
    "type": "mqtt-broker",
    "name": "Local MQTT Broker",
    "broker": "127.0.0.1",
    "port": "1883",
    "clientid": "",
    "autoConnect": true,
    "usetls": false,
    "protocolVersion": "4",
    "keepalive": "60",
    "cleansession": true,
    "autoUnsubscribe": true,
    "birthTopic": "",
    "birthQos": "0",
    "birthRetain": "false",
    "birthPayload": "",
    "closeTopic": "",
    "closeQos": "0",
    "closeRetain": "false",
    "closePayload": "",
    "willTopic": "",
    "willQos": "0",
    "willRetain": "false",
    "willPayload": ""
  },
  {
    "id": "mqtt_in_telemetry",
    "type": "mqtt in",
    "z": "a1f0c001",
    "name": "ESP32 Telemetry",
    "topic": "cps/lab/telemetry",
    "qos": "0",
    "datatype": "auto-detect",
    "broker": "mqtt_broker_agentic",
    "nl": false,
    "rap": true,
    "rh": 0,
    "inputs": 0,
    "x": 140,
    "y": 100,
    "wires": [["json_telemetry"]]
  },
  {
    "id": "mqtt_in_status",
    "type": "mqtt in",
    "z": "a1f0c001",
    "name": "ESP32 Status",
    "topic": "cps/lab/status",
    "qos": "0",
    "datatype": "auto-detect",
    "broker": "mqtt_broker_agentic",
    "nl": false,
    "rap": true,
    "rh": 0,
    "inputs": 0,
    "x": 130,
    "y": 160,
    "wires": [["json_status"]]
  },
  {
    "id": "json_telemetry",
    "type": "json",
    "z": "a1f0c001",
    "name": "Parse Telemetry",
    "property": "payload",
    "action": "obj",
    "pretty": false,
    "x": 350,
    "y": 100,
    "wires": [["telemetry_fanout", "build_prompt"]]
  },
  {
    "id": "json_status",
    "type": "json",
    "z": "a1f0c001",
    "name": "Parse Status",
    "property": "payload",
    "action": "obj",
    "pretty": false,
    "x": 340,
    "y": 160,
    "wires": [["ui_text_status"]]
  },
  {
    "id": "telemetry_fanout",
    "type": "function",
    "z": "a1f0c001",
    "name": "Prepare Dashboard Values",
    "func": "const p = msg.payload;
return [
  { payload: Number(p.adc || 0) },
  { payload: String(p.light_level || '') },
  { payload: String(p.led_state || '') },
  { payload: String(p.mode || '') },
  { payload: Number(p.adc || 0), topic: 'ADC' },
  { payload: (p.device || 'esp32') + ' | RSSI: ' + (p.rssi || 0) + ' dBm | Last: ' + (p.last_action || '') }
];",
    "outputs": 6,
    "noerr": 0,
    "initialize": "",
    "finalize": "",
    "libs": [],
    "x": 610,
    "y": 100,
    "wires": [["ui_gauge_adc"], ["ui_text_light"], ["ui_text_led"], ["ui_text_mode"], ["ui_chart_adc"], ["ui_text_info"]]
  },
  {
    "id": "ui_gauge_adc",
    "type": "ui_gauge",
    "z": "a1f0c001",
    "name": "ADC Gauge",
    "group": "ui_group_status",
    "order": 1,
    "width": 4,
    "height": 4,
    "gtype": "gage",
    "title": "LDR ADC",
    "label": "0-4095",
    "format": "{{value}}",
    "min": 0,
    "max": "4095",
    "colors": ["#00b500", "#e6e600", "#ca3838"],
    "seg1": "1200",
    "seg2": "2800",
    "x": 900,
    "y": 40,
    "wires": []
  },
  {
    "id": "ui_text_light",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_status",
    "order": 2,
    "width": 4,
    "height": 1,
    "name": "Light Level",
    "label": "Light Level",
    "format": "{{msg.payload}}",
    "layout": "row-spread",
    "x": 900,
    "y": 80,
    "wires": []
  },
  {
    "id": "ui_text_led",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_status",
    "order": 3,
    "width": 4,
    "height": 1,
    "name": "LED State",
    "label": "LED State",
    "format": "{{msg.payload}}",
    "layout": "row-spread",
    "x": 900,
    "y": 120,
    "wires": []
  },
  {
    "id": "ui_text_mode",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_status",
    "order": 4,
    "width": 4,
    "height": 1,
    "name": "Mode",
    "label": "Mode",
    "format": "{{msg.payload}}",
    "layout": "row-spread",
    "x": 900,
    "y": 160,
    "wires": []
  },
  {
    "id": "ui_chart_adc",
    "type": "ui_chart",
    "z": "a1f0c001",
    "name": "ADC Chart",
    "group": "ui_group_chart",
    "order": 1,
    "width": 12,
    "height": 5,
    "label": "LDR ADC Over Time",
    "chartType": "line",
    "legend": "true",
    "xformat": "HH:mm:ss",
    "interpolate": "linear",
    "nodata": "No data",
    "dot": false,
    "ymin": "0",
    "ymax": "4095",
    "removeOlder": "1",
    "removeOlderPoints": "",
    "removeOlderUnit": "3600",
    "cutout": 0,
    "useOneColor": false,
    "useUTC": false,
    "colors": ["#1f77b4", "#aec7e8", "#ff7f0e"],
    "outputs": 1,
    "useDifferentColor": false,
    "className": "",
    "x": 890,
    "y": 220,
    "wires": [[]]
  },
  {
    "id": "ui_text_info",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_chart",
    "order": 2,
    "width": 12,
    "height": 1,
    "name": "Device Info",
    "label": "Device Info",
    "format": "{{msg.payload}}",
    "layout": "row-left",
    "x": 890,
    "y": 260,
    "wires": []
  },
  {
    "id": "ui_text_status",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_chart",
    "order": 3,
    "width": 12,
    "height": 1,
    "name": "Status Messages",
    "label": "Status",
    "format": "{{msg.payload.status}} | Mode: {{msg.payload.mode}} | LED: {{msg.payload.led_state}}",
    "layout": "row-left",
    "x": 590,
    "y": 160,
    "wires": []
  },
  {
    "id": "build_prompt",
    "type": "function",
    "z": "a1f0c001",
    "name": "Build LLM Prompt",
    "func": "const p = msg.payload;
const prompt = `You are an agentic controller for an ESP32 lighting system.

Inputs:
- device: ${p.device}
- adc: ${p.adc}
- light_level: ${p.light_level}
- led_state: ${p.led_state}
- mode: ${p.mode}

Rules:
1. If mode is MANUAL, do not override the actuator and return HOLD.
2. If light_level is dark and mode is AUTO, recommend LED_ON.
3. If light_level is bright and mode is AUTO, recommend LED_OFF.
4. If light_level is medium, recommend HOLD.
5. Return JSON only.

Required JSON format:
{
  \"reasoning\": \"...\",
  \"decision\": \"LED_ON\" | \"LED_OFF\" | \"HOLD\"
}`;

msg.headers = {
  'Content-Type': 'application/json',
  'Authorization': 'Bearer YOUR_OPENAI_API_KEY'
};

msg.payload = {
  model: 'gpt-4.1-mini',
  input: prompt
};

msg.original_telemetry = p;
return msg;",
    "outputs": 1,
    "noerr": 0,
    "initialize": "",
    "finalize": "",
    "libs": [],
    "x": 600,
    "y": 340,
    "wires": [["http_llm"]]
  },
  {
    "id": "http_llm",
    "type": "http request",
    "z": "a1f0c001",
    "name": "LLM API Request",
    "method": "POST",
    "ret": "obj",
    "paytoqs": "ignore",
    "url": "https://api.openai.com/v1/responses",
    "tls": "",
    "persist": false,
    "proxy": "",
    "insecureHTTPParser": false,
    "authType": "",
    "senderr": false,
    "headers": [],
    "x": 860,
    "y": 340,
    "wires": [["extract_llm_json"]]
  },
  {
    "id": "extract_llm_json",
    "type": "function",
    "z": "a1f0c001",
    "name": "Extract LLM Decision",
    "func": "let text = '';
try {
  if (msg.payload.output && Array.isArray(msg.payload.output)) {
    for (const item of msg.payload.output) {
      if (item.content && Array.isArray(item.content)) {
        for (const c of item.content) {
          if (c.type === 'output_text' && c.text) text += c.text;
        }
      }
    }
  }
} catch (e) {}

if (!text) {
  text = JSON.stringify({ reasoning: 'No parsable model output', decision: 'HOLD' });
}

let decisionObj;
try {
  decisionObj = JSON.parse(text);
} catch (e) {
  decisionObj = { reasoning: text, decision: 'HOLD' };
}

msg.payload = decisionObj;
msg.original_telemetry = msg.original_telemetry || {};
return msg;",
    "outputs": 1,
    "noerr": 0,
    "initialize": "",
    "finalize": "",
    "libs": [],
    "x": 1090,
    "y": 340,
    "wires": [["ui_text_reasoning", "safe_filter"]]
  },
  {
    "id": "ui_text_reasoning",
    "type": "ui_text",
    "z": "a1f0c001",
    "group": "ui_group_chart",
    "order": 4,
    "width": 12,
    "height": 2,
    "name": "Agent Reasoning",
    "label": "Agent Reasoning",
    "format": "{{msg.payload.reasoning}} | Decision: {{msg.payload.decision}}",
    "layout": "col-center",
    "x": 1330,
    "y": 300,
    "wires": []
  },
  {
    "id": "safe_filter",
    "type": "function",
    "z": "a1f0c001",
    "name": "Safe Command Filter",
    "func": "let d = String(msg.payload.decision || 'HOLD').trim();
const allowed = ['LED_ON', 'LED_OFF', 'HOLD'];
if (!allowed.includes(d)) d = 'HOLD';

msg.payload = {
  target: 'esp32_lab_01',
  action: d
};
return msg;",
    "outputs": 1,
    "noerr": 0,
    "initialize": "",
    "finalize": "",
    "libs": [],
    "x": 1320,
    "y": 360,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "mqtt_out_cmd",
    "type": "mqtt out",
    "z": "a1f0c001",
    "name": "Send Command to ESP32",
    "topic": "cps/lab/cmd",
    "qos": "",
    "retain": "",
    "respTopic": "",
    "contentType": "",
    "userProps": "",
    "correl": "",
    "expiry": "",
    "broker": "mqtt_broker_agentic",
    "x": 1570,
    "y": 360,
    "wires": []
  },
  {
    "id": "inject_led_on",
    "type": "inject",
    "z": "a1f0c001",
    "name": "LED ON",
    "props": [{"p": "payload"}],
    "repeat": "",
    "crontab": "",
    "once": false,
    "onceDelay": 0.1,
    "topic": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"LED_ON\"}",
    "payloadType": "json",
    "x": 120,
    "y": 440,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "inject_led_off",
    "type": "inject",
    "z": "a1f0c001",
    "name": "LED OFF",
    "props": [{"p": "payload"}],
    "repeat": "",
    "crontab": "",
    "once": false,
    "onceDelay": 0.1,
    "topic": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"LED_OFF\"}",
    "payloadType": "json",
    "x": 120,
    "y": 480,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "inject_auto",
    "type": "inject",
    "z": "a1f0c001",
    "name": "MODE AUTO",
    "props": [{"p": "payload"}],
    "repeat": "",
    "crontab": "",
    "once": false,
    "onceDelay": 0.1,
    "topic": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"MODE_AUTO\"}",
    "payloadType": "json",
    "x": 130,
    "y": 520,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "inject_manual",
    "type": "inject",
    "z": "a1f0c001",
    "name": "MODE MANUAL",
    "props": [{"p": "payload"}],
    "repeat": "",
    "crontab": "",
    "once": false,
    "onceDelay": 0.1,
    "topic": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"MODE_MANUAL\"}",
    "payloadType": "json",
    "x": 140,
    "y": 560,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "ui_btn_led_on",
    "type": "ui_button",
    "z": "a1f0c001",
    "name": "UI LED ON",
    "group": "ui_group_control",
    "order": 1,
    "width": 3,
    "height": 1,
    "passthru": false,
    "label": "LED ON",
    "tooltip": "",
    "color": "",
    "bgcolor": "",
    "className": "",
    "icon": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"LED_ON\"}",
    "payloadType": "json",
    "topic": "",
    "topicType": "str",
    "x": 370,
    "y": 440,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "ui_btn_led_off",
    "type": "ui_button",
    "z": "a1f0c001",
    "name": "UI LED OFF",
    "group": "ui_group_control",
    "order": 2,
    "width": 3,
    "height": 1,
    "passthru": false,
    "label": "LED OFF",
    "tooltip": "",
    "color": "",
    "bgcolor": "",
    "className": "",
    "icon": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"LED_OFF\"}",
    "payloadType": "json",
    "topic": "",
    "topicType": "str",
    "x": 370,
    "y": 480,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "ui_btn_auto",
    "type": "ui_button",
    "z": "a1f0c001",
    "name": "UI AUTO",
    "group": "ui_group_control",
    "order": 3,
    "width": 3,
    "height": 1,
    "passthru": false,
    "label": "AUTO",
    "tooltip": "",
    "color": "",
    "bgcolor": "",
    "className": "",
    "icon": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"MODE_AUTO\"}",
    "payloadType": "json",
    "topic": "",
    "topicType": "str",
    "x": 360,
    "y": 520,
    "wires": [["mqtt_out_cmd"]]
  },
  {
    "id": "ui_btn_manual",
    "type": "ui_button",
    "z": "a1f0c001",
    "name": "UI MANUAL",
    "group": "ui_group_control",
    "order": 4,
    "width": 3,
    "height": 1,
    "passthru": false,
    "label": "MANUAL",
    "tooltip": "",
    "color": "",
    "bgcolor": "",
    "className": "",
    "icon": "",
    "payload": "{\"target\":\"esp32_lab_01\",\"action\":\"MODE_MANUAL\"}",
    "payloadType": "json",
    "topic": "",
    "topicType": "str",
    "x": 370,
    "y": 560,
    "wires": [["mqtt_out_cmd"]]
  }
]
```

---

## 8. LLM Prompt

```text
You are an intelligent lighting controller.
If room is dark -> LED_ON
If room is bright -> LED_OFF
If medium -> HOLD
Return JSON only.
```

---

## 9. Safety Filter

Only allow:

* LED_ON
* LED_OFF
* HOLD

---

## 10. Experiments

### Scenario A: Dark

Expected: LED_ON

### Scenario B: Bright

Expected: LED_OFF

### Scenario C: Medium

Expected: HOLD

### Scenario D: Manual Override

Expected: Agent does not override

---

## 11. Report Template

* Objective
* Method
* Results
* Discussion
* Conclusion

---

## 12. CPS Mapping

* Connection → ESP32 + LDR
* Conversion → ADC processing
* Cyber → Node-RED dashboard
* Cognition → LLM agent
* Configuration → LED control
