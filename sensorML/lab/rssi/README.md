# Lab: SensorML → ESP32 (RSSI Self-Describing Sensor)

## Objective

This lab demonstrates how **SensorML (subset)** can be used to describe a **Wi‑Fi RSSI sensor** as metadata and how an ESP32 can read this metadata and apply it in real operation.

By the end of this lab, students will be able to:
- Use **SensorML as metadata** to describe an RSSI sensor
- Parse simplified SensorML on an ESP32
- Apply SensorML parameters to:
  - Configure sampling rate
  - Perform calibration (scale / offset)
  - Attach unit (dBm) and uncertainty
- Read Wi‑Fi RSSI values and output **self‑describing JSON**
- Understand RSSI as a **virtual sensor** in CPS/IoT

> **Note:** RSSI is radio information, not a physical sensor.  
> In CPS/IoT, it is treated as a **virtual sensor**.

---

## 1) SensorML (Simplified for RSSI)

A minimal SensorML subset is sufficient for this lab and can be embedded as a string in the ESP32 code.

### Example: `sensorml.xml`

```xml
<SensorML>
  <System>
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
  </System>
</SensorML>
```

### Practical Meaning

| SensorML Field | Usage |
|---------------|-------|
| `samplingRateHz` | RSSI reading frequency (Hz) |
| `scale / offset` | RSSI compensation or calibration |
| `uncertainty` | Measurement uncertainty (dB) |
| `targetSSID` | Target AP when multiple networks exist |

---

## 2) ESP32 Arduino Code — SensorML → RSSI

Two operating modes are possible:

- **Mode A (Recommended):**  
  Connect to Wi‑Fi and use `WiFi.RSSI()`  
- **Mode B:**  
  Scan networks and extract RSSI without connecting (slower)

This lab uses **Mode A** for simplicity and reliability.

```cpp
/*
  Lab: SensorML -> ESP32 (WiFi RSSI)
  - Parse simplified SensorML XML (string tag extraction)
  - Use samplingRateHz, scale, offset, uncertainty
  - Connect WiFi and read RSSI in dBm
  - Print self-describing JSON line
*/

#include <Arduino.h>
#include <WiFi.h>

// ---- Put your WiFi credentials here ----
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

// ---- SensorML XML (embedded) ----
const char* SENSORML_XML = R"xml(
<SensorML>
  <System>
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
  </System>
</SensorML>
)xml";

// ---- Simple tag extraction helpers ----
String getTagValue(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int start = xml.indexOf(openTag);
  if (start < 0) return "";
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end < 0) return "";
  return xml.substring(start, end);
}

float getTagFloat(const String& xml, const String& tag, float defaultVal) {
  String v = getTagValue(xml, tag);
  v.trim();
  if (v.length() == 0) return defaultVal;
  return v.toFloat();
}

// ---- Parsed config from SensorML ----
String sensorId, observedProperty, uom, targetSSID;
float samplingRateHz = 2.0f;
float scale = 1.0f;
float offset = 0.0f;
float uncertainty = 2.0f;

unsigned long samplePeriodMs = 500;
unsigned long lastSampleMs = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - t0 > 15000) { // 15s timeout
      Serial.println("\nWiFi connect timeout. Check SSID/PASS.");
      return;
    }
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Parse SensorML
  String xml = String(SENSORML_XML);
  sensorId = getTagValue(xml, "identifier");
  observedProperty = getTagValue(xml, "observedProperty");
  uom = getTagValue(xml, "uom");
  targetSSID = getTagValue(xml, "targetSSID");

  samplingRateHz = getTagFloat(xml, "samplingRateHz", 2.0f);
  scale = getTagFloat(xml, "scale", 1.0f);
  offset = getTagFloat(xml, "offset", 0.0f);
  uncertainty = getTagFloat(xml, "uncertainty", 2.0f);

  if (samplingRateHz < 0.2f) samplingRateHz = 0.2f; // avoid too slow/zero
  samplePeriodMs = (unsigned long)(1000.0f / samplingRateHz);

  Serial.println("=== SensorML Parsed Config ===");
  Serial.print("identifier: "); Serial.println(sensorId);
  Serial.print("observedProperty: "); Serial.println(observedProperty);
  Serial.print("uom: "); Serial.println(uom);
  Serial.print("samplingRateHz: "); Serial.println(samplingRateHz, 2);
  Serial.print("scale: "); Serial.println(scale, 4);
  Serial.print("offset: "); Serial.println(offset, 4);
  Serial.print("uncertainty: "); Serial.println(uncertainty, 2);
  Serial.print("targetSSID: "); Serial.println(targetSSID);
  Serial.print("samplePeriodMs: "); Serial.println(samplePeriodMs);
  Serial.println("================================");

  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // Try reconnect occasionally (simple)
    connectWiFi();
    delay(1000);
    return;
  }

  unsigned long now = millis();
  if (now - lastSampleMs >= samplePeriodMs) {
    lastSampleMs = now;

    int rssiRaw = WiFi.RSSI();           // typically negative dBm, e.g., -45
    float rssiCal = rssiRaw * scale + offset;

    // Self-describing output
    Serial.print("{\"id\":\"");
    Serial.print(sensorId);
    Serial.print("\",\"property\":\"");
    Serial.print(observedProperty);
    Serial.print("\",\"value\":");
    Serial.print(rssiCal, 2);
    Serial.print(",\"uom\":\"");
    Serial.print(uom);
    Serial.print("\",\"uncertainty\":");
    Serial.print(uncertainty, 2);
    Serial.print(",\"ssid\":\"");
    Serial.print(WIFI_SSID);
    Serial.print("\",\"ts_ms\":");
    Serial.print(now);
    Serial.println("}");
  }
}

```

### Key Features
- String‑based XML tag extraction (intro‑friendly)
- Sampling controlled by SensorML
- Self‑describing JSON output

---

## 3) What Students Will Observe

- RSSI values printed as **JSON self‑describing data**
- Metadata (`id`, `property`, `uom`, `uncertainty`, `ssid`) travels with the value
- Sampling rate changes by editing `<samplingRateHz>` in SensorML
- RSSI varies with distance to the Wi‑Fi access point:
  - Near AP: ~ −40 dBm
  - Far from AP: ~ −75 to −90 dBm

---

## 4) Lab Activities (Checklist)

- Set correct `WIFI_SSID` and `WIFI_PASS`
- Run the code and verify parsed SensorML output
- Change `<samplingRateHz>` to **1, 2, 5**
- Move closer to / farther from the router and observe RSSI changes
- Experiment with calibration:
  - `offset = +3` (bias compensation)
  - `scale = 1.05` (scaling adjustment)
- *(Optional CPS rule)*  
  If RSSI < −75 dBm → trigger LED or warning message

---

## 5) Deliverables

Students must submit:
- Modified **SensorML** (sampling, calibration, uncertainty)
- Serial log (≥ 30 seconds) showing RSSI changes with distance
- Short reflection (5–10 lines):  
  **“How does SensorML help other systems understand an RSSI sensor?”**

---

## Key Takeaway

This lab shows how **SensorML enables self‑describing virtual sensors**, allowing CPS and IoT systems to interpret RSSI data correctly without hard‑coded assumptions.
