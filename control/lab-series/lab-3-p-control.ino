/*
Lab 3: Proportional Control (P)
- Reads sensor (ADC) as y
- Receives setpoint + Kp on cps/ctrl/setpoint: {"sp":0..4095,"kp":0.0..}
- Outputs PWM u to LED via ledcWrite (0..255)
- Publishes telemetry cps/ctrl/sensor: {"sp","y","e","u","kp"}
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
volatile float kp = 0.08f; // scale from ADC error to PWM

unsigned long lastMs = 0;
const unsigned long TS_MS = 50;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<160> doc;
  if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
    if (doc["sp"].is<int>()) sp = constrain(doc["sp"].as<int>(), 0, 4095);
    if (doc["kp"].is<float>()) kp = doc["kp"].as<float>();
    if (kp < 0) kp = 0;
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-lab3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
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

    float u_f = kp * (float)e;     // P control
    int u = constrain((int)lround(u_f), 0, 255);

    ledcWrite(PWM_CH, u);

    StaticJsonDocument<192> doc;
    doc["ts"] = (uint32_t)now;
    doc["sp"] = sp;
    doc["y"]  = y;
    doc["e"]  = e;
    doc["u"]  = u;
    doc["kp"] = kp;

    char buf[192];
    size_t n = serializeJson(doc, buf);
    mqtt.publish(TOPIC_SENSOR, buf, n);
  }
}
