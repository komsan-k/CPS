# Smart City CPS Lab — TinyML Congestion Predictor (ESP32 Buttons Mock Sensors, 10s Window)

This package implements **Option A**: a **3-class congestion predictor** (`LOW / MEDIUM / HIGH`) using **TinyML (TFLite Micro) on ESP32**, with **button mock sensors**:

- **GPIO14** = “vehicle detected” for **North–South (NS)**
- **GPIO16** = “vehicle detected” for **East–West (EW)**
- **1 press = 1 vehicle**

Every **10 seconds**, the ESP32 aggregates vehicle counts and:
1) Publishes the window counts to MQTT (`city/intersection1/traffic_10s`)
2) Runs TinyML inference to predict **next-window** congestion class and publishes prediction (`city/intersection1/prediction`)

---

## 1) MQTT Topics

### Publish (from ESP32)
- `city/intersection1/traffic_10s`
  ```json
  {
    "ts_ms": 12345678,
    "ns_10s": 7,
    "ew_10s": 3,
    "hist_ns": [7, 5, 2],
    "hist_ew": [3, 4, 1],
    "window_ms": 10000
  }
  ```
- `city/intersection1/prediction`
  ```json
  {"pred":"MEDIUM","cls":1}
  ```

### Optional Subscribe (to ESP32)
- `city/intersection1/control` (if you later add adaptive traffic light control)
  ```json
  {"green_ns":40,"green_ew":30}
  ```

---

## 2) Labeling Rule (Ground Truth for Training)

For training, compute:

`total_10s = ns_10s + ew_10s`

Then label:
- `LOW (0)` if `total_10s <= 3`
- `MEDIUM (1)` if `4 <= total_10s <= 8`
- `HIGH (2)` if `total_10s >= 9`

**Prediction target:** the class of the **next** window (`t+1`).

---

## 3) Features (Input Vector)

Use last 3 windows to capture trend:

`[ns0, ew0, ns1, ew1, ns2, ew2]`

Where:
- `ns0, ew0` = current window counts
- `ns1, ew1` = previous window
- `ns2, ew2` = two windows ago

### Normalization (simple + consistent)
Assume a realistic maximum presses per 10s per direction, e.g.:

`MAX_COUNT_PER_WINDOW = 20`

Normalize each count:

`x_norm = clamp(count, 0, MAX) / MAX`

This avoids needing `x_min/x_max` on device.

---

## 4) Training (Python)

1) Collect data (easiest): use Node-RED flow to log a CSV dataset.
2) Train model and export int8 TFLite: run:

```bash
python train_traffic_classifier.py --csv traffic_dataset.csv --out traffic_cls_int8.tflite
```

3) Convert `.tflite` to C array:

```bash
xxd -i traffic_cls_int8.tflite > traffic_model_data.cc
```

4) In Arduino project:
- Rename to `traffic_model_data.cpp`
- Create a small header `traffic_model_data.h` (example provided below)
- Include it in `esp32_traffic_tinyml_10s.ino`

---

## 5) ESP32 Firmware

Open `esp32_traffic_tinyml_10s.ino` in Arduino IDE.
- Set Wi-Fi and MQTT parameters
- Add TFLM library (Arduino: **TensorFlowLite_ESP32** or **tflite-micro** bundle as your course uses)
- Add model files `traffic_model_data.cpp/.h`

---

## 6) Node-RED

Import `node-red_flow_smart_traffic_logger_and_dashboard.json`

It provides:
- MQTT input for `traffic_10s`
- A simple dashboard (counts + predicted class)
- CSV logging (dataset builder)

---

## 7) File List

- `esp32_traffic_tinyml_10s.ino` — ESP32 buttons → 10s counts → TinyML prediction → MQTT
- `train_traffic_classifier.py` — train MLP (6→8→3) and export int8 TFLite
- `node-red_flow_smart_traffic_logger_and_dashboard.json` — Node-RED flow (logging + dashboard)
- `traffic_model_data.h` — header template for the model array

---

## 8) Notes for Labs

- If students press too fast, increase `DEBOUNCE_MS`.
- If counts are usually small, adjust thresholds or MAX_COUNT_PER_WINDOW.
- For more realism, add “queue estimate” and green time as extra features later (8–10 features).

Good luck with your Smart City CPS lab!
