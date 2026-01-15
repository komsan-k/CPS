/*
  Lab: SensorML -> ESP32 (MPU6050 Self-Describing IMU) [COMPILE-READY]
  -------------------------------------------------------------------
  - Uses Adafruit_MPU6050 library to read MPU6050 via I2C
  - Parses a simplified SensorML subset
  - Applies:
      samplingRateHz
      accel range (rangeG)
      gyro range (rangeDps)
      per-axis calibration (scale/offset) for accel and gyro
      uom + uncertainty
  - Prints self-describing JSON lines to Serial

  IMPORTANT:
  - Install libraries via Arduino Library Manager:
      * Adafruit MPU6050
      * Adafruit Unified Sensor
      * Adafruit BusIO (usually pulled automatically)
  - Wiring (ESP32 default):
      SDA = GPIO21, SCL = GPIO22
  - Typical MPU6050 module I2C address: 0x68 (GY-521)

  Parser approach:
  - We parse within <accelerometer>...</accelerometer> and <gyroscope>...</gyroscope>
    to correctly separate tags that share the same names (scaleX, offsetX, etc.)
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

static const int I2C_SDA = 21;
static const int I2C_SCL = 22;

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

  // Extract accelerometer block
  String accelBlock;
  if (getBlock(xml, "accelerometer", accelBlock)) {
    c.accelUom = getTagValueIn(accelBlock, "uom");
    if (c.accelUom.length() == 0) c.accelUom = "m/s2";
    c.accelRangeG = (int)getTagFloatIn(accelBlock, "rangeG", 2.0f);
    c.accelUnc = getTagFloatIn(accelBlock, "uncertainty", 0.15f);

    String calBlock;
    if (getBlock(accelBlock, "calibration", calBlock)) {
      parseAxisCal(calBlock, c.accelCal);
    }
  }

  // Extract gyroscope block
  String gyroBlock;
  if (getBlock(xml, "gyroscope", gyroBlock)) {
    c.gyroUom = getTagValueIn(gyroBlock, "uom");
    if (c.gyroUom.length() == 0) c.gyroUom = "deg/s";
    c.gyroRangeDps = (int)getTagFloatIn(gyroBlock, "rangeDps", 250.0f);
    c.gyroUnc = getTagFloatIn(gyroBlock, "uncertainty", 1.0f);

    String calBlock;
    if (getBlock(gyroBlock, "calibration", calBlock)) {
      parseAxisCal(calBlock, c.gyroCal);
    }
  }
}

// -------------------- Apply range settings --------------------
static void applyMpuRanges(const ImuConfig& c) {
  // Accelerometer range mapping
  if (c.accelRangeG <= 2) mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  else if (c.accelRangeG <= 4) mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  else if (c.accelRangeG <= 8) mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  else mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

  // Gyro range mapping
  if (c.gyroRangeDps <= 250) mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  else if (c.gyroRangeDps <= 500) mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  else if (c.gyroRangeDps <= 1000) mpu.setGyroRange(MPU6050_RANGE_1000_DEG);
  else mpu.setGyroRange(MPU6050_RANGE_2000_DEG);

  // Filter bandwidth: pick something reasonable for many labs
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

static void printConfig(const ImuConfig& c) {
  Serial.println("=== SensorML Parsed Config (MPU6050) ===");
  Serial.print("identifier: "); Serial.println(c.id);
  Serial.print("samplingRateHz: "); Serial.println(c.samplingRateHz, 2);
  Serial.print("samplePeriodMs: "); Serial.println(c.samplePeriodMs);

  Serial.println("[Accelerometer]");
  Serial.print("  uom: "); Serial.println(c.accelUom);
  Serial.print("  rangeG: "); Serial.println(c.accelRangeG);
  Serial.print("  uncertainty: "); Serial.println(c.accelUnc, 3);
  Serial.print("  scale: "); Serial.print(c.accelCal.scaleX, 3); Serial.print(", ");
  Serial.print(c.accelCal.scaleY, 3); Serial.print(", ");
  Serial.println(c.accelCal.scaleZ, 3);
  Serial.print("  offset: "); Serial.print(c.accelCal.offX, 3); Serial.print(", ");
  Serial.print(c.accelCal.offY, 3); Serial.print(", ");
  Serial.println(c.accelCal.offZ, 3);

  Serial.println("[Gyroscope]");
  Serial.print("  uom: "); Serial.println(c.gyroUom);
  Serial.print("  rangeDps: "); Serial.println(c.gyroRangeDps);
  Serial.print("  uncertainty: "); Serial.println(c.gyroUnc, 3);
  Serial.print("  scale: "); Serial.print(c.gyroCal.scaleX, 3); Serial.print(", ");
  Serial.print(c.gyroCal.scaleY, 3); Serial.print(", ");
  Serial.println(c.gyroCal.scaleZ, 3);
  Serial.print("  offset: "); Serial.print(c.gyroCal.offX, 3); Serial.print(", ");
  Serial.print(c.gyroCal.offY, 3); Serial.print(", ");
  Serial.println(c.gyroCal.offZ, 3);

  Serial.println("=======================================");
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(I2C_SDA, I2C_SCL);

  parseSensorML(cfg);
  printConfig(cfg);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring and I2C address (usually 0x68).");
    while (true) delay(10);
  }

  applyMpuRanges(cfg);

  Serial.println("MPU6050 initialized.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleMs < cfg.samplePeriodMs) return;
  lastSampleMs = now;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Adafruit provides:
  // - a.acceleration.* in m/s^2
  // - g.gyro.* in rad/s -> convert to deg/s for our SensorML uom
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

  // Self-describing JSON output
  Serial.print("{\"id\":\""); Serial.print(cfg.id); Serial.print("\",");
  Serial.print("\"ts_ms\":"); Serial.print(now); Serial.print(",");

  Serial.print("\"accel\":{\"uom\":\""); Serial.print(cfg.accelUom);
  Serial.print("\",\"unc\":"); Serial.print(cfg.accelUnc, 3);
  Serial.print(",\"x\":"); Serial.print(axC, 3);
  Serial.print(",\"y\":"); Serial.print(ayC, 3);
  Serial.print(",\"z\":"); Serial.print(azC, 3);
  Serial.print("},");

  Serial.print("\"gyro\":{\"uom\":\""); Serial.print(cfg.gyroUom);
  Serial.print("\",\"unc\":"); Serial.print(cfg.gyroUnc, 3);
  Serial.print(",\"x\":"); Serial.print(gxC, 3);
  Serial.print(",\"y\":"); Serial.print(gyC, 3);
  Serial.print(",\"z\":"); Serial.print(gzC, 3);
  Serial.print("},");

  Serial.print("\"temp_c\":"); Serial.print(temp.temperature, 2);
  Serial.println("}");
}
