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
