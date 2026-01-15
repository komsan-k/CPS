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
