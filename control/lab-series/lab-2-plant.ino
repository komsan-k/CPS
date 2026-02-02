/*
Lab 2: Modeling & Dynamics (First-order response)
- Subscribes actuator command: cps/ctrl/act  (JSON: {"u":0..255})
- Publishes: cps/ctrl/sensor (JSON: {"u":..,"y":..,"alpha":..})
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "192.168.1.10";
const uint16_t MQTT_PORT = 1883;

static const int PWM_PIN = 2; // LED pin
static const int PWM_CH  = 0;
static const int PWM_FREQ = 5000;
static const int PWM_RES  = 8; // 0..255

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* TOPIC_ACT    = "cps/ctrl/act";
const char* TOPIC_SENSOR = "cps/ctrl/sensor";

volatile int u_cmd = 0;
float alpha = 0.92f;     // closer to 1 => slower plant
float y = 0.0f;

unsigned long lastMs = 0;
const unsigned long TS_MS = 50; // sample time

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
    if (doc["u"].is<int>()) {
      u_cmd = constrain(doc["u"].as<int>(), 0, 255);
    }
    if (doc["alpha"].is<float>()) {
      alpha = doc["alpha"].as<float>();
      if (alpha < 0.0f) alpha = 0.0f;
      if (alpha > 0.999f) alpha = 0.999f;
    }
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  while (!mqtt.connected()) {
    String cid = "esp32-lab2-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      mqtt.subscribe(TOPIC_ACT);
    } else delay(500);
  }
}

void setup() {
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

    // Apply PWM input to physical LED (optional visual)
    ledcWrite(PWM_CH, u_cmd);

    // First-order plant output (simulated)
    y = alpha * y + (1.0f - alpha) * (float)u_cmd;

    StaticJsonDocument<128> doc;
    doc["ts"] = (uint32_t)now;
    doc["u"]  = u_cmd;
    doc["y"]  = y;
    doc["alpha"] = alpha;
    char buf[128];
    size_t n = serializeJson(doc, buf);
    mqtt.publish(TOPIC_SENSOR, buf, n);
  }
}
