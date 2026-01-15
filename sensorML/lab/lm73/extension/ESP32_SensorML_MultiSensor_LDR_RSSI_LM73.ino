/*
  ESP32 SensorML Multi-Sensor (LDR + WiFi RSSI + LM73)
  ---------------------------------------------------
  - Parses simplified multi-sensor SensorML XML (3 sensors)
  - Independent sampling per sensor
  - Outputs self-describing JSON per reading

  Sensors:
  1) LDR via ADC (GPIO34)
  2) WiFi RSSI via WiFi.RSSI() (connect required)
  3) LM73 via I2C (GPIO21 SDA, GPIO22 SCL) with I2C address scan

  Optional CPS rules:
  - LED ON if RSSI < -75 dBm OR Temp > 35 °C
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

// -------------------- User Wi-Fi Config --------------------
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";
// -----------------------------------------------------------

// Pins
static const int LDR_PIN = 34;   // ADC input-only pin
static const int LED_PIN = 2;    // Onboard LED (varies by board)
static const int I2C_SDA = 21;
static const int I2C_SCL = 22;

// -------------------- Multi-Sensor SensorML (simplified) --------------------
const char* SENSORML_XML = R"xml(
<SensorML>
  <Sensors>

    <Sensor>
      <identifier>LDR_ESP32_01</identifier>
      <observedProperty>LightLevel</observedProperty>
      <uom>adc_counts</uom>
      <samplingRateHz>20</samplingRateHz>
      <calibration>
        <scale>1.0</scale>
        <offset>0.0</offset>
      </calibration>
      <uncertainty>0.05</uncertainty>
    </Sensor>

    <Sensor>
      <identifier>WIFI_RSSI_ESP32_01</identifier>
      <observedProperty>WiFiRSSI</observedProperty>
      <uom>dBm</uom>
      <samplingRateHz>2</samplingRateHz>
      <calibration>
        <scale>1.0</scale>
        <offset>0.0</offset>
      </calibration>
      <uncertainty>2.0</uncertainty>
      <targetSSID>YourWiFiSSID</targetSSID>
    </Sensor>

    <Sensor>
      <identifier>LM73_ESP32_01</identifier>
      <observedProperty>Temperature</observedProperty>
      <uom>degC</uom>
      <samplingRateHz>2</samplingRateHz>
      <calibration>
        <scale>1.000</scale>
        <offset>0.00</offset>
      </calibration>
      <uncertainty>1.0</uncertainty>
    </Sensor>

  </Sensors>
</SensorML>
)xml";

// -------------------- Helpers: extract tags --------------------
static String getTagValue(const String& xml, const String& tag) {
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

static float getTagFloat(const String& xml, const String& tag, float defVal) {
  String v = getTagValue(xml, tag);
  if (v.length() == 0) return defVal;
  return v.toFloat();
}

// Extract the Nth <Sensor> ... </Sensor> block
static bool getSensorBlock(const String& xml, int index, String& outBlock) {
  const String open = "<Sensor>";
  const String close = "</Sensor>";

  int pos = 0;
  for (int i = 0; i <= index; i++) {
    int s = xml.indexOf(open, pos);
    if (s < 0) return false;
    int e = xml.indexOf(close, s);
    if (e < 0) return false;
    e += close.length();
    if (i == index) {
      outBlock = xml.substring(s, e);
      outBlock.trim();
      return true;
    }
    pos = e;
  }
  return false;
}

// -------------------- Sensor Config --------------------
struct SensorConfig {
  String id;
  String prop;
  String uom;
  float rateHz = 1.0f;
  float scale = 1.0f;
  float offset = 0.0f;
  float uncertainty = 0.0f;
  String targetSSID; // RSSI only
};

static bool parseSensorConfig(const String& block, SensorConfig& cfg) {
  cfg.id = getTagValue(block, "identifier");
  cfg.prop = getTagValue(block, "observedProperty");
  cfg.uom = getTagValue(block, "uom");
  cfg.rateHz = getTagFloat(block, "samplingRateHz", 1.0f);
  cfg.scale = getTagFloat(block, "scale", 1.0f);
  cfg.offset = getTagFloat(block, "offset", 0.0f);
  cfg.uncertainty = getTagFloat(block, "uncertainty", 0.0f);
  cfg.targetSSID = getTagValue(block, "targetSSID");

  if (cfg.rateHz < 0.2f) cfg.rateHz = 0.2f; // avoid zero/very slow
  if (cfg.id.length() == 0 || cfg.prop.length() == 0 || cfg.uom.length() == 0) return false;
  return true;
}

// -------------------- State --------------------
SensorConfig cfgLdr, cfgRssi, cfgLm73;

unsigned long periodLdrMs = 50;
unsigned long periodRssiMs = 500;
unsigned long periodLm73Ms = 500;

unsigned long lastLdrMs = 0;
unsigned long lastRssiMs = 0;
unsigned long lastLm73Ms = 0;

uint8_t lm73Addr = 0;

// -------------------- Wi-Fi --------------------
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
  Serial.print("[WiFi] SSID: ");
  Serial.println(WiFi.SSID());
}

// -------------------- I2C scan + LM73 read --------------------
static uint8_t scanI2CFirstDevice() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return addr;
  }
  return 0;
}

static bool readLM73TempC(float &tempC) {
  if (lm73Addr == 0) return false;

  // Pointer -> Temperature register 0x00
  Wire.beginTransmission(lm73Addr);
  Wire.write(0x00);
  if (Wire.endTransmission(false) != 0) return false; // repeated start

  if (Wire.requestFrom((int)lm73Addr, 2) != 2) return false;
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();

  int16_t raw = (int16_t)((msb << 8) | lsb);
  int16_t temp14 = raw >> 2;     // signed, bits 15..2
  tempC = temp14 * 0.03125f;     // 0.03125 °C/LSB
  return true;
}

// -------------------- Output helpers --------------------
static void printJsonBasic(const SensorConfig& cfg, float value, unsigned long ts) {
  Serial.print("{\"id\":\"");
  Serial.print(cfg.id);
  Serial.print("\",\"property\":\"");
  Serial.print(cfg.prop);
  Serial.print("\",\"value\":");
  Serial.print(value, 3);
  Serial.print(",\"uom\":\"");
  Serial.print(cfg.uom);
  Serial.print("\",\"uncertainty\":");
  Serial.print(cfg.uncertainty, 3);
  Serial.print(",\"ts_ms\":");
  Serial.print(ts);
  Serial.println("}");
}

static void printJsonRssi(float value, unsigned long ts) {
  Serial.print("{\"id\":\"");
  Serial.print(cfgRssi.id);
  Serial.print("\",\"property\":\"");
  Serial.print(cfgRssi.prop);
  Serial.print("\",\"value\":");
  Serial.print(value, 2);
  Serial.print(",\"uom\":\"");
  Serial.print(cfgRssi.uom);
  Serial.print("\",\"uncertainty\":");
  Serial.print(cfgRssi.uncertainty, 2);
  Serial.print(",\"ssid\":\"");
  Serial.print(WiFi.SSID());
  Serial.print("\",\"ts_ms\":");
  Serial.print(ts);
  Serial.println("}");
}

static void printJsonLm73(float value, unsigned long ts) {
  Serial.print("{\"id\":\"");
  Serial.print(cfgLm73.id);
  Serial.print("\",\"property\":\"");
  Serial.print(cfgLm73.prop);
  Serial.print("\",\"value\":");
  Serial.print(value, 3);
  Serial.print(",\"uom\":\"");
  Serial.print(cfgLm73.uom);
  Serial.print("\",\"uncertainty\":");
  Serial.print(cfgLm73.uncertainty, 3);
  Serial.print(",\"i2c_addr\":\"0x");
  Serial.print(lm73Addr, HEX);
  Serial.print("\",\"ts_ms\":");
  Serial.print(ts);
  Serial.println("}");
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  analogReadResolution(12); // LDR

  // I2C init
  Wire.begin(I2C_SDA, I2C_SCL);

  // Parse SensorML (3 sensors)
  String xml = String(SENSORML_XML);

  String b0, b1, b2;
  bool ok0 = getSensorBlock(xml, 0, b0);
  bool ok1 = getSensorBlock(xml, 1, b1);
  bool ok2 = getSensorBlock(xml, 2, b2);

  bool p0 = ok0 && parseSensorConfig(b0, cfgLdr);
  bool p1 = ok1 && parseSensorConfig(b1, cfgRssi);
  bool p2 = ok2 && parseSensorConfig(b2, cfgLm73);

  // Defaults if parsing fails
  if (!p0) {
    cfgLdr = {"LDR_ESP32_01","LightLevel","adc_counts",20.0f,1.0f,0.0f,0.05f,""};
  }
  if (!p1) {
    cfgRssi = {"WIFI_RSSI_ESP32_01","WiFiRSSI","dBm",2.0f,1.0f,0.0f,2.0f,String(WIFI_SSID)};
  }
  if (!p2) {
    cfgLm73 = {"LM73_ESP32_01","Temperature","degC",2.0f,1.0f,0.0f,1.0f,""};
  }

  periodLdrMs  = (unsigned long)(1000.0f / cfgLdr.rateHz);
  periodRssiMs = (unsigned long)(1000.0f / cfgRssi.rateHz);
  periodLm73Ms = (unsigned long)(1000.0f / cfgLm73.rateHz);

  Serial.println("=== Parsed SensorML (3-Sensor) ===");

  Serial.println("[LDR]");
  Serial.print("  id: "); Serial.println(cfgLdr.id);
  Serial.print("  rateHz: "); Serial.println(cfgLdr.rateHz, 2);
  Serial.print("  periodMs: "); Serial.println(periodLdrMs);

  Serial.println("[RSSI]");
  Serial.print("  id: "); Serial.println(cfgRssi.id);
  Serial.print("  rateHz: "); Serial.println(cfgRssi.rateHz, 2);
  Serial.print("  targetSSID: "); Serial.println(cfgRssi.targetSSID);
  Serial.print("  periodMs: "); Serial.println(periodRssiMs);

  Serial.println("[LM73]");
  Serial.print("  id: "); Serial.println(cfgLm73.id);
  Serial.print("  rateHz: "); Serial.println(cfgLm73.rateHz, 2);
  Serial.print("  periodMs: "); Serial.println(periodLm73Ms);

  Serial.println("==================================");

  // Connect Wi-Fi (for RSSI)
  connectWiFi();

  // Find LM73 address
  Serial.println("[I2C] scanning...");
  lm73Addr = scanI2CFirstDevice();
  if (lm73Addr == 0) {
    Serial.println("[I2C] no device found. Check LM73 wiring/pullups/ADDR.");
  } else {
    Serial.print("[I2C] found device at 0x");
    Serial.println(lm73Addr, HEX);
  }
}

void loop() {
  unsigned long now = millis();

  // Reconnect Wi-Fi if needed (simple)
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    delay(300);
  }

  // LDR
  if (now - lastLdrMs >= periodLdrMs) {
    lastLdrMs = now;

    int raw = analogRead(LDR_PIN);
    float cal = raw * cfgLdr.scale + cfgLdr.offset;

    // Print LDR JSON
    Serial.print("{\"id\":\"");
    Serial.print(cfgLdr.id);
    Serial.print("\",\"property\":\"");
    Serial.print(cfgLdr.prop);
    Serial.print("\",\"value\":");
    Serial.print(cal, 2);
    Serial.print(",\"uom\":\"");
    Serial.print(cfgLdr.uom);
    Serial.print("\",\"uncertainty\":");
    Serial.print(cfgLdr.uncertainty, 4);
    Serial.print(",\"ts_ms\":");
    Serial.print(now);
    Serial.println("}");
  }

  // RSSI
  float rssiCal = NAN;
  if (now - lastRssiMs >= periodRssiMs) {
    lastRssiMs = now;

    int rssiRaw = WiFi.RSSI();
    rssiCal = rssiRaw * cfgRssi.scale + cfgRssi.offset;
    printJsonRssi(rssiCal, now);
  }

  // LM73
  float tempCal = NAN;
  if (now - lastLm73Ms >= periodLm73Ms) {
    lastLm73Ms = now;

    if (lm73Addr != 0) {
      float tC = 0;
      if (readLM73TempC(tC)) {
        tempCal = tC * cfgLm73.scale + cfgLm73.offset;
        printJsonLm73(tempCal, now);
      } else {
        Serial.println("{\"error\":\"LM73 read failed\"}");
      }
    } else {
      // If no device, print once in a while
      Serial.println("{\"error\":\"LM73 not found\"}");
    }
  }

  // Optional CPS rules -> LED
  // weak RSSI OR high temperature
  const float weakRssiThresh = -75.0f; // dBm
  const float highTempThresh = 35.0f;  // °C

  bool weak = (!isnan(rssiCal) && (rssiCal < weakRssiThresh));
  bool hot  = (!isnan(tempCal) && (tempCal > highTempThresh));

  if (weak || hot) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}
