/*
  Lab 1 — Multi-Sensor Data Acquisition
  ESP32 + LDR (ADC) + LM73 (I2C) + MPU6050 (I2C)
  Publishes JSON telemetry via MQTT.

  Required libraries:
    - PubSubClient by Nick O'Leary
    - ArduinoJson by Benoit Blanchon
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "secrets.h"

// ------------------- Pins -------------------
static const int PIN_LDR = 34;     // ADC1 input-only
// I2C default pins on many ESP32 devkits: SDA=21, SCL=22

// ------------------- I2C Addresses -------------------
// If your modules use different addresses, run I2C_Scanner.ino
static const uint8_t LM73_ADDR    = 0x48;  // common default
static const uint8_t MPU6050_ADDR = 0x68;  // common default

// ------------------- MQTT -------------------
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
  // Reserved for future labs (commands)
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

// ------------------- LDR -------------------
int readLdrRaw() {
  return analogRead(PIN_LDR);  // 0..4095
}

// ------------------- LM73 -------------------
bool readLm73C(float &tempC) {
  // Common LM73 temp reg 0x00, two bytes; often 14-bit value
  Wire.beginTransmission(LM73_ADDR);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission(false) != 0) return false;

  uint8_t n = Wire.requestFrom((int)LM73_ADDR, 2);
  if (n != 2) return false;

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  int16_t raw = (int16_t)((msb << 8) | lsb);

  raw >>= 2; // 14-bit
  if (raw & 0x2000) raw |= 0xC000; // sign extend

  tempC = (float)raw * 0.03125f; // 0.03125°C/LSB (common 14-bit)
  return true;
}

// ------------------- MPU6050 -------------------
bool mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return (Wire.endTransmission() == 0);
}

bool mpuReadBytes(uint8_t reg, uint8_t *buf, size_t len) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  uint8_t n = Wire.requestFrom((int)MPU6050_ADDR, (int)len);
  if (n != len) return false;
  for (size_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool readMpuAccelG(float &ax, float &ay, float &az) {
  // Accel registers 0x3B..0x40
  uint8_t b[6];
  if (!mpuReadBytes(0x3B, b, 6)) return false;

  int16_t rawX = (int16_t)((b[0] << 8) | b[1]);
  int16_t rawY = (int16_t)((b[2] << 8) | b[3]);
  int16_t rawZ = (int16_t)((b[4] << 8) | b[5]);

  const float scale = 16384.0f; // ±2g
  ax = (float)rawX / scale;
  ay = (float)rawY / scale;
  az = (float)rawZ / scale;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // ADC setup
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // I2C
  Wire.begin(); // SDA=21, SCL=22 default

  // Wake up MPU6050 (clear sleep bit)
  mpuWrite(0x6B, 0x00);

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

    // Read sensors
    int luxRaw = readLdrRaw();

    float tempC = NAN;
    bool okTemp = readLm73C(tempC);

    float ax = NAN, ay = NAN, az = NAN;
    bool okAccel = readMpuAccelG(ax, ay, az);

    // Build JSON telemetry
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["ts_ms"] = (uint32_t)now;
    doc["lux_raw"] = luxRaw;

    if (okTemp) doc["temp_c"] = tempC;
    else doc["temp_c"] = nullptr;

    if (okAccel) {
      doc["ax_g"] = ax;
      doc["ay_g"] = ay;
      doc["az_g"] = az;
    } else {
      doc["ax_g"] = nullptr;
      doc["ay_g"] = nullptr;
      doc["az_g"] = nullptr;
    }

    char out[256];
    size_t n = serializeJson(doc, out, sizeof(out));

    mqtt.publish(MQTT_PUB_TOPIC, out, n);
    Serial.println(out);
  }
}
