# Lab: SensorML → ESP32 (LDR Self‑Describing Sensor)

## Objective

This lab introduces **SensorML as sensor metadata (not measured values)** and demonstrates how an ESP32 can **read simplified SensorML** and use it in real operation.

By the end of this lab, students will be able to:
- Understand SensorML as **metadata describing sensors**
- Parse a simplified SensorML subset on ESP32
- Use SensorML to configure:
  - Sampling rate
  - Calibration (scale / offset)
  - Unit and uncertainty
- Read LDR values via ADC and output **self‑describing sensor data** via Serial (or MQTT, optional)

---

## Hardware / Wokwi Setup

### Required Components
- ESP32 DevKit
- LDR
- 10kΩ resistor (voltage divider)
- (Optional) 1 × LED

### Circuit Connection (Recommended)

```
3.3V ── LDR ──●── 10kΩ ── GND
              │
           ADC (GPIO34)
```

- Recommended ADC pin: **GPIO34** (input‑only, stable for ADC)

---

## 1) SensorML (Simplified for Lab Use)

For this lab, we use a **SensorML subset** containing only essential fields needed for device configuration.

### Example: `sensorml.xml`

```xml
<SensorML>
  <System>
    <identifier>LDR_ESP32_01</identifier>

    <observedProperty>LightLevel</observedProperty>
    <uom>adc_counts</uom>

    <samplingRateHz>20</samplingRateHz>

    <calibration>
      <scale>1.0</scale>
      <offset>0.0</offset>
    </calibration>

    <uncertainty>0.05</uncertainty>
  </System>
</SensorML>
```

### Practical Meaning

| SensorML Field | Used for |
|---------------|---------|
| `samplingRateHz` | ADC reading interval |
| `scale / offset` | Calibration of raw ADC values |
| `uom` | Unit attached to sensor data |
| `uncertainty` | Confidence / safety margin for CPS |

---

## 2) ESP32 Arduino Code  
### Parsing SensorML and Reading LDR

Since full XML parsing is heavy for introductory labs, this example uses **string‑based tag extraction** to demonstrate the concept while remaining practical.

```cpp
/*
  Lab: SensorML -> ESP32 (LDR)
  - Parse simplified SensorML XML (string-based)
  - Use samplingRateHz, scale, offset, uncertainty
  - Read LDR on ADC pin and print calibrated output + metadata
*/

#include <Arduino.h>

static const int LDR_PIN = 34;  // ADC input-only pin

// ---- SensorML XML (embedded) ----
const char* SENSORML_XML = R"xml(
<SensorML>
  <System>
    <identifier>LDR_ESP32_01</identifier>
    <observedProperty>LightLevel</observedProperty>
    <uom>adc_counts</uom>
    <samplingRateHz>20</samplingRateHz>
    <calibration>
      <scale>1.0</scale>
      <offset>0.0</offset>
    </calibration>
    <uncertainty>0.05</uncertainty>
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

// ---- Parsed config ----
String sensorId, observedProperty, uom;
float samplingRateHz = 20.0f;
float scale = 1.0f;
float offset = 0.0f;
float uncertainty = 0.0f;

unsigned long samplePeriodMs = 50;
unsigned long lastSampleMs = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  analogReadResolution(12); // 0..4095

  String xml = String(SENSORML_XML);

  sensorId = getTagValue(xml, "identifier");
  observedProperty = getTagValue(xml, "observedProperty");
  uom = getTagValue(xml, "uom");

  samplingRateHz = getTagFloat(xml, "samplingRateHz", 20.0f);
  scale = getTagFloat(xml, "scale", 1.0f);
  offset = getTagFloat(xml, "offset", 0.0f);
  uncertainty = getTagFloat(xml, "uncertainty", 0.0f);

  if (samplingRateHz < 1.0f) samplingRateHz = 1.0f;
  samplePeriodMs = (unsigned long)(1000.0f / samplingRateHz);

  Serial.println("=== SensorML Parsed Config ===");
  Serial.println(sensorId);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleMs >= samplePeriodMs) {
    lastSampleMs = now;

    int raw = analogRead(LDR_PIN);
    float calibrated = raw * scale + offset;

    Serial.print("{"id":"");
    Serial.print(sensorId);
    Serial.print("","property":"");
    Serial.print(observedProperty);
    Serial.print("","value":");
    Serial.print(calibrated, 2);
    Serial.print(","uom":"");
    Serial.print(uom);
    Serial.print("","uncertainty":");
    Serial.print(uncertainty, 4);
    Serial.print(","ts_ms":");
    Serial.print(now);
    Serial.println("}");
  }
}
```

---

## 3) What Students Will Observe

- LDR readings printed as **JSON self‑describing data**
- Metadata (`id`, `property`, `uom`, `uncertainty`) travels with the value
- Sampling rate changes by editing `<samplingRateHz>` in SensorML

---

## 4) Lab Activities (Checklist)

- Run the code and verify parsed SensorML configuration
- Change `<samplingRateHz>` to 5, 10, and 50 Hz
- Modify calibration (`scale`, `offset`) and observe changes
- Vary light on the LDR and observe output changes
- (Optional) Add LED control using a threshold defined in SensorML

---

## 5) Student Deliverables

- Screenshot of Serial output (≥ 30 seconds)
- Modified SensorML file (sampling + calibration)
- Short reflection (5–10 lines):  
  **“How does SensorML improve interoperability in CPS/IoT systems?”**

---

**Key Takeaway:**  
This lab demonstrates how **SensorML enables self‑describing sensors**, allowing CPS and IoT systems to understand sensor data without hard‑coded assumptions.
