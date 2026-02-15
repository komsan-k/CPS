/*
  🌆 Smart City CPS Lab: Street Light Digital Twin (ESP32 + MQTT + Node-RED)
  - Sensor: LDR (lux proxy) on GPIO36
  - Actuator: LED on GPIO2
  - Publishes telemetry JSON to: smartcity/streetlight/telemetry
  - Publishes state JSON to:     smartcity/streetlight/state
  - Subscribes commands to:      smartcity/streetlight/cmd  ("ON"/"OFF")

  Libraries:
    - PubSubClient (Nick O'Leary)
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ========= USER CONFIG =========
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST = "YOUR_MQTT_BROKER_IP_OR_HOST";
const uint16_t MQTT_PORT = 1883;

// Topics
const char* TOPIC_TELEMETRY = "smartcity/streetlight/telemetry";
const char* TOPIC_STATE     = "smartcity/streetlight/state";
const char* TOPIC_CMD       = "smartcity/streetlight/cmd";

// Pins
static const int PIN_LDR = 36;   // ADC input-only
static const int PIN_LED = 2;    // Built-in LED on many ESP32 boards

// Sampling / publish
static const uint32_t PUBLISH_MS = 1000;

// LDR processing (simple)
static const int ADC_MAX = 4095;     // 12-bit ADC
static const float VREF = 3.3f;      // ADC reference approx

WiFiClient espClient;
PubSubClient mqtt(espClient);

String ledState = "OFF";

// Convert ADC to a simple "lux proxy" (0..1000)
// NOTE: This is not calibrated lux. It's a scaled brightness indicator.
int adcToLuxProxy(int adc) {
  // In a typical divider, brighter light -> lower LDR resistance -> ADC changes.
  // Scale to a convenient range.
  float norm = (float)adc / (float)ADC_MAX;        // 0..1
  int lux = (int)(norm * 1000.0f);                 // 0..1000
  if (lux < 0) lux = 0;
  if (lux > 1000) lux = 1000;
  return lux;
}

void publishState(int luxProxy) {
  // JSON: {"led":"ON","lux":123,"ts":1700000000}
  unsigned long ts = millis();
  String payload = "{";
  payload += "\"led\":\"" + ledState + "\",";
  payload += "\"lux\":" + String(luxProxy) + ",";
  payload += "\"ts_ms\":" + String(ts);
  payload += "}";
  mqtt.publish(TOPIC_STATE, payload.c_str(), true); // retained
}

void publishTelemetry(int luxProxy) {
  unsigned long ts = millis();
  String payload = "{";
  payload += "\"lux\":" + String(luxProxy) + ",";
  payload += "\"ts_ms\":" + String(ts);
  payload += "}";
  mqtt.publish(TOPIC_TELEMETRY, payload.c_str(), false);
}

void applyLed(const String& cmd) {
  if (cmd == "ON") {
    digitalWrite(PIN_LED, HIGH);
    ledState = "ON";
  } else {
    digitalWrite(PIN_LED, LOW);
    ledState = "OFF";
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  msg.toUpperCase();

  if (t == TOPIC_CMD) {
    if (msg == "ON" || msg == "OFF") {
      applyLed(msg);
      // publish immediately so Node-RED can see ack
      int adc = analogRead(PIN_LDR);
      int luxProxy = adcToLuxProxy(adc);
      publishState(luxProxy);
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "ESP32-StreetLight-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("MQTT connecting as ");
    Serial.println(clientId);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT connected.");
      mqtt.subscribe(TOPIC_CMD);
      // publish retained initial state
      int adc = analogRead(PIN_LDR);
      int luxProxy = adcToLuxProxy(adc);
      publishState(luxProxy);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqtt.state());
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // ADC config (optional)
  analogReadResolution(12);

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();

  mqtt.loop();

  static uint32_t lastPub = 0;
  uint32_t now = millis();
  if (now - lastPub >= PUBLISH_MS) {
    lastPub = now;

    int adc = analogRead(PIN_LDR);
    int luxProxy = adcToLuxProxy(adc);

    publishTelemetry(luxProxy);
    publishState(luxProxy);

    Serial.printf("lux=%d led=%s\n", luxProxy, ledState.c_str());
  }
}
