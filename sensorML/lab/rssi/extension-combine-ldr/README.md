# Combined Lab: Multi-Sensor SensorML → ESP32 (LDR + Wi‑Fi RSSI Self‑Describing Sensors)

## Overview

This lab combines **two sensors** on an ESP32:
1. **LDR (physical sensor)** read via ADC
2. **Wi‑Fi RSSI (virtual sensor)** read from the radio subsystem

Both sensors are described using a **simplified SensorML subset**, enabling the ESP32 to publish **self‑describing data** with:
- sensor identifiers
- observed properties
- units
- sampling rates
- calibration (scale/offset)
- uncertainty

---

## Learning Outcomes

Students will be able to:
- Explain how SensorML acts as **metadata** for both physical and virtual sensors
- Parse a simplified SensorML configuration for **multiple sensors**
- Configure **different sampling rates** per sensor
- Apply calibration and uncertainty in data reporting
- Output combined readings as **self‑describing JSON** (Serial, MQTT optional)

---

## Hardware / Wokwi

### Required
- ESP32 DevKit
- LDR
- 10kΩ resistor (voltage divider)

### Optional
- LED (status / warning)

### LDR Wiring
```
3.3V ── LDR ──●── 10kΩ ── GND
              │
           ADC (GPIO34)
```

---

## SensorML Subset (Multi‑Sensor)

In real SensorML, multiple sensors can be modeled as systems/components.  
For teaching, we use a **simple multi‑sensor XML** that contains two sensor blocks.

Save as `sensorml.xml` (or embed in code as a string):

```xml
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
```

### Practical Mapping
- LDR uses ADC sampling at **20 Hz**
- RSSI is read at **2 Hz**
- Each sensor has independent **scale/offset** and **uncertainty**
- `targetSSID` documents which Wi‑Fi network RSSI refers to

---

## Output Format (Self‑Describing JSON)

Publish/print one JSON object per measurement:

### LDR Telemetry Example
```json
{
  "id":"LDR_ESP32_01",
  "property":"LightLevel",
  "value":1780.25,
  "uom":"adc_counts",
  "uncertainty":0.05,
  "ts_ms":123456
}
```

### RSSI Telemetry Example
```json
{
  "id":"WIFI_RSSI_ESP32_01",
  "property":"WiFiRSSI",
  "value":-55.0,
  "uom":"dBm",
  "uncertainty":2.0,
  "ssid":"YourWiFiSSID",
  "ts_ms":123999
}
```

---

## Suggested Implementation Plan (ESP32)

### Step 1: Parse Multi‑Sensor SensorML
- Parse two sensor blocks into two configs:
  - `cfgLdr`
  - `cfgRssi`

### Step 2: Two Sampling Timers
- Use independent timers:
  - LDR timer based on `cfgLdr.samplingRateHz`
  - RSSI timer based on `cfgRssi.samplingRateHz`

### Step 3: Read + Calibrate + Output
- LDR: `analogRead(GPIO34)` → apply scale/offset → JSON
- RSSI: `WiFi.RSSI()` → apply scale/offset → JSON

---

## Lab Activities (Checklist)

1. Run the combined lab and verify both sensors print JSON lines
2. Change LDR `<samplingRateHz>` (10, 20, 50) and observe output frequency
3. Change RSSI `<samplingRateHz>` (1, 2, 5) and observe output frequency
4. Validate RSSI changes by moving closer/farther from the router
5. Apply calibration:
   - LDR offset = −50
   - RSSI offset = +3
6. *(Optional CPS rule)*  
   If RSSI < −75 dBm → print warning or turn on LED

---

## Deliverables

Students must submit:
- Modified multi‑sensor SensorML (both sensors)
- Serial log (≥ 30 seconds) showing:
  - LDR change with light
  - RSSI change with distance
- Short reflection (5–10 lines):  
  **“How does SensorML support interoperability for multi‑sensor CPS/IoT devices?”**

---

## Extension Ideas (Optional)

- Publish both sensors via MQTT:
  - `device/ldr01/telemetry`
  - `device/rssi01/telemetry`
  - or a unified topic `device/esp32_01/telemetry`
- Node‑RED dashboard:
  - LDR gauge + chart
  - RSSI gauge + chart
  - metadata view (retained)

---

**Key Takeaway:**  
A multi‑sensor ESP32 can become a **self‑describing CPS node** when SensorML metadata is used to standardize configuration and meaning of both physical and virtual sensor data.
