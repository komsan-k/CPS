
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "secrets.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define LDR_PIN 34
#define LM73_ADDR 0x48
#define MPU6050_ADDR 0x68

const int MA_WINDOW = 10;
float ldrBuffer[MA_WINDOW];
int ldrIndex = 0;
float accelOffsetX = 0;

float movingAverage(float newVal) {
  ldrBuffer[ldrIndex] = newVal;
  ldrIndex = (ldrIndex + 1) % MA_WINDOW;
  float sum = 0;
  for(int i=0;i<MA_WINDOW;i++) sum += ldrBuffer[i];
  return sum / MA_WINDOW;
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) mqtt.connect(DEVICE_ID);
}

bool readLm73(float &tempC) {
  Wire.beginTransmission(LM73_ADDR);
  Wire.write(0x00);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(LM73_ADDR, 2);
  int16_t raw = (Wire.read() << 8) | Wire.read();
  raw >>= 2;
  tempC = raw * 0.03125;
  return true;
}

bool readMpu(float &ax) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(MPU6050_ADDR, 2);
  int16_t rawX = (Wire.read() << 8) | Wire.read();
  ax = rawX / 16384.0;
  return true;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  Wire.begin();
  connectWiFi();
  connectMQTT();

  float sum=0;
  for(int i=0;i<50;i++){
    float ax;
    readMpu(ax);
    sum += ax;
    delay(20);
  }
  accelOffsetX = sum/50.0;
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  float luxRaw = analogRead(LDR_PIN);
  float luxFiltered = movingAverage(luxRaw);

  float tempRaw;
  readLm73(tempRaw);
  static float tempFiltered = tempRaw;
  tempFiltered = 0.9*tempFiltered + 0.1*tempRaw;

  float axRaw;
  readMpu(axRaw);
  float axCorrected = axRaw - accelOffsetX;

  StaticJsonDocument<256> doc;
  doc["lux_raw"] = luxRaw;
  doc["lux_filtered"] = luxFiltered;
  doc["temp_raw"] = tempRaw;
  doc["temp_filtered"] = tempFiltered;
  doc["ax_raw"] = axRaw;
  doc["ax_corrected"] = axCorrected;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  mqtt.publish(MQTT_PUB_TOPIC, buffer, n);
  Serial.println(buffer);

  delay(1000);
}
