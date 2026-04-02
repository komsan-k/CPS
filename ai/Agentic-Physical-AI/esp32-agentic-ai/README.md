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

## 7. Node-RED Flow Logic

1. MQTT in → telemetry
2. JSON parse
3. Dashboard gauge + chart
4. LLM prompt function
5. HTTP request / LLM node
6. Safe command filter
7. MQTT out → ESP32

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
