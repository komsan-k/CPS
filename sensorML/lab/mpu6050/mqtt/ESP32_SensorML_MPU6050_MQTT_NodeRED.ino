/*
  Lab: SensorML -> ESP32 (MPU6050 Self-Describing IMU) + MQTT + Node-RED
  --------------------------------------------------------------------
  - Reads MPU6050 via I2C using Adafruit_MPU6050
  - Parses SensorML subset (block-based: <accelerometer> and <gyroscope>)
  - Applies samplingRateHz, ranges, per-axis calibration, uom, uncertainty
  - Publishes:
      * device/imu01/metadata   (retained, once at boot)
      * device/imu01/telemetry  (stream)
      * device/imu01/cmd        (optional commands: {"led":1} etc. - not used here)
  - Also prints telemetry JSON to Serial

  Libraries (Arduino Library Manager):
  - Adafruit MPU6050
  - Adafruit Unified Sensor
  - Adafruit BusIO
  - PubSubClient

  ESP32 WiFi:
  - Built-in WiFi.h

  Wiring:
  - SDA=GPIO21, SCL=GPIO22
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// -------------------- User Wi-Fi + MQTT Config --------------------
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

const char* MQTT_HOST = "192.168.1.10";  // <-- change to your broker IP/hostname
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_META = "device/imu01/metadata";
const char* TOPIC_TELE = "device/imu01/telemetry";
// ---------------------------------------------------------------

static const int I2C_SDA = 21;
static const int I2C_SCL = 22;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
Adafruit_MPU6050 mpu;

// -------------------- SensorML XML (embedded) --------------------
const char* SENSORML_XML = R"xml(
<SensorML>
  <System>
    <identifier>MPU6050_ESP32_01</identifier>

    <samplingRateHz>50</samplingRateHz>

    <accelerometer>
      <uom>m/s2</uom>
      <rangeG>2</rangeG>
      <uncertainty>0.15</uncertainty>
      <calibration>
        <scaleX>1.000</scaleX><offsetX>0.00</offsetX>
        <scaleY>1.000</scaleY><offsetY>0.00</offsetY>
        <scaleZ>1.000</scaleZ><offsetZ>0.00</offsetZ>
      </calibration>
    </accelerometer>

    <gyroscope>
      <uom>deg/s</uom>
      <rangeDps>250</rangeDps>
      <uncertainty>1.0</uncertainty>
      <calibration>
        <scaleX>1.000</scaleX><offsetX>0.00</offsetX>
        <scaleY>1.000</scaleY><offsetY>0.00</offsetY>
        <scaleZ>1.000</scaleZ><offsetZ>0.00</offsetZ>
      </calibration>
    </gyroscope>
  </System>
</SensorML>
)xml";

// -------------------- Simple parsing helpers --------------------
static String getTagValueIn(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int start = xml.indexOf(openTag);
  if (start < 0) return "";
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end < 0) return "";
  String v = xml.substring(start, end);
  v.trim();
  return v;
}

static float getTagFloatIn(const String& xml, const String& tag, float defVal) {
  String v = getTagValueIn(xml, tag);
  if (v.length() == 0) return defVal;
  return v.toFloat();
}

static bool getBlock(const String& xml, const String& tag, String& outBlock) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int s = xml.indexOf(openTag);
  if (s < 0) return false;
  int e = xml.indexOf(closeTag, s);
  if (e < 0) return false;
  e += closeTag.length();
  outBlock = xml.substring(s, e);
  outBlock.trim();
  return true;
}

// -------------------- Config structures --------------------
struct AxisCal {
  float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
  float offX   = 0.0f, offY   = 0.0f, offZ   = 0.0f;
};

struct ImuConfig {
  String id;
  float samplingRateHz = 50.0f;
  unsigned long samplePeriodMs = 20;

  // Accelerometer
  String accelUom = "m/s2";
  int accelRangeG = 2;
  float accelUnc = 0.15f;
  AxisCal accelCal;

  // Gyroscope
  String gyroUom = "deg/s";
  int gyroRangeDps = 250;
  float gyroUnc = 1.0f;
  AxisCal gyroCal;
};

ImuConfig cfg;
unsigned long lastSampleMs = 0;

// -------------------- Parse SensorML --------------------
static void parseAxisCal(const String& block, AxisCal& cal) {
  cal.scaleX = getTagFloatIn(block, "scaleX", 1.0f);
  cal.offX   = getTagFloatIn(block, "offsetX", 0.0f);
  cal.scaleY = getTagFloatIn(block, "scaleY", 1.0f);
  cal.offY   = getTagFloatIn(block, "offsetY", 0.0f);
  cal.scaleZ = getTagFloatIn(block, "scaleZ", 1.0f);
  cal.offZ   = getTagFloatIn(block, "offsetZ", 0.0f);
}

static void parseSensorML(ImuConfig& c) {
  String xml = String(SENSORML_XML);

  c.id = getTagValueIn(xml, "identifier");
  if (c.id.length() == 0) c.id = "MPU6050_ESP32_01";

  c.samplingRateHz = getTagFloatIn(xml, "samplingRateHz", 50.0f);
  if (c.samplingRateHz < 1.0f) c.samplingRateHz = 1.0f;
  c.samplePeriodMs = (unsigned long)(1000.0f / c.samplingRateHz);

  // Accelerometer block
  String accelBlock;
  if (getBlock(xml, "accelerometer", accelBlock)) {
    c.accelUom = getTagValueIn(accelBlock, "uom");
    if (c.accelUom.length() == 0) c.accelUom = "m/s2";
    c.accelRangeG = (int)getTagFloatIn(accelBlock, "rangeG", 2.0f);
    c.accelUnc = getTagFloatIn(accelBlock, "uncertainty", 0.15f);

    String calBlock;
    if (getBlock(accelBlock, "calibration", calBlock)) parseAxisCal(calBlock, c.accelCal);
  }

  // Gyroscope block
  String gyroBlock;
  if (getBlock(xml, "gyroscope", gyroBlock)) {
    c.gyroUom = getTagValueIn(gyroBlock, "uom");
    if (c.gyroUom.length() == 0) c.gyroUom = "deg/s";
    c.gyroRangeDps = (int)getTagFloatIn(gyroBlock, "rangeDps", 250.0f);
    c.gyroUnc = getTagFloatIn(gyroBlock, "uncertainty", 1.0f);

    String calBlock;
    if (getBlock(gyroBlock, "calibration", calBlock)) parseAxisCal(calBlock, c.gyroCal);
  }
}

// -------------------- Apply range settings --------------------
static void applyMpuRanges(const ImuConfig& c) {
  if (c.accelRangeG <= 2) mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  else if (c.accelRangeG <= 4) mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  else if (c.accelRangeG <= 8) mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  else mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

  if (c.gyroRangeDps <= 250) mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  else if (c.gyroRangeDps <= 500) mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  else if (c.gyroRangeDps <= 1000) mpu.setGyroRange(MPU6050_RANGE_1000_DEG);
  else mpu.setGyroRange(MPU6050_RANGE_2000_DEG);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

// -------------------- WiFi / MQTT --------------------
static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[WiFi] connecting");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - t0 > 15000) {
      Serial.println("\n[WiFi] timeout. Check SSID/PASS.");
      return;
    }
  }
  Serial.println();
  Serial.print("[WiFi] connected, IP: ");
  Serial.println(WiFi.localIP());
}

static void ensureMqttConnected() {
  if (mqtt.connected()) return;

  while (!mqtt.connected()) {
    String clientId = "esp32-imu-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("[MQTT] connecting as ");
    Serial.println(clientId);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("[MQTT] connected");
      // publish metadata retained once connected
      // (safe to send again on reconnect)
      // handled below in publishMetadata()
      return;
    } else {
      Serial.print("[MQTT] failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retrying in 2s");
      delay(2000);
    }
  }
}

// -------------------- Publishing helpers --------------------
static void publishMetadata() {
  // Keep metadata concise for dashboards
  String meta = "{";
  meta += "\"id\":\"" + cfg.id + "\",";
  meta += "\"samplingRateHz\":" + String(cfg.samplingRateHz, 2) + ",";
  meta += "\"accel\":{\"uom\":\"" + cfg.accelUom + "\",\"rangeG\":" + String(cfg.accelRangeG) + ",\"unc\":" + String(cfg.accelUnc, 3) + "},";
  meta += "\"gyro\":{\"uom\":\"" + cfg.gyroUom + "\",\"rangeDps\":" + String(cfg.gyroRangeDps) + ",\"unc\":" + String(cfg.gyroUnc, 3) + "}";
  meta += "}";

  mqtt.publish(TOPIC_META, meta.c_str(), true); // retained
  Serial.print("[MQTT] meta -> ");
  Serial.println(TOPIC_META);
}

static void publishTelemetry(float axC, float ayC, float azC,
                             float gxC, float gyC, float gzC,
                             float tempC, unsigned long ts) {
  // compact JSON for streaming
  String msg = "{";
  msg += "\"id\":\"" + cfg.id + "\",";
  msg += "\"ts_ms\":" + String(ts) + ",";
  msg += "\"accel\":{\"uom\":\"" + cfg.accelUom + "\",\"unc\":" + String(cfg.accelUnc, 3)
      + ",\"x\":" + String(axC, 3) + ",\"y\":" + String(ayC, 3) + ",\"z\":" + String(azC, 3) + "},";
  msg += "\"gyro\":{\"uom\":\"" + cfg.gyroUom + "\",\"unc\":" + String(cfg.gyroUnc, 3)
      + ",\"x\":" + String(gxC, 3) + ",\"y\":" + String(gyC, 3) + ",\"z\":" + String(gzC, 3) + "},";
  msg += "\"temp_c\":" + String(tempC, 2);
  msg += "}";

  mqtt.publish(TOPIC_TELE, msg.c_str(), false);

  // also print to Serial (useful for debugging)
  Serial.println(msg);
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(I2C_SDA, I2C_SCL);

  parseSensorML(cfg);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring and I2C address (usually 0x68).");
    while (true) delay(10);
  }
  applyMpuRanges(cfg);

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  ensureMqttConnected();
  publishMetadata();
}

void loop() {
  // keep connections alive
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  ensureMqttConnected();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastSampleMs < cfg.samplePeriodMs) return;
  lastSampleMs = now;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Raw values from Adafruit:
  // accel: m/s^2
  // gyro:  rad/s -> convert to deg/s
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float gx = g.gyro.x * 180.0f / PI; // deg/s
  float gy = g.gyro.y * 180.0f / PI;
  float gz = g.gyro.z * 180.0f / PI;

  // Apply per-axis calibration
  float axC = ax * cfg.accelCal.scaleX + cfg.accelCal.offX;
  float ayC = ay * cfg.accelCal.scaleY + cfg.accelCal.offY;
  float azC = az * cfg.accelCal.scaleZ + cfg.accelCal.offZ;

  float gxC = gx * cfg.gyroCal.scaleX + cfg.gyroCal.offX;
  float gyC = gy * cfg.gyroCal.scaleY + cfg.gyroCal.offY;
  float gzC = gz * cfg.gyroCal.scaleZ + cfg.gyroCal.offZ;

  publishTelemetry(axC, ayC, azC, gxC, gyC, gzC, temp.temperature, now);
}
