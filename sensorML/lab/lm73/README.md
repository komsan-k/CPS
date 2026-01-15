# Lab: SensorML → ESP32 (LM73 Self-Describing Temperature Sensor)

## Objective

This lab demonstrates how **SensorML (subset)** can be used to describe a **temperature sensor (LM73)** and how an ESP32 can read this metadata and apply it in real operation.

By the end of this lab, students will be able to:
- Use SensorML metadata to describe the LM73 temperature sensor
- Configure ESP32 behavior from SensorML:
  - Sampling rate
  - Calibration (scale / offset)
  - Unit, uncertainty, and sensor ID
- Read temperature data from the LM73 via **I2C**
- Output **self-describing JSON** containing both values and metadata

---

## 1) Hardware Wiring (I2C)

### ESP32 ↔ LM73 Connections

| LM73 Pin | ESP32 Pin |
|--------|-----------|
| VDD | 3.3V |
| GND | GND |
| SMBDAT (SDA) | GPIO21 |
| SMBCLK (SCL) | GPIO22 |
| ADDR | GND / floating / VDD (select address) |

> **Note:**  
> If the LM73 module does not include pull-up resistors on SDA/SCL, add **4.7 kΩ pull-ups to 3.3V**.  
> Many breakout boards already include them.

---

## 2) SensorML Subset for LM73

A minimal SensorML subset is sufficient and can be embedded directly into the ESP32 code.

### Example: `sensorml.xml`

```xml
<SensorML>
  <System>
    <identifier>LM73_ESP32_01</identifier>
    <observedProperty>Temperature</observedProperty>
    <uom>degC</uom>

    <samplingRateHz>2</samplingRateHz>

    <calibration>
      <scale>1.000</scale>
      <offset>0.00</offset>
    </calibration>

    <uncertainty>1.0</uncertainty>
  </System>
</SensorML>
```

### Practical Mapping

| SensorML Field | Used for |
|---------------|----------|
| `samplingRateHz` | Temperature sampling interval |
| `scale / offset` | Calibration (e.g., +0.5 °C bias) |
| `uncertainty` | Measurement confidence |
| `identifier / uom` | Self-describing output |

---

## 3) LM73 Temperature Reading Principle

The LM73 temperature sensor is read using the following method:

- Pointer register is set to **0x00** (Temperature Register)
- Temperature data is **2 bytes**
- Bits **15..2** contain temperature data (bits 1..0 unused)
- Conversion:
  - `temp14 = raw >> 2` (signed shift)
  - `tempC = temp14 × 0.03125`

> Resolution: **0.03125 °C per LSB** (maximum resolution mode)

---

## 4) ESP32 Arduino Code — SensorML → I2C → LM73

This implementation is **lab-safe** and student-friendly because:
- It automatically scans for the LM73 I2C address
- It applies SensorML-based sampling and calibration
- It outputs self-describing JSON

(See the provided `.ino` file for the complete implementation.)

```cpp
#include <Arduino.h>
#include <Wire.h>

// ---------- SensorML XML (embedded) ----------
const char* SENSORML_XML = R"xml(
<SensorML>
  <System>
    <identifier>LM73_ESP32_01</identifier>
    <observedProperty>Temperature</observedProperty>
    <uom>degC</uom>
    <samplingRateHz>2</samplingRateHz>
    <calibration>
      <scale>1.000</scale>
      <offset>0.00</offset>
    </calibration>
    <uncertainty>1.0</uncertainty>
  </System>
</SensorML>
)xml";

// ---------- Simple XML tag extraction ----------
String getTagValue(const String& xml, const String& tag) {
  String openTag = "<" + tag + ">";
  String closeTag = "</" + tag + ">";
  int s = xml.indexOf(openTag);
  if (s < 0) return "";
  s += openTag.length();
  int e = xml.indexOf(closeTag, s);
  if (e < 0) return "";
  return xml.substring(s, e);
}

float getTagFloat(const String& xml, const String& tag, float defVal) {
  String v = getTagValue(xml, tag);
  v.trim();
  if (v.length() == 0) return defVal;
  return v.toFloat();
}

// ---------- Parsed config ----------
String sensorId, observedProperty, uom;
float samplingRateHz = 2.0f;
float scale = 1.0f;
float offset = 0.0f;
float uncertainty = 1.0f;
unsigned long samplePeriodMs = 500;
unsigned long lastMs = 0;

// ---------- I2C / LM73 ----------
uint8_t lm73Addr = 0;

// I2C scan helper
uint8_t scanI2C() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return addr;
  }
  return 0;
}

// Read LM73 temperature register (pointer 0x00) -> degC
bool readLM73TempC(float &tempC) {
  if (lm73Addr == 0) return false;

  // Set pointer to Temperature register = 0x00
  Wire.beginTransmission(lm73Addr);
  Wire.write(0x00);
  if (Wire.endTransmission(false) != 0) return false; // repeated start

  // Read 2 bytes
  if (Wire.requestFrom((int)lm73Addr, 2) != 2) return false;
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();

  int16_t raw = (int16_t)((msb << 8) | lsb);
  int16_t temp14 = raw >> 2;               // bits 15..2 are data
  tempC = temp14 * 0.03125f;               // 0.03125 °C/LSB (max-res scale)
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(21, 22);

  // Parse SensorML (subset)
  String xml = String(SENSORML_XML);
  sensorId = getTagValue(xml, "identifier");
  observedProperty = getTagValue(xml, "observedProperty");
  uom = getTagValue(xml, "uom");
  samplingRateHz = getTagFloat(xml, "samplingRateHz", 2.0f);
  scale = getTagFloat(xml, "scale", 1.0f);
  offset = getTagFloat(xml, "offset", 0.0f);
  uncertainty = getTagFloat(xml, "uncertainty", 1.0f);

  if (samplingRateHz < 0.2f) samplingRateHz = 0.2f;
  samplePeriodMs = (unsigned long)(1000.0f / samplingRateHz);

  Serial.println("=== SensorML Parsed Config ===");
  Serial.print("id: "); Serial.println(sensorId);
  Serial.print("property: "); Serial.println(observedProperty);
  Serial.print("uom: "); Serial.println(uom);
  Serial.print("samplingRateHz: "); Serial.println(samplingRateHz, 2);
  Serial.print("scale: "); Serial.println(scale, 4);
  Serial.print("offset: "); Serial.println(offset, 4);
  Serial.print("uncertainty: "); Serial.println(uncertainty, 3);
  Serial.print("samplePeriodMs: "); Serial.println(samplePeriodMs);
  Serial.println("================================");

  // Find LM73 address
  Serial.println("Scanning I2C...");
  lm73Addr = scanI2C();
  if (lm73Addr == 0) {
    Serial.println("No I2C device found. Check wiring/pullups/ADDR.");
  } else {
    Serial.print("Found I2C device at 0x");
    Serial.println(lm73Addr, HEX);
  }
}

void loop() {
  if (lm73Addr == 0) {
    delay(1000);
    return;
  }

  unsigned long now = millis();
  if (now - lastMs < samplePeriodMs) return;
  lastMs = now;

  float tC = 0;
  if (!readLM73TempC(tC)) {
    Serial.println("{\"error\":\"LM73 read failed\"}");
    return;
  }

  // Apply calibration from SensorML
  float tCal = tC * scale + offset;

  // Self-describing JSON output
  Serial.print("{\"id\":\""); Serial.print(sensorId);
  Serial.print("\",\"property\":\""); Serial.print(observedProperty);
  Serial.print("\",\"value\":"); Serial.print(tCal, 3);
  Serial.print(",\"uom\":\""); Serial.print(uom);
  Serial.print("\",\"uncertainty\":"); Serial.print(uncertainty, 3);
  Serial.print(",\"i2c_addr\":\"0x"); Serial.print(lm73Addr, HEX);
  Serial.print("\",\"ts_ms\":"); Serial.print(now);
  Serial.println("}");
}

```
---

## 5) Lab Activities (Checklist)

- Wire the I2C circuit and run the program  
  → Verify output: `Found I2C device at 0x..`
- Change `<samplingRateHz>` to **1, 2, 5** and observe output frequency
- Test calibration:
  - Set `<offset>0.50</offset>` and observe +0.5 °C shift
- Touch or gently warm the sensor and observe temperature change
- *(Optional CPS rule)*  
  Turn on LED when **temperature > 35 °C**

---

## 6) Deliverables

Students must submit:
- Modified **SensorML** (sampling + calibration)
- Serial log (≥ 30 seconds):
  - steady temperature
  - temperature increase by touch
- Short reflection (5–10 lines):  
  **“How does SensorML improve interoperability of temperature data in CPS/IoT systems?”**

---

## Key Takeaway

This lab demonstrates how **SensorML enables self-describing temperature sensors**, allowing CPS and IoT systems to interpret temperature data consistently without hard-coded assumptions.
