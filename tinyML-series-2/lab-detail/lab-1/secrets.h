#pragma once
// ======== EDIT THESE SETTINGS ========
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// MQTT broker (Mosquitto recommended)
#define MQTT_HOST       "192.168.1.10"
#define MQTT_PORT       1883

// Topics
#define MQTT_PUB_TOPIC  "tinymlcps/lab1/telemetry"
#define MQTT_SUB_TOPIC  "tinymlcps/lab1/cmd"   // reserved

// Device identity
#define DEVICE_ID       "ESP32_TinyMLCPS_01"
