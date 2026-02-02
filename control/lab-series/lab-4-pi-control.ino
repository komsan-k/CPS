/*
Lab 4: PI Control + Anti-windup
- Topic cps/ctrl/setpoint: {"sp":0..4095,"kp":..,"ki":..}
- Telemetry cps/ctrl/sensor: {"sp","y","e","u","kp","ki","i","sat"}
Anti-windup: stop integrating when saturated AND error drives further into saturation.
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

const char* TOPIC_SETPOINT = "cps/ctrl/setpoint";
const char* TOPIC_SENSOR   = "cps/ctrl/sensor";

volatile int sp = 2000;
volatile float kp = 0.08f;
volatile float ki = 0.002f; // integral gain per sample (TS_MS)

float i_term = 0.0f;

unsigned long lastMs = 0;
const unsigned long TS_MS = 50;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<192> doc;
  if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
    if (doc["sp"].is<int>()) sp = constrain(doc["sp"].as<int>(), 0, 4095);
    if (doc["kp"].is<float>()) kp = max(0.0f, doc["kp"].as<float>());
    if (doc["ki"].is<float>()) ki = max(0.0f, doc["ki"].as<float>());
    if (doc["resetI"].is<bool>() && doc["resetI"].as<bool>()) i_term = 0.0f;
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-lab4-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      mqtt.subscribe(TOPIC_SETPOINT);
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

    int y = analogRead(SENSOR_PIN);
    int e = sp - y;

    // Compute unsaturated control
    float p = kp * (float)e;
    float u_unsat = p + i_term;

    int u = (int)lround(u_unsat);
    bool sat = false;

    if (u > 255) { u = 255; sat = true; }
    if (u < 0)   { u = 0;   sat = true; }

    // Anti-windup condition:
    // integrate if not saturated OR if error would move u back toward range
    bool integrate = !sat;
    if (sat) {
      if (u == 255 && e < 0) integrate = true; // error reduces output
      if (u == 0   && e > 0) integrate = true; // error increases output
    }

    if (integrate) {
      i_term += ki * (float)e;
      // clamp i_term to avoid runaway
      if (i_term > 255) i_term = 255;
      if (i_term < -255) i_term = -255;
    }

    ledcWrite(PWM_CH, u);

    StaticJsonDocument<256> doc;
    doc["ts"]  = (uint32_t)now;
    doc["sp"]  = sp;
    doc["y"]   = y;
    doc["e"]   = e;
    doc["u"]   = u;
    doc["kp"]  = kp;
    doc["ki"]  = ki;
    doc["i"]   = i_term;
    doc["sat"] = sat ? 1 : 0;

    char buf[256];
    size_t n = serializeJson(doc, buf);
    mqtt.publish(TOPIC_SENSOR, buf, n);
  }
}
