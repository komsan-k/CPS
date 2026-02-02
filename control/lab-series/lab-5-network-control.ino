/*
Lab 5: Networked Control System (ESP32 as plant)
- Publishes sensor y to cps/ctrl/sensor
- Subscribes actuator u from cps/ctrl/act (JSON: {"u":0..255})
- Applies PWM u
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "192.168.1.10";
const uint16_t MQTT_PORT = 1883;

static const int SENSOR_PIN = 34;
static const int PWM_PIN = 2;
static const int PWM_CH  = 0;
static const int PWM_FREQ = 5000;
static const int PWM_RES  = 8;

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* TOPIC_SENSOR = "cps/ctrl/sensor";
const char* TOPIC_ACT    = "cps/ctrl/act";

volatile int u_cmd = 0;

unsigned long lastMs = 0;
const unsigned long TS_MS = 50;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<96> doc;
  if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
    if (doc["u"].is<int>()) u_cmd = constrain(doc["u"].as<int>(), 0, 255);
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-lab5-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      mqtt.subscribe(TOPIC_ACT);
    } else delay(500);
  }
}

void setup() {
  analogReadResolution(12);

  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWM_PIN, PWM_CH);
  ledcWrite(PWM_CH, 0);

  connectWiFi();
  connectMQTT();
  lastMs = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastMs >= TS_MS) {
    lastMs += TS_MS;

    ledcWrite(PWM_CH, u_cmd);

    int y = analogRead(SENSOR_PIN);

    StaticJsonDocument<128> doc;
    doc["ts"] = (uint32_t)now;
    doc["y"]  = y;
    doc["u"]  = u_cmd;
    char buf[128];
    size_t n = serializeJson(doc, buf);
    mqtt.publish(TOPIC_SENSOR, buf, n);
  }
}
