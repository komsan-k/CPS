# ESP32 LDR Data Logger (CSV) for TinyML Dataset

This project provides a **clean ESP32 data collection sketch** for an LDR sensor on GPIO36.

It logs data in **CSV format** for:
- Training datasets
- INT8 representative datasets (TFLite)

---

## ✅ Features

- Label switching via Serial (`dark`, `normal`, `bright`)
- CSV output format:
  ```
  timestamp_ms,adc,normalized,label
  ```
- Moving average smoothing (noise reduction)
- Fixed sampling interval (default: 100 ms)

---

## 📟 Arduino Sketch

Save as: `esp32_ldr_logger_csv.ino`

```cpp
#include <Arduino.h>

static const int LDR_PIN = 36;
static const uint32_t SAMPLE_MS = 100;

// Moving average window (set 1 to disable)
static const int MA_WINDOW = 8;

int ma_buf[MA_WINDOW];
int ma_idx = 0;
long ma_sum = 0;

String current_label = "dark";

static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

int read_adc_ma() {
  int raw = analogRead(LDR_PIN);

  ma_sum -= ma_buf[ma_idx];
  ma_buf[ma_idx] = raw;
  ma_sum += raw;
  ma_idx = (ma_idx + 1) % MA_WINDOW;

  return (int)(ma_sum / MA_WINDOW);
}

void print_help() {
  Serial.println("\n=== ESP32 LDR CSV Logger ===");
  Serial.println("Commands:");
  Serial.println("dark | normal | bright");
  Serial.println("label? | header | help");
  Serial.println("\nCSV:");
  Serial.println("timestamp_ms,adc,normalized,label\n");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  analogReadResolution(12);

  for (int i = 0; i < MA_WINDOW; i++) ma_buf[i] = 0;

  print_help();
  Serial.println("timestamp_ms,adc,normalized,label");
}

void handle_serial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "dark" || cmd == "normal" || cmd == "bright") {
    current_label = cmd;
    Serial.printf("# label: %s\n", current_label.c_str());
  }
}

void loop() {
  static uint32_t last_ms = 0;

  handle_serial();

  uint32_t now = millis();
  if (now - last_ms < SAMPLE_MS) return;
  last_ms = now;

  int adc = read_adc_ma();
  float x = clamp01((float)adc / 4095.0f);

  Serial.printf("%lu,%d,%.6f,%s\n",
                (unsigned long)now, adc, x, current_label.c_str());
}
```

---

## 🧪 Data Collection Workflow

1. Open Serial Monitor (115200 baud)

2. Set environment → type:
```
dark
```
Collect ~200–500 samples

3. Change lighting:
```
normal
```

4. Bright condition:
```
bright
```

5. Copy all CSV lines → save as:
```
ldr_dataset.csv
```

---

## 🧠 Representative Dataset (INT8 Quantization)

Only the **normalized column** is required.

### Python Example

```python
import numpy as np
import pandas as pd

df = pd.read_csv("ldr_dataset.csv", comment="#")
X = df["normalized"].astype(np.float32).values.reshape(-1, 1)

def representative_dataset():
    for i in range(min(len(X), 500)):
        yield [X[i:i+1]]
```

---

## 🚀 Applications

- TinyML training dataset generation
- ESP32 edge AI experiments
- CPS digital twin calibration
- Smart lighting systems

---

## 📌 Tips

- Use real data → better INT8 accuracy
- Increase sampling if noisy
- Reduce MA_WINDOW for faster response

---

## ✅ Summary

This tool provides a **clean pipeline from real sensor → dataset → TinyML model**.

Perfect for:
- ESP32 + TFLM labs
- CPS + AI integration
- Research prototyping
