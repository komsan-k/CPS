/*
  ESP32 LDR → MQTT (JSON) + optional LED control

  Publishes JSON like:
  {"device":"esp32-ldr-01","ts":1700000000000,"adc":1234,"v":0.99,"lux_index":456.7,"lux_smooth":440.2}

  Subscribes control topic for LED:
  payload examples: "ON", "OFF", "1", "0", "true", "false"

  Libraries (install via Arduino Library Manager):
  - PubSubClient (by Nick O'Leary)
  - ArduinoJson (by Benoit Blanchon)

  Notes:
  - This code produces "lux_index" (relative lux). For true lux, you must calibrate your LDR circuit.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// -------------------- WiFi --------------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -------------------- MQTT --------------------
const char* MQTT_HOST     = "broker.hivemq.com";   // your broker IP / hostname
const uint16_t MQTT_PORT  = 1883;

const char* MQTT_USER     = "";               // optional
const char* MQTT_PASS     = "";               // optional

// Topics
const char* TOPIC_LDR_PUB   = "cps/ldr/data";
const char* TOPIC_LED_SUB   = "cps/led/cmd";
const char* TOPIC_STATUS    = "cps/ldr/status";

// Client ID
const char* DEVICE_ID = "esp32-ldr-01";

// -------------------- Pins --------------------
const int PIN_LDR_ADC = 36;     // GPIO36 (ADC1_CH0) good for analog input
const int PIN_LED     = 2;      // onboard LED on many ESP32 dev boards (change if needed)

// -------------------- Sampling --------------------
const uint32_t SAMPLE_MS = 500;     // publish every 500 ms

// -------------------- ADC / Voltage --------------------
// ESP32 ADC reference is not perfectly linear; treat voltage as approximate.
const float ADC_MAX = 4095.0f;
const float VREF    = 3.3f;

// -------------------- Simple smoothing --------------------
const float ALPHA = 0.25f;  // 0..1 (higher = more responsive, lower = smoother)
float luxSmooth = NAN;

// -------------------- LuxIndex mapping --------------------
// LDR reading is inverse-ish depending on divider wiring.
// We'll compute lux_index using a simple monotonic mapping from ADC.
// If your divider makes ADC larger in darkness, set INVERT = true.
// If ADC larger in brightness, set INVERT = false.
const bool INVERT = true;

// -------------------- MQTT objects --------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

uint32_t lastSample = 0;

// -------------------- Helpers --------------------
void setLed(bool on) {
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

bool parseOnOff(const String& s) {
  String t = s;
  t.trim();
  t.toLowerCase();
  return (t == "1" || t == "on" || t == "true" || t == "yes");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == TOPIC_LED_SUB) {
    bool on = parseOnOff(msg);
    setLed(on);

    // Publish status ack
    StaticJsonDocument<256> doc;
    doc["device"] = DEVICE_ID;
    doc["ts"] = (uint64_t)millis(); // local uptime ms (ok for status)
    doc["led"] = on ? "ON" : "OFF";
    char out[256];
    size_t n = serializeJson(doc, out, sizeof(out));
    mqtt.publish(TOPIC_STATUS, out, n, false);
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    bool ok;
    if (String(MQTT_USER).length() > 0) {
      ok = mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 1, true, "offline");
    } else {
      ok = mqtt.connect(DEVICE_ID, TOPIC_STATUS, 1, true, "offline");
    }

    if (ok) {
      mqtt.subscribe(TOPIC_LED_SUB);

      // LWT/online status
      mqtt.publish(TOPIC_STATUS, "online", true);

      // Optional: publish device info
      StaticJsonDocument<256> doc;
      doc["device"] = DEVICE_ID;
      doc["ip"] = WiFi.localIP().toString();
      doc["rssi"] = WiFi.RSSI();
      char out[256];
      size_t n = serializeJson(doc, out, sizeof(out));
      mqtt.publish(TOPIC_STATUS, out, n, false);
    } else {
      delay(1000);
    }
  }
}

// Convert ADC to a relative "lux index" (0..1000-ish)
float adcToLuxIndex(int adc) {
  float x = (float)adc / ADC_MAX; // 0..1

  // Make brightness go up when light increases
  // If INVERT=true, high ADC means darker, so invert it.
  if (INVERT) x = 1.0f - x;

  // Nonlinear curve (gamma) to emphasize low light changes
  // You can tune gamma (1.0 = linear). Typical: 1.5..3.0
  const float gamma = 2.2f;
  float y = powf(constrain(x, 0.0f, 1.0f), gamma);

  // Scale to a convenient range
  return 1000.0f * y; // "lux_index" (relative)
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_LED, OUTPUT);
  setLed(false);

  // ADC config (ESP32)
  analogReadResolution(12);             // 0..4095
  analogSetPinAttenuation(PIN_LDR_ADC, ADC_11db); // better range up to ~3.3V (approx)

  connectWiFi();
  connectMQTT();

  lastSample = millis();
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
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    int adc = analogRead(PIN_LDR_ADC);
    float v = (adc / ADC_MAX) * VREF;
    float luxIndex = adcToLuxIndex(adc);

    // Exponential smoothing
    if (isnan(luxSmooth)) luxSmooth = luxIndex;
    luxSmooth = ALPHA * luxIndex + (1.0f - ALPHA) * luxSmooth;

    // Build JSON
    StaticJsonDocument<256> doc;
    doc["device"] = DEVICE_ID;
    doc["ts"] = (uint64_t)millis();     // uptime ms; Node-RED can stamp real time if needed
    doc["adc"] = adc;
    doc["v"] = v;
    doc["lux_index"] = luxIndex;
    doc["lux_smooth"] = luxSmooth;
    doc["rssi"] = WiFi.RSSI();

    char payload[256];
    size_t n = serializeJson(doc, payload, sizeof(payload));

    // Publish
    mqtt.publish(TOPIC_LDR_PUB, payload, n, false);

    // Debug
    Serial.println(payload);
  }
}
