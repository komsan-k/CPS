# Lab: SensorML → ESP32 (MPU6050 Self-Describing IMU)

## Objective

This lab demonstrates how an **Inertial Measurement Unit (IMU)** can be described using a **SensorML (subset)** and how an ESP32 can read this metadata and apply it in real operation.

By the end of this lab, students will be able to:
- Describe an **IMU (MPU6050)** using SensorML metadata
- Configure ESP32 behavior from SensorML:
  - Sampling rate
  - Accelerometer and gyroscope ranges
  - Per-axis calibration (scale / offset)
  - Units and uncertainty
- Read accelerometer and gyroscope data via **I2C**
- Output **self-describing JSON** containing IMU data with metadata

---

## 1) Hardware Wiring (I2C)

### ESP32 ↔ MPU6050 Connections

| MPU6050 | ESP32 |
|-------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |
| INT | Not used in this lab |

> **Note:**  
> For common GY-521 modules, the I2C address is **0x68** (AD0 = 0).  
> Pull-up resistors on SDA/SCL are usually included on the module.

---

## 2) SensorML Subset for MPU6050

The SensorML subset focuses on parameters that are **easy to understand and directly usable** in practice: sampling rate, ranges, per-axis calibration, units, and uncertainty.

### Example: `sensorml.xml`

```xml
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
```

---

## 3) Practical Mapping (Conceptual)

| SensorML Field | Meaning in ESP32 |
|---------------|------------------|
| `samplingRateHz` | IMU read interval |
| `rangeG` | Accelerometer full-scale range |
| `rangeDps` | Gyroscope full-scale range |
| `scaleX/Y/Z` | Per-axis calibration |
| `offsetX/Y/Z` | Bias compensation |
| `uom` | Units attached to data |
| `uncertainty` | Measurement reliability |

---

## 4) ESP32 Arduino Implementation (Overview)

The ESP32 firmware performs the following steps:

1. Parse SensorML (subset) using string-based tag extraction  
2. Configure MPU6050:
   - Accelerometer range
   - Gyroscope range
   - Sampling period
3. Read IMU data via **Adafruit_MPU6050** library
4. Apply per-axis calibration
5. Output **self-describing JSON** including metadata

### JSON Output Example

```json
{
  "id":"MPU6050_ESP32_01",
  "ts_ms":123456,
  "accel":{"uom":"m/s2","unc":0.15,"x":0.12,"y":-0.03,"z":9.78},
  "gyro":{"uom":"deg/s","unc":1.0,"x":0.5,"y":-0.1,"z":0.0},
  "temp_c":31.2
}
```

---

## 5) Parser Note (Important for Learning)

The example implementation uses **simple tag-based parsing**.  
Because `<accelerometer>` and `<gyroscope>` contain tags with the same names, full separation is limited.

> For advanced labs, a **block-based parser**  
> (`<accelerometer>...</accelerometer>`, `<gyroscope>...</gyroscope>`)  
> can be introduced while keeping the concept accessible to students.

---

## 6) Lab Activities (Checklist)

- Run the program and verify parsed SensorML configuration
- Change `<samplingRateHz>` to **10, 50, 100**
- Move the board and observe accelerometer and gyroscope responses
- Apply calibration:
  - Adjust Z-axis offset to reduce bias when stationary
- *(Optional CPS rule)*  
  If `|gyro|` exceeds a threshold → trigger LED or buzzer (motion/vibration detection)

---

## 7) Deliverables

Students must submit:
- Modified **SensorML** (sampling rate, ranges, calibration)
- Serial log:
  - 10 s stationary
  - 10 s in motion
- Short reflection (5–10 lines):  
  **“How does SensorML make IMU data interoperable in CPS/IoT systems?”**

---

## Key Takeaway

This lab shows how **SensorML transforms raw IMU signals into interoperable, self-describing data**, enabling robust integration of motion sensing into Cyber-Physical and IoT systems.
