/*
  Lab: SensorML -> ESP32 (LDR Self-Describing Sensor) + MQTT + Node-RED
  --------------------------------------------------------------------
  - Parses a simplified SensorML subset (string-based tag extraction)
  - Uses samplingRateHz, scale, offset, uncertainty
  - Reads LDR via ADC and publishes self-describing JSON telemetry to MQTT
  - Publishes a compact metadata JSON (retained) at boot
  - Optional: receives actuator command from Node-RED via MQTT topic

  Libraries:
  - WiFi (built-in for ESP32 Arduino core)
  - PubSubClient (install via Arduino Library Manager)

  Topics (recommended):
  - device/ldr01/metadata  (retained, publish once at boot)
  - device/ldr01/telemetry (publish periodically)
  - device/ldr01/cmd       (optional command back to ESP32)

  Notes:
  - ADC pin: GPIO34 (input-only)
  - LED pin: GPIO2 (commonly onboard LED)
*/

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// -------------------- User Config --------------------
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";

// MQTT broker (e.g., Mosquitto on PC/RPi/VM)
const char* MQTT_HOST = "192.168.1.10";
const int   MQTT_PORT = 1883;

// MQTT topics
String TOPIC_META = "device/ldr01/metadata";
String TOPIC_TLM  = "device/ldr01/telemetry";
String TOPIC_CMD  = "device/ldr01/cmd";

// Pins
static const int LDR_PIN = 34;   // ADC input-only pin
static const int LED_PIN = 2;    // Onboard LED (varies by board)
// -----------------------------------------------------

WiFiClient espClient;
PubSubClient mqtt(espClient);

// -------------------- SensorML XML (embedded) --------------------
const char* SENSORML_XML = R"xml(
<SensorML>
  <System>
    <identifier>LDR_ESP32_01</identifier>
    <observedProperty>LightLevel</observedProperty>
    <uom>adc_counts</uom>
    <samplingRateHz>20</samplingRateHz>
    <calibration>
      <scale>1.0</scale>
      <offset>0.0</offset>
    </calibration>
    <uncertainty>0.05</uncertainty>
  </System>
</SensorML>
)xml";

// -------------------- Simple XML Tag Extraction --------------------
String getTagValue(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int start = xml.indexOf(openTag);
  if (start < 0) return "";
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end < 0) return "";
  String v = xml.substring(start, end);
  v.trim();
  return v;
}

float getTagFloat(const String& xml, const String& tag, float defaultVal) {
  String v = getTagValue(xml, tag);
  if (v.length() == 0) return defaultVal;
  return v.toFloat();
}

// -------------------- Parsed Config --------------------
String sensorId, observedProperty, uom;
float samplingRateHz = 20.0f;
float scale = 1.0f;
float offset = 0.0f;
float uncertainty = 0.05f;

unsigned long samplePeriodMs = 50;
unsigned long lastSampleMs = 0;

// Optional LED override from MQTT command
// mode: -1 = auto, 0 = force off, 1 = force on
int ledOverride = -1;

// -------------------- WiFi + MQTT --------------------
void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[WiFi] connected, IP: ");
  Serial.println(WiFi.localIP());
}

static bool payloadContainsLedCommand(const String& s, int& outLed) {
  // Very simple parser for payload like {"led":1} or {"led":0}
  int k = s.indexOf("\"led\"");
  if (k < 0) k = s.indexOf("led");
  if (k < 0) return false;

  int colon = s.indexOf(':', k);
  if (colon < 0) return false;

  // find first digit after colon
  for (int i = colon + 1; i < (int)s.length(); i++) {
    char c = s[i];
    if (c == '0' || c == '1') {
      outLed = (c == '1') ? 1 : 0;
      return true;
    }
  }
  return false;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  msg.reserve(length + 4);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  Serial.print("[MQTT] msg on ");
  Serial.print(t);
  Serial.print(" : ");
  Serial.println(msg);

  if (t == TOPIC_CMD) {
    int led = -1;
    if (payloadContainsLedCommand(msg, led)) {
      ledOverride = led ? 1 : 0;
      Serial.print("[CMD] LED override = ");
      Serial.println(ledOverride);
    } else if (msg.indexOf("auto") >= 0) {
      ledOverride = -1; // back to auto
      Serial.println("[CMD] LED override = AUTO");
    }
  }
}

void mqttConnect() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    String cid = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("[MQTT] connecting as ");
    Serial.println(cid);

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

// -------------------- Publishing --------------------
void publishMetadataRetained() {
  String meta;
  meta.reserve(220);

  meta += "{";
  meta += "\"id\":\"" + sensorId + "\",";
  meta += "\"observedProperty\":\"" + observedProperty + "\",";
  meta += "\"uom\":\"" + uom + "\",";
  meta += "\"samplingRateHz\":" + String(samplingRateHz, 2) + ",";
  meta += "\"calibration\":{";
  meta += "\"scale\":" + String(scale, 6) + ",";
  meta += "\"offset\":" + String(offset, 6);
  meta += "},";
  meta += "\"uncertainty\":" + String(uncertainty, 4);
  meta += "}";

  // retained = true so Node-RED receives metadata immediately after subscribing
  mqtt.publish(TOPIC_META.c_str(), meta.c_str(), true);
  Serial.println("[MQTT] published metadata (retained)");
}

void publishTelemetry(float value, unsigned long nowMs) {
  String msg;
  msg.reserve(180);

  msg += "{";
  msg += "\"id\":\"" + sensorId + "\",";
  msg += "\"property\":\"" + observedProperty + "\",";
  msg += "\"value\":" + String(value, 2) + ",";
  msg += "\"uom\":\"" + uom + "\",";
  msg += "\"uncertainty\":" + String(uncertainty, 4) + ",";
  msg += "\"ts_ms\":" + String(nowMs);
  msg += "}";

  mqtt.publish(TOPIC_TLM.c_str(), msg.c_str());
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ADC config
  analogReadResolution(12); // 0..4095
  // Optional (if you need wider voltage range):
  // analogSetAttenuation(ADC_11db);

  // Parse simplified SensorML
  String xml = String(SENSORML_XML);
  sensorId = getTagValue(xml, "identifier");
  observedProperty = getTagValue(xml, "observedProperty");
  uom = getTagValue(xml, "uom");

  samplingRateHz = getTagFloat(xml, "samplingRateHz", 20.0f);
  scale = getTagFloat(xml, "scale", 1.0f);
  offset = getTagFloat(xml, "offset", 0.0f);
  uncertainty = getTagFloat(xml, "uncertainty", 0.05f);

  if (sensorId.length() == 0) sensorId = "LDR_ESP32_01";
  if (observedProperty.length() == 0) observedProperty = "LightLevel";
  if (uom.length() == 0) uom = "adc_counts";
  if (samplingRateHz < 1.0f) samplingRateHz = 1.0f;

  samplePeriodMs = (unsigned long)(1000.0f / samplingRateHz);

  Serial.println("=== SensorML Parsed Config ===");
  Serial.print("identifier: "); Serial.println(sensorId);
  Serial.print("observedProperty: "); Serial.println(observedProperty);
  Serial.print("uom: "); Serial.println(uom);
  Serial.print("samplingRateHz: "); Serial.println(samplingRateHz, 2);
  Serial.print("scale: "); Serial.println(scale, 6);
  Serial.print("offset: "); Serial.println(offset, 6);
  Serial.print("uncertainty: "); Serial.println(uncertainty, 4);
  Serial.print("samplePeriodMs: "); Serial.println(samplePeriodMs);
  Serial.println("================================");

  // Connect Wi-Fi and MQTT
  wifiConnect();
  mqttConnect();

  // Publish metadata once (retained)
  publishMetadataRetained();
}

void loop() {
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastSampleMs < samplePeriodMs) return;
  lastSampleMs = now;

  int raw = analogRead(LDR_PIN);         // 0..4095
  float calibrated = raw * scale + offset;

  // Auto LED behavior (optional): turn ON when "dark" (low value)
  // You can tune threshold or add it to SensorML as <darkThreshold>.
  const float darkThreshold = 1200.0f;
  bool ledAuto = (calibrated < darkThreshold);

  // Apply override if commanded via MQTT
  if (ledOverride == 1) digitalWrite(LED_PIN, HIGH);
  else if (ledOverride == 0) digitalWrite(LED_PIN, LOW);
  else digitalWrite(LED_PIN, ledAuto ? HIGH : LOW);

  // Print to Serial (self-describing JSON)
  Serial.print("{\"id\":\"");
  Serial.print(sensorId);
  Serial.print("\",\"property\":\"");
  Serial.print(observedProperty);
  Serial.print("\",\"value\":");
  Serial.print(calibrated, 2);
  Serial.print(",\"uom\":\"");
  Serial.print(uom);
  Serial.print("\",\"uncertainty\":");
  Serial.print(uncertainty, 4);
  Serial.print(",\"ts_ms\":");
  Serial.print(now);
  Serial.println("}");

  // Publish to MQTT
  publishTelemetry(calibrated, now);
}
