/*
  Graduate Lab: ESP32 → Node-RED → Digital Twin → Policy Loop
  ESP32 Role:
   - Read LDR (analog)
   - Publish JSON telemetry to MQTT
   - Subscribe to MQTT actuator commands (AUTO / ON / OFF)
   - Apply safe actuation (rate limit + failsafe)

  Dependencies (Arduino Library Manager):
   - PubSubClient by Nick O'Leary

  Wiring (typical):
   - LDR divider output -> GPIO34 (ADC1_CH6)
   - LED -> GPIO2 (built-in LED on many ESP32 dev boards) or external LED + resistor
*/

#include <WiFi.h>
#include <PubSubClient.h>

// =========================
// Wi-Fi & MQTT CONFIG
// =========================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST = "192.168.1.10";   // e.g., Mosquitto / Node-RED machine IP
const uint16_t MQTT_PORT = 1883;

// MQTT topics
const char* TOPIC_TELEMETRY = "cps/esp32/lab01/telemetry";
const char* TOPIC_COMMAND   = "cps/esp32/lab01/command";
const char* TOPIC_STATUS    = "cps/esp32/lab01/status";

// Optional device identity
const char* DEVICE_ID = "esp32_lab01";

// =========================
// HARDWARE CONFIG
// =========================
const int PIN_LDR = 36;   // ADC1 pin recommended
const int PIN_LED = 2;    // Built-in LED on many ESP32 boards

// =========================
// SAMPLING / CONTROL CONFIG
// =========================
const uint32_t PUBLISH_INTERVAL_MS = 1000;   // telemetry interval
const uint32_t ACTUATION_MIN_GAP_MS = 300;   // rate-limit for LED changes

// Local safety thresholds (Node-RED can override decisions via commands)
int LDR_THRESHOLD = 500;   // 0..4095 (depends on your divider)

// =========================
// MODE & STATE
// =========================
enum ControlMode : uint8_t { MODE_AUTO = 0, MODE_FORCE_ON = 1, MODE_FORCE_OFF = 2 };
volatile ControlMode mode = MODE_AUTO;

bool ledState = false;
uint32_t lastPublishMs = 0;
uint32_t lastActuationMs = 0;

// A simple "last command time" for failsafe behavior (optional)
uint32_t lastCmdMs = 0;
const uint32_t COMMAND_TIMEOUT_MS = 5UL * 60UL * 1000UL; // 5 minutes

WiFiClient espClient;
PubSubClient mqtt(espClient);

// =========================
// UTIL: SAFE LED SET
// =========================
void setLedSafe(bool newState) {
  uint32_t now = millis();
  if (now - lastActuationMs < ACTUATION_MIN_GAP_MS) return; // rate limit

  ledState = newState;
  digitalWrite(PIN_LED, ledState ? HIGH : LOW);
  lastActuationMs = now;
}

// =========================
// MQTT: PUBLISH STATUS
// =========================
void publishStatus(const char* statusMsg) {
  // Minimal JSON status (no external JSON lib needed)
  char payload[256];
  uint32_t ts = (uint32_t)(millis() / 1000UL);

  snprintf(payload, sizeof(payload),
           "{\"device\":\"%s\",\"ts\":%lu,\"mode\":\"%s\",\"led\":%s,\"msg\":\"%s\"}",
           DEVICE_ID,
           (unsigned long)ts,
           (mode == MODE_AUTO) ? "AUTO" : (mode == MODE_FORCE_ON) ? "ON" : "OFF",
           ledState ? "true" : "false",
           statusMsg);

  mqtt.publish(TOPIC_STATUS, payload, true); // retained status
}

// =========================
// MQTT: COMMAND PARSER
// Supports very simple commands to keep it robust.
//
// Accepted payload examples:
// 1) {"mode":"AUTO"}
// 2) {"mode":"ON"}
// 3) {"mode":"OFF"}
// 4) {"threshold":650}  (optional; updates local LDR threshold for AUTO)
// 5) {"mode":"AUTO","threshold":600}
//
// Note: This is a light parser (string search). For graduate labs, it’s OK
// because Node-RED will generate clean JSON.
// =========================
bool jsonHasKeyValue(const String& s, const char* key, const char* value) {
  String k = String("\"") + key + String("\"");
  String v = String("\"") + value + String("\"");
  int ki = s.indexOf(k);
  if (ki < 0) return false;
  int vi = s.indexOf(v, ki);
  return (vi >= 0);
}

int jsonReadInt(const String& s, const char* key, int defaultVal) {
  String k = String("\"") + key + String("\"");
  int ki = s.indexOf(k);
  if (ki < 0) return defaultVal;

  int colon = s.indexOf(':', ki);
  if (colon < 0) return defaultVal;

  // read until comma or }
  int end1 = s.indexOf(',', colon + 1);
  int end2 = s.indexOf('}', colon + 1);
  int end = (end1 < 0) ? end2 : ((end2 < 0) ? end1 : min(end1, end2));
  if (end < 0) return defaultVal;

  String num = s.substring(colon + 1, end);
  num.trim();
  // handle possible quotes accidentally
  num.replace("\"", "");
  return num.toInt();
}

// MQTT callback
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  (void)topic;

  String msg;
  msg.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  lastCmdMs = millis();

  // Parse mode
  if (jsonHasKeyValue(msg, "mode", "AUTO")) {
    mode = MODE_AUTO;
    publishStatus("Mode set to AUTO");
  } else if (jsonHasKeyValue(msg, "mode", "ON")) {
    mode = MODE_FORCE_ON;
    setLedSafe(true);
    publishStatus("Mode set to ON (forced)");
  } else if (jsonHasKeyValue(msg, "mode", "OFF")) {
    mode = MODE_FORCE_OFF;
    setLedSafe(false);
    publishStatus("Mode set to OFF (forced)");
  }

  // Parse threshold (optional)
  int th = jsonReadInt(msg, "threshold", -1);
  if (th >= 0 && th <= 4095) {
    LDR_THRESHOLD = th;
    publishStatus("Threshold updated");
  }
}

// =========================
// WIFI + MQTT CONNECT
// =========================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    if (millis() - start > 20000) break; // 20s timeout
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = String(DEVICE_ID) + "_" + String((uint32_t)ESP.getEfuseMac(), HEX);

    // Last Will: offline status
    const char* willPayload = "{\"status\":\"offline\"}";
    if (mqtt.connect(clientId.c_str(), TOPIC_STATUS, 1, true, willPayload)) {
      mqtt.subscribe(TOPIC_COMMAND);

      // Publish online retained
      mqtt.publish(TOPIC_STATUS, "{\"status\":\"online\"}", true);
      publishStatus("Boot complete / connected");
    } else {
      delay(1000);
    }
  }
}

// =========================
// TELEMETRY PUBLISH
// =========================
void publishTelemetry(int ldrRaw, bool decisionLed) {
  char payload[256];
  uint32_t ts = (uint32_t)(millis() / 1000UL);

  // decisionLed = what AUTO would do (for explainability), even if policy overrides upstream
  snprintf(payload, sizeof(payload),
           "{\"device\":\"%s\",\"ts\":%lu,\"ldr\":%d,\"threshold\":%d,\"mode\":\"%s\",\"auto_decision_led\":%s,\"led\":%s}",
           DEVICE_ID,
           (unsigned long)ts,
           ldrRaw,
           LDR_THRESHOLD,
           (mode == MODE_AUTO) ? "AUTO" : (mode == MODE_FORCE_ON) ? "ON" : "OFF",
           decisionLed ? "true" : "false",
           ledState ? "true" : "false");

  mqtt.publish(TOPIC_TELEMETRY, payload, false);
}

// =========================
// SETUP / LOOP
// =========================
void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // ADC configuration
  analogReadResolution(12);         // 0..4095
  analogSetAttenuation(ADC_11db);   // wider range for ESP32 ADC

  Serial.begin(115200);
  delay(200);

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  uint32_t now = millis();

  // Optional failsafe: if no commands for a long time, revert to AUTO
  if (mode != MODE_AUTO && (now - lastCmdMs > COMMAND_TIMEOUT_MS)) {
    mode = MODE_AUTO;
    publishStatus("Failsafe: command timeout → AUTO");
  }

  // Read sensor
  int ldr = analogRead(PIN_LDR);

  // AUTO decision (local). Note: Node-RED can still override via MODE_FORCE_ON/OFF.
  bool autoDecisionLed = (ldr < LDR_THRESHOLD);

  // Apply local actuation based on mode
  if (mode == MODE_AUTO) {
    setLedSafe(autoDecisionLed);
  } else if (mode == MODE_FORCE_ON) {
    setLedSafe(true);
  } else { // MODE_FORCE_OFF
    setLedSafe(false);
  }

  // Publish telemetry periodically
  if (now - lastPublishMs >= PUBLISH_INTERVAL_MS) {
    lastPublishMs = now;
    publishTelemetry(ldr, autoDecisionLed);
  }
}
