/*
  LM73 Temperature Trend Prediction (ESP32/Arduino)
  -------------------------------------------------
  - Reads LM73 via I2C
  - Smooths using Exponential Moving Average (EMA)
  - Computes dT/dt
  - Predicts temperature lookahead seconds ahead
  - Triggers ALERT output (or controls FAN pin) if predicted temp exceeds limit

  Notes:
  - LM73 is an I2C temperature sensor (Texas Instruments).
  - This sketch assumes an LM73 at I2C address 0x48 (common). Change if needed.
  - Temperature register is typically 0x00 (verify with your LM73 variant/datasheet).
*/

#include <Wire.h>

// ---------- User Config ----------
static const uint8_t LM73_ADDR = 0x4D;   // LM73 I2C address (change if your board differs)
static const uint8_t REG_TEMP  = 0x00;   // Temperature register (common default)

static const int PIN_ALERT = 25;         // Output pin to emulate ALERT action (ESP32 GPIO25)
static const int PIN_FAN   = 26;         // Optional: fan control pin (ESP32 GPIO26)

static const float ALPHA = 0.2f;         // EMA smoothing factor (0.1–0.3 typical)
static const float LOOKAHEAD_SEC = 5.0f; // Predict 5 seconds ahead
static const float SAFETY_LIMIT_C = 60.0f; // Safety threshold (°C)

// Sampling interval (ms)
static const uint32_t SAMPLE_MS = 200;

// ---------- State ----------
float emaTempC = NAN;
float prevEmaTempC = NAN;
uint32_t prevMs = 0;

bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t* buf, size_t len) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false; // repeated-start
  if (Wire.requestFrom((int)addr, (int)len) != (int)len) return false;
  for (size_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

/*
  Convert LM73 temperature register to Celsius.
  LM73 temp is commonly a signed value with fractional bits (depending on resolution).
  Many LM73 configurations use a 16-bit register where LSB = 0.03125°C (1/32) in 14-bit mode.
  Here we assume:
    - 16-bit signed
    - value >> 2 gives 14-bit aligned (depends on LM73 mode)
    - LSB step = 0.03125°C
  If your readings look wrong, adjust shifting/scale per datasheet configuration.
*/
float lm73RawToCelsius(int16_t raw) {
  // Assume raw contains temperature in 14-bit left-justified within 16 bits:
  // shift right by 2 to get 14-bit value (signed)
  int16_t temp14 = raw >> 2;
  return (float)temp14 * 0.03125f; // 1/32 °C
}

bool readLM73TempC(float &tempC) {
  uint8_t b[2];
  if (!i2cReadBytes(LM73_ADDR, REG_TEMP, b, 2)) return false;

  // LM73 returns MSB first in many configs
  int16_t raw = (int16_t)((b[0] << 8) | b[1]);
  tempC = lm73RawToCelsius(raw);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(4, 5); // ESP32: defaults to SDA=4, SCL=5 on many boards

  pinMode(PIN_ALERT, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_ALERT, LOW);
  digitalWrite(PIN_FAN, LOW);

  Serial.println("LM73 Trend Prediction started.");
}

void loop() {
  uint32_t now = millis();
  if (now - prevMs < SAMPLE_MS) return;

  float tempC;
  if (!readLM73TempC(tempC)) {
    Serial.println("LM73 read failed (check wiring/address).");
    prevMs = now;
    return;
  }

  // ---------- EMA Smoothing ----------
  if (isnan(emaTempC)) {
    emaTempC = tempC;
    prevEmaTempC = emaTempC;
  } else {
    emaTempC = ALPHA * tempC + (1.0f - ALPHA) * emaTempC;
  }

  // ---------- dT/dt (°C/s) ----------
  float dt = (now - prevMs) / 1000.0f;
  float dTdt = 0.0f;
  if (dt > 0.0f) dTdt = (emaTempC - prevEmaTempC) / dt;

  // ---------- Predict ----------
  float Tpred = emaTempC + dTdt * LOOKAHEAD_SEC;

  // ---------- Action ----------
  bool alert = (Tpred >= SAFETY_LIMIT_C);
  digitalWrite(PIN_ALERT, alert ? HIGH : LOW);
  digitalWrite(PIN_FAN,   alert ? HIGH : LOW); // simple: fan ON when predicted limit exceeded

  // ---------- Log ----------
  Serial.print("Raw=");
  Serial.print(tempC, 4);
  Serial.print("C | EMA=");
  Serial.print(emaTempC, 4);
  Serial.print("C | dT/dt=");
  Serial.print(dTdt, 4);
  Serial.print(" C/s | Pred(");
  Serial.print(LOOKAHEAD_SEC, 1);
  Serial.print("s)=");
  Serial.print(Tpred, 4);
  Serial.print("C | ALERT=");
  Serial.println(alert ? "ON" : "OFF");

  prevEmaTempC = emaTempC;
  prevMs = now;
}
