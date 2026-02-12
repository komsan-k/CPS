/*
  Lab 0 — CPS Communication Foundation (ESP32 + MQTT)
  Publishes system status: RSSI + uptime, with sensor placeholders.

  Required libraries:
    - PubSubClient by Nick O'Leary
    - ArduinoJson by Benoit Blanchon
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

static const uint32_t PUBLISH_PERIOD_MS = 1000;
uint32_t lastPublish = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Reserved for future labs (commands). Kept minimal for safety.
  (void)topic; (void)payload; (void)length;
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  Serial.print("MQTT connecting");
  while (!mqtt.connected()) {
    String clientId = String(DEVICE_ID) + "_" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("\nMQTT connected.");
      mqtt.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.print(".");
      delay(800);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  uint32_t now = millis();
  if (now - lastPublish >= PUBLISH_PERIOD_MS) {
    lastPublish = now;

    // Build JSON telemetry
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["ts_ms"] = (uint32_t)now;
    doc["uptime_s"] = (uint32_t)(now / 1000);
    doc["rssi_dbm"] = WiFi.RSSI();
    doc["free_heap"] = (uint32_t)ESP.getFreeHeap();

    // Sensor placeholders for next labs
    JsonObject ph = doc.createNestedObject("placeholders");
    ph["lux"] = nullptr;
    ph["temp_c"] = nullptr;
    ph["ax_g"] = nullptr;
    ph["ay_g"] = nullptr;
    ph["az_g"] = nullptr;

    char out[256];
    size_t n = serializeJson(doc, out, sizeof(out));

    mqtt.publish(MQTT_PUB_TOPIC, out, n);
    Serial.println(out);
  }
}
