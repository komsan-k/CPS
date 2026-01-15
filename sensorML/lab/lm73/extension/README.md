# Combined Lab: Multi-Sensor SensorML → ESP32 (LDR + Wi‑Fi RSSI + LM73)

## Overview

This lab builds a **multi-sensor CPS node** on ESP32 with:

1) **LDR** (physical sensor via ADC)  
2) **Wi‑Fi RSSI** (virtual sensor from radio subsystem)  
3) **LM73 Temperature** (physical sensor via I2C)

All sensors are described by a **simplified SensorML subset**, so the ESP32 can:
- Configure **sampling rates** per sensor
- Apply **calibration** (`scale`, `offset`)
- Attach **unit** (`uom`) and **uncertainty**
- Output **self-describing JSON** messages (value + metadata)

---

## Learning Outcomes

Students will be able to:
- Explain SensorML as **metadata**, not measurements
- Model a multi-sensor device using a **SensorML subset**
- Implement **independent sampling timers** for multiple sensors
- Calibrate data using `scale/offset`
- Publish **self-describing JSON** suitable for CPS/IoT pipelines (Serial, MQTT optional)

---

## Hardware / Wokwi

### Required
- ESP32 DevKit
- LDR + 10kΩ resistor (voltage divider)
- LM73 temperature sensor (I2C)

### Optional
- LED for warnings (RSSI weak / temperature high)

---

## Wiring

### 1) LDR (ADC)
```
3.3V ── LDR ──●── 10kΩ ── GND
              │
           ADC (GPIO34)
```

### 2) LM73 (I2C)
| LM73 | ESP32 |
|---|---|
| VDD | 3.3V |
| GND | GND |
| SDA (SMBDAT) | GPIO21 |
| SCL (SMBCLK) | GPIO22 |
| ADDR | GND / floating / VDD (depends on module) |

> If pull-ups are not present on the LM73 module, add **4.7kΩ** pull-ups on SDA/SCL to 3.3V.

---

## SensorML Subset (3 Sensors)

Save as `sensorml.xml` or embed as a string in code.

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
```

---

## Output Format (Self‑Describing JSON)

The ESP32 prints **one JSON object per sensor reading**.

### LDR example
```json
{"id":"LDR_ESP32_01","property":"LightLevel","value":1780.25,"uom":"adc_counts","uncertainty":0.05,"ts_ms":123456}
```

### RSSI example
```json
{"id":"WIFI_RSSI_ESP32_01","property":"WiFiRSSI","value":-55.0,"uom":"dBm","uncertainty":2.0,"ssid":"YourWiFiSSID","ts_ms":123999}
```

### LM73 example
```json
{"id":"LM73_ESP32_01","property":"Temperature","value":31.125,"uom":"degC","uncertainty":1.0,"i2c_addr":"0x48","ts_ms":124200}
```

---

## LM73 Reading Notes (used in code)

- Temperature register pointer: `0x00`
- Read 2 bytes: MSB, LSB
- Bits 15..2 contain signed temperature value
- Conversion:
  - `temp14 = (int16_t(raw) >> 2)`
  - `tempC = temp14 * 0.03125`

---

## Lab Activities (Checklist)

1. Run the combined program and verify:
   - LDR JSON lines appear
   - RSSI JSON lines appear after Wi‑Fi connects
   - LM73 address is detected by I2C scan
2. Change sampling rates in SensorML:
   - LDR: 10 / 20 / 50 Hz
   - RSSI: 1 / 2 / 5 Hz
   - LM73: 1 / 2 / 5 Hz
3. Experiments:
   - LDR: cover/uncover sensor (value changes)
   - RSSI: walk closer/farther from AP (RSSI changes)
   - LM73: touch/warm sensor (temperature rises)
4. Calibration experiments:
   - LDR offset = −50
   - RSSI offset = +3
   - LM73 offset = +0.50 °C
5. Optional CPS rules:
   - If RSSI < −75 dBm → LED ON
   - If Temp > 35 °C → LED ON

---

## Deliverables

Students submit:
- Updated multi-sensor SensorML (3 sensors)
- Serial log (≥ 30 seconds) showing variations for:
  - light, distance-to-AP, and temperature
- Reflection (5–10 lines):  
  **“How does SensorML support interoperability for multi-sensor CPS/IoT devices?”**

---

## Files Provided

- `ESP32_SensorML_MultiSensor_LDR_RSSI_LM73.ino` (complete compile-ready Arduino code)
