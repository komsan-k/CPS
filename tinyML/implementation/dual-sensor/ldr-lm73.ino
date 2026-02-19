/*
  Plain TinyML (Hardcoded Neural Network)
  ESP32 + LDR (ADC) + LM73 (I2C)
  ---------------------------------------
  - Inputs:  light_norm (0..1), temp_norm (0..1)
  - NN:      2 -> 8 (ReLU) -> 3 (scores) -> softmax
  - Output:  class label + confidence
  - Includes: simple confidence hysteresis to prevent flicker

  Wiring:
  - LDR divider: 3.3V -> LDR -> GPIO34 -> 10k -> GND
  - LM73: VCC 3.3V, GND, SDA GPIO21, SCL GPIO22

  Adjust:
  - LM73 I2C address (0x48 common)
  - temp conversion (assumes 14-bit mode scaling)
*/

#include <Wire.h>
#include <math.h>

// ------------------ Pins & I2C ------------------
static const int LDR_PIN = 34;      // ADC1
static const uint8_t LM73_ADDR = 0x48;
static const uint8_t LM73_REG_TEMP = 0x00;

// ------------------ Classes ------------------
static const int NUM_CLASSES = 3;
static const char* LABELS[NUM_CLASSES] = {
  "SUNNY_OUTDOOR",
  "INDOOR_LED",
  "NIGHT"
};

// ------------------ Normalization ------------------
// LDR: ADC 0..4095 -> 0..1
// Many LDR dividers yield higher ADC in the dark, so we flip (optional).
static inline float norm_light(int adc) {
  float x = (float)adc / 4095.0f;
  return 1.0f - x; // flip: bright -> higher value
}

// Temp: map Celsius -> 0..1 (choose maxTemp for your environment)
// Example: 0..50°C => 0..1
static inline float norm_temp(float celsius) {
  float x = celsius / 50.0f;
  if (x < 0) x = 0;
  if (x > 1) x = 1;
  return x;
}

// ------------------ LM73 Read ------------------
bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t* buf, size_t len) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false; // repeated-start
  if (Wire.requestFrom((int)addr, (int)len) != (int)len) return false;
  for (size_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

// LM73 temp register conversion (assumes 14-bit, 0.03125°C per LSB after >>2)
float lm73RawToCelsius(int16_t raw) {
  int16_t temp14 = raw >> 2;        // align 14-bit signed
  return (float)temp14 * 0.03125f;  // 1/32 °C
}

bool readLM73C(float &tempC) {
  uint8_t b[2];
  if (!i2cReadBytes(LM73_ADDR, LM73_REG_TEMP, b, 2)) return false;
  int16_t raw = (int16_t)((b[0] << 8) | b[1]); // MSB first
  tempC = lm73RawToCelsius(raw);
  return true;
}

// ------------------ Plain Neural Network ------------------
// NN: 2 inputs -> 8 hidden -> 3 outputs
// Replace these arrays with weights from your training script.

// Hidden layer weights: w1[8][2]
float w1[8][2] = {
  {  0.30f, -0.10f},
  { -0.25f,  0.40f},
  {  0.50f,  0.20f},
  {  0.10f, -0.35f},
  { -0.60f,  0.15f},
  {  0.45f,  0.05f},
  { -0.20f, -0.30f},
  {  0.15f,  0.55f}
};
float b1[8] = {0.05f, -0.02f, 0.01f, 0.00f, 0.03f, -0.01f, 0.02f, 0.04f};

// Output weights: w2[3][8]
float w2[3][8] = {
  { 0.20f, -0.10f, 0.40f, 0.05f, -0.20f, 0.10f, -0.05f, 0.15f }, // SUNNY_OUTDOOR
  { -0.05f, 0.30f, -0.10f, 0.35f, 0.10f, -0.05f, 0.25f, -0.15f }, // INDOOR_LED
  { -0.20f, -0.15f, -0.05f, -0.10f, 0.30f, -0.10f, 0.05f, 0.25f }  // NIGHT
};
float b2[3] = {0.00f, 0.00f, 0.00f};

static inline float relu(float x) { return (x > 0) ? x : 0; }

void softmax3(const float in[3], float out[3]) {
  // stable softmax
  float m = in[0];
  if (in[1] > m) m = in[1];
  if (in[2] > m) m = in[2];

  float e0 = expf(in[0] - m);
  float e1 = expf(in[1] - m);
  float e2 = expf(in[2] - m);
  float s = e0 + e1 + e2;

  out[0] = e0 / s;
  out[1] = e1 / s;
  out[2] = e2 / s;
}

int argmax3(const float p[3]) {
  int k = 0;
  if (p[1] > p[k]) k = 1;
  if (p[2] > p[k]) k = 2;
  return k;
}

void nn_predict(float light_norm, float temp_norm, float probs_out[3]) {
  float hidden[8];

  // hidden = ReLU(w1 * input + b1)
  for (int i = 0; i < 8; i++) {
    float sum = b1[i];
    sum += w1[i][0] * light_norm;
    sum += w1[i][1] * temp_norm;
    hidden[i] = relu(sum);
  }

  // scores = w2 * hidden + b2
  float scores[3] = {b2[0], b2[1], b2[2]};
  for (int c = 0; c < 3; c++) {
    for (int j = 0; j < 8; j++) scores[c] += w2[c][j] * hidden[j];
  }

  // probabilities
  softmax3(scores, probs_out);
}

// ------------------ Hysteresis / Debounce ------------------
static int stableClass = 0;
static uint8_t stableCount = 0;
static const float CONF_TH = 0.80f;       // require >=80% confidence
static const uint8_t HOLD_N = 5;          // require N consecutive loops

// ------------------ Loop ------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin(); // ESP32 default SDA=21, SCL=22 on many boards
  pinMode(LDR_PIN, INPUT);

  Serial.println("Dual-Sensor Plain TinyML (LDR + LM73) started.");
}

void loop() {
  // 1) Read sensors
  int ldr_adc = analogRead(LDR_PIN);

  float tempC;
  bool okTemp = readLM73C(tempC);
  if (!okTemp) {
    Serial.println("LM73 read failed (check wiring/address).");
    delay(300);
    return;
  }

  // 2) Normalize inputs
  float x_light = norm_light(ldr_adc);
  float x_temp  = norm_temp(tempC);

  // 3) Predict
  float probs[3];
  nn_predict(x_light, x_temp, probs);

  int pred = argmax3(probs);
  float conf = probs[pred];

  // 4) Confidence hysteresis (avoid flicker)
  if (conf >= CONF_TH) {
    if (pred == stableClass) {
      if (stableCount < HOLD_N) stableCount++;
    } else {
      // candidate new class
      stableCount++;
      if (stableCount >= HOLD_N) {
        stableClass = pred;
        stableCount = 0;
      }
    }
  } else {
    // low confidence → decay counter
    if (stableCount > 0) stableCount--;
  }

  // 5) Print
  Serial.print("LDR_ADC=");
  Serial.print(ldr_adc);
  Serial.print(" light=");
  Serial.print(x_light, 3);
  Serial.print(" | LM73=");
  Serial.print(tempC, 2);
  Serial.print("C temp=");
  Serial.print(x_temp, 3);

  Serial.print(" | probs: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(LABELS[i]);
    Serial.print("=");
    Serial.print(probs[i], 3);
    Serial.print(" ");
  }

  Serial.print("| stable=");
  Serial.print(LABELS[stableClass]);
  Serial.print(" (conf=");
  Serial.print(conf, 2);
  Serial.println(")");

  delay(200);
}
