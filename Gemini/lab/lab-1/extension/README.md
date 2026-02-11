# Lab Extension: Add LM73 Temperature Sensor 

## Goal

Upgrade the existing **ESP32 LDR → MQTT (JSON) + LED control** lab to
also: - Read **LM73 temperature** over I²C - Publish temperature in the
SAME JSON message - Keep the safe publish pattern:

    mqtt.publish(topic, (const uint8_t*)out, n, false);

------------------------------------------------------------------------

## 1) Hardware Upgrade (LM73 wiring)

### LM73 ↔ ESP32 (I²C)

  LM73   ESP32
  ------ ----------------------------
  VDD    3.3V
  GND    GND
  SDA    GPIO4
  SCL    GPIO5
  ADDR   0x4D


------------------------------------------------------------------------

## 2) Add Configuration Constants

Add near USER CONFIG:

``` cpp
#include <Wire.h>

const uint8_t LM73_ADDR = 0x4D;
const float LM73_SCALE  = 1.0f;
const float LM73_OFFSET = 0.0f;
const float LM73_UNC    = 1.0f;
```

------------------------------------------------------------------------

## 3) Initialize I²C in setup()

``` cpp
Wire.begin(4, 5);
```

------------------------------------------------------------------------

## 4) LM73 Read Function

``` cpp
static bool readLM73C(float &tempC) {
  Wire.beginTransmission(LM73_ADDR);
  Wire.write(0x00);
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom((int)LM73_ADDR, 2) != 2) return false;

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();

  int16_t raw = (int16_t)((msb << 8) | lsb);
  int16_t temp14 = raw >> 2;
  tempC = temp14 * 0.03125f;
  return true;
}
```

------------------------------------------------------------------------

## 5) Extend JSON Payload

Inside loop():

``` cpp
float tempC = NAN;
bool okLM73 = readLM73C(tempC);

float tempCal = NAN;
if (okLM73) tempCal = tempC * LM73_SCALE + LM73_OFFSET;

doc["temp_ok"] = okLM73;
if (okLM73) {
  doc["temp_c"] = tempCal;
  doc["temp_uom"] = "degC";
  doc["temp_unc"] = LM73_UNC;
} else {
  doc["temp_err"] = "LM73 read failed";
}
```

------------------------------------------------------------------------

## 6) Optional CPS Rule

``` cpp
const float TEMP_ALARM = 35.0f;

if (okLM73 && tempCal > TEMP_ALARM) {
  setLed(true);
}
```

------------------------------------------------------------------------

## 7) Node-RED Dashboard Update

Add widgets: - MQTT IN topic: `cps/ldr/data` - JSON node - Gauge →
`msg.payload.temp_c` - Chart → temperature vs time

------------------------------------------------------------------------

## 8) Deliverables

1.  Dashboard screenshot (LDR + RSSI + Temperature)
2.  Serial output (30 seconds)
3.  Short explanation:
    -   What does `temp_unc` mean?
4.  Calibration test:
    -   change `LM73_OFFSET` and observe difference


