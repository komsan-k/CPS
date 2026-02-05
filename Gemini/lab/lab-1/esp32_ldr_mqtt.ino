/*
  Clean ESP32 LDR → MQTT (JSON) + LED control (error-free PubSubClient types)

  Publishes JSON to:  cps/ldr/data
  Subscribes LED cmd: cps/led/cmd   ("ON"/"OFF"/"1"/"0"/"true"/"false")
  Status topic:       cps/ldr/status

  Required libraries (Arduino Library Manager):
  - PubSubClient (Nick O'Leary)
  - ArduinoJson (Benoit Blanchon)

  Wiring (typical voltage divider):
    3V3 --- LDR ---+--- ADC(GPIO36)
                   |
                  10k
                   |
                  GND

  Notes:
  - "lux_index" is a RELATIVE brightness scale (0..1000). Calibrate if you need real lux.
  - Fixes the common compile error by publishing payload as (const uint8_t*).
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ======================= USER CONFIG =======================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST = "192.168.1.10";   // broker IP / hostname
const uint16_t MQTT_PORT = 1883;

// Leave empty if not used
const char* MQTT_USER = "";
const char* MQTT_PASS = "";

// Topics
const char* TOPIC_LDR_PUB = "cps/ldr/data";
const char* TOPIC_LED_SUB = "cps/led/cmd";
const char* TOPIC_STATUS  = "cps/ldr/status";

// Device ID (must be unique on broker)
const char* DEVICE_ID = "esp32-ldr-01";

// Pins
const int PIN_LDR_ADC = 36;  // GPIO36 (ADC1_CH0)
const int PIN_LED     = 2;   // many ESP32 dev boards have LED on GPIO2 (change if needed)

// Sampling
const uint32_t SAMPLE_MS = 500;

// LDR mapping
const bool INVERT = true;   // if ADC increases when it's darker, keep true; else set false
const float GAMMA = 2.2f;   // curve shape (1.0 = linear)
const float ALPHA = 0.25f;  // smoothing factor (0..1)

// ===========================================================

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

static float luxSmooth = NAN;
static uint32_t lastSample = 0;

// ---------- Helpers ----------
static inline void setLed(bool on) {
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

static inline bool parseOnOff(String s) {
  s.trim();
  s.toLowerCase();
  return (s == "1" || s == "on" || s == "true" || s == "yes");
}

static float adcToLuxIndex(int adc) {
  // 12-bit ADC: 0..4095
  float x = (float)adc / 4095.0f;    // 0..1
  if (INVERT) x = 1.0f - x;          // make larger = brighter (if needed)
  x = constrain(x, 0.0f, 1.0f);
  float y = powf(x, GAMMA);          // nonlinear curve
  return 1000.0f * y;                // relative brightness scale
}

static void publishStatus(const char* statusText) {
  mqtt.publish(TOPIC_STATUS, statusText, true); // retained online/offline
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload bytes -> String
  String s;
  s.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) s += (char)payload[i];

  if (String(topic) == TOPIC_LED_SUB) {
    bool on = parseOnOff(s);
    setLed(on);

    // Optional: publish JSON ack to status topic
    StaticJsonDocument<192> doc;
    doc["device"] = DEVICE_ID;
    doc["ts"] = (uint32_t)millis();
    doc["led"] = on ? "ON" : "OFF";

    char out[192];
    size_t n = serializeJson(doc, out, sizeof(out));

    mqtt.publish(TOPIC_STATUS, (const uint8_t*)out, n, false); // <-- type-safe
  }
}

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

static void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  // Last Will (offline) retained
  while (!mqtt.connected()) {
    bool ok;
    if (MQTT_USER && strlen(MQTT_USER) > 0) {
      ok = mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "offline");
    } else {
      ok = mqtt.connect(DEVICE_ID, nullptr, nullptr, TOPIC_STATUS, 1, true, "offline");
    }

    if (ok) {
      publishStatus("online");
      mqtt.subscribe(TOPIC_LED_SUB);
    } else {
      delay(1000);
    }
  }
}

// ---------- Setup / Loop ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_LED, OUTPUT);
  setLed(false);

  // ADC setup
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_LDR_ADC, ADC_11db);

  connectWiFi();
  connectMQTT();

  lastSample = millis();
}

void loop() {
  // Maintain connections
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // Publish sensor data periodically
  uint32_t now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    int adc = analogRead(PIN_LDR_ADC);
    float luxIndex = adcToLuxIndex(adc);

    // Exponential smoothing
    if (isnan(luxSmooth)) luxSmooth = luxIndex;
    luxSmooth = ALPHA * luxIndex + (1.0f - ALPHA) * luxSmooth;

    // Build JSON payload
    StaticJsonDocument<256> doc;
    doc["device"] = DEVICE_ID;
    doc["ts"] = (uint32_t)millis(); // uptime ms (Node-RED can add real timestamp)
    doc["adc"] = adc;
    doc["lux_index"] = luxIndex;
    doc["lux_smooth"] = luxSmooth;
    doc["rssi"] = WiFi.RSSI();

    char out[256];
    size_t n = serializeJson(doc, out, sizeof(out));

    // Publish with correct payload type (fix compile error)
    bool ok = mqtt.publish(TOPIC_LDR_PUB, (const uint8_t*)out, n, false);

    // Debug
    if (!ok) Serial.println("MQTT publish failed");
    Serial.println(out);
  }
}
