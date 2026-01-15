/*
  Combined Lab: Multi-Sensor SensorML -> ESP32 (LDR + Wi-Fi RSSI)
  ----------------------------------------------------------------
  - Parses a simplified multi-sensor SensorML XML (2 sensors)
  - Each sensor has its own:
      identifier, observedProperty, uom, samplingRateHz,
      calibration (scale/offset), uncertainty
  - LDR: analogRead(GPIO34)
  - RSSI: WiFi.RSSI() after connecting
  - Prints self-describing JSON lines for each sensor to Serial

  Optional:
  - Turn on LED when RSSI is weak (< -75 dBm) (CPS rule)

  Requirements:
  - ESP32 Arduino core
  - WiFi library (built-in)

  Note:
  - This is a "teaching" parser using String searches (not full XML parsing).
*/

#include <Arduino.h>
#include <WiFi.h>

// -------------------- User Wi-Fi Config --------------------
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";
// -----------------------------------------------------------

// Pins
static const int LDR_PIN = 34;   // ADC input-only pin
static const int LED_PIN = 2;    // Onboard LED (varies by board)

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
  String targetSSID; // only for RSSI sensor
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

  if (cfg.rateHz < 0.2f) cfg.rateHz = 0.2f; // avoid zero/very slow issues
  if (cfg.id.length() == 0 || cfg.prop.length() == 0 || cfg.uom.length() == 0) return false;
  return true;
}

// -------------------- State --------------------
SensorConfig cfgLdr;
SensorConfig cfgRssi;

unsigned long periodLdrMs = 50;
unsigned long periodRssiMs = 500;

unsigned long lastLdrMs = 0;
unsigned long lastRssiMs = 0;

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

// -------------------- Output helpers --------------------
static void printJsonLdr(float value, unsigned long ts) {
  Serial.print("{\"id\":\"");
  Serial.print(cfgLdr.id);
  Serial.print("\",\"property\":\"");
  Serial.print(cfgLdr.prop);
  Serial.print("\",\"value\":");
  Serial.print(value, 2);
  Serial.print(",\"uom\":\"");
  Serial.print(cfgLdr.uom);
  Serial.print("\",\"uncertainty\":");
  Serial.print(cfgLdr.uncertainty, 4);
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

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ADC config for LDR
  analogReadResolution(12); // 0..4095

  // Parse multi-sensor XML
  String xml = String(SENSORML_XML);

  String block0, block1;
  bool ok0 = getSensorBlock(xml, 0, block0);
  bool ok1 = getSensorBlock(xml, 1, block1);

  bool parsed0 = ok0 && parseSensorConfig(block0, cfgLdr);
  bool parsed1 = ok1 && parseSensorConfig(block1, cfgRssi);

  if (!parsed0) {
    // defaults
    cfgLdr.id = "LDR_ESP32_01";
    cfgLdr.prop = "LightLevel";
    cfgLdr.uom = "adc_counts";
    cfgLdr.rateHz = 20.0f;
    cfgLdr.scale = 1.0f;
    cfgLdr.offset = 0.0f;
    cfgLdr.uncertainty = 0.05f;
  }
  if (!parsed1) {
    cfgRssi.id = "WIFI_RSSI_ESP32_01";
    cfgRssi.prop = "WiFiRSSI";
    cfgRssi.uom = "dBm";
    cfgRssi.rateHz = 2.0f;
    cfgRssi.scale = 1.0f;
    cfgRssi.offset = 0.0f;
    cfgRssi.uncertainty = 2.0f;
    cfgRssi.targetSSID = WIFI_SSID;
  }

  periodLdrMs = (unsigned long)(1000.0f / cfgLdr.rateHz);
  periodRssiMs = (unsigned long)(1000.0f / cfgRssi.rateHz);

  Serial.println("=== Parsed SensorML (Multi-Sensor) ===");

  Serial.println("[LDR]");
  Serial.print("  id: "); Serial.println(cfgLdr.id);
  Serial.print("  prop: "); Serial.println(cfgLdr.prop);
  Serial.print("  uom: "); Serial.println(cfgLdr.uom);
  Serial.print("  rateHz: "); Serial.println(cfgLdr.rateHz, 2);
  Serial.print("  scale: "); Serial.println(cfgLdr.scale, 6);
  Serial.print("  offset: "); Serial.println(cfgLdr.offset, 6);
  Serial.print("  uncertainty: "); Serial.println(cfgLdr.uncertainty, 4);
  Serial.print("  periodMs: "); Serial.println(periodLdrMs);

  Serial.println("[RSSI]");
  Serial.print("  id: "); Serial.println(cfgRssi.id);
  Serial.print("  prop: "); Serial.println(cfgRssi.prop);
  Serial.print("  uom: "); Serial.println(cfgRssi.uom);
  Serial.print("  rateHz: "); Serial.println(cfgRssi.rateHz, 2);
  Serial.print("  scale: "); Serial.println(cfgRssi.scale, 6);
  Serial.print("  offset: "); Serial.println(cfgRssi.offset, 6);
  Serial.print("  uncertainty: "); Serial.println(cfgRssi.uncertainty, 2);
  Serial.print("  targetSSID: "); Serial.println(cfgRssi.targetSSID);
  Serial.print("  periodMs: "); Serial.println(periodRssiMs);

  Serial.println("=====================================");

  // Connect Wi-Fi for RSSI sensor (Mode A)
  connectWiFi();
}

void loop() {
  unsigned long now = millis();

  // Reconnect Wi-Fi if needed (simple)
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    delay(500);
  }

  // LDR sampling
  if (now - lastLdrMs >= periodLdrMs) {
    lastLdrMs = now;

    int raw = analogRead(LDR_PIN);
    float cal = raw * cfgLdr.scale + cfgLdr.offset;

    printJsonLdr(cal, now);
  }

  // RSSI sampling
  if (now - lastRssiMs >= periodRssiMs) {
    lastRssiMs = now;

    int rssiRaw = WiFi.RSSI();          // negative dBm
    float rssiCal = rssiRaw * cfgRssi.scale + cfgRssi.offset;

    // Optional CPS rule: weak signal warning / LED
    const float weakThresh = -75.0f; // dBm
    if (rssiCal < weakThresh) digitalWrite(LED_PIN, HIGH);
    else digitalWrite(LED_PIN, LOW);

    printJsonRssi(rssiCal, now);
  }
}
