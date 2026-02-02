/*
Lab 1: Basic Closed-Loop CPS (On/Off control)
- Publishes sensor to: cps/ctrl/sensor
- Subscribes setpoint from: cps/ctrl/setpoint  (JSON: {"sp":1234})
- Publishes status to: cps/ctrl/status
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ====== USER CONFIG ======
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "192.168.1.10";   // Mosquitto IP
const uint16_t MQTT_PORT = 1883;
// =========================

static const int SENSOR_PIN = 34; // ADC
static const int LED_PIN    = 2;  // digital output

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* TOPIC_SENSOR   = "cps/ctrl/sensor";
const char* TOPIC_SETPOINT = "cps/ctrl/setpoint";
const char* TOPIC_STATUS   = "cps/ctrl/status";

volatile int setpoint = 2000; // ADC 0..4095 default

unsigned long lastPubMs = 0;
const unsigned long PUB_PERIOD_MS = 250;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (!err && doc["sp"].is<int>()) {
    setpoint = doc["sp"].as<int>();
    if (setpoint < 0) setpoint = 0;
    if (setpoint > 4095) setpoint = 4095;
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-lab1-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      mqtt.subscribe(TOPIC_SETPOINT);
    } else {
      delay(500);
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  analogReadResolution(12); // 0..4095

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  int y = analogRead(SENSOR_PIN);

  // Bang-bang control: LED ON when y < setpoint
  bool ledOn = (y < setpoint);
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  // Publish telemetry
  unsigned long now = millis();
  if (now - lastPubMs >= PUB_PERIOD_MS) {
    lastPubMs = now;

    StaticJsonDocument<128> sdoc;
    sdoc["ts"] = (uint32_t)now;
    sdoc["y"]  = y;
    sdoc["sp"] = setpoint;
    char buf[128];
    size_t n = serializeJson(sdoc, buf);
    mqtt.publish(TOPIC_SENSOR, buf, n);

    StaticJsonDocument<96> st;
    st["led"] = ledOn ? 1 : 0;
    char buf2[96];
    n = serializeJson(st, buf2);
    mqtt.publish(TOPIC_STATUS, buf2, n);
  }
}
