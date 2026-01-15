# ESP32 + SensorML (subset) + TinyML (TFLM) — Compile-ready Project

This project shows a **real** compile-ready structure that connects:

SensorML (subset XML) → ESP32 sensing → feature extraction → **TensorFlow Lite for Microcontrollers (TFLM)** inference → safety layer → actuator.

> Note: The included `model.h` contains a **placeholder** model byte array.  
> The project still **compiles**; at runtime it will fall back to a simple rule-based classifier if the model cannot be loaded.  
> Replace `g_model[]` with your own converted `.tflite` (flatbuffer) to enable real inference.

## Folder Structure

```
esp32_sensorml_tflm_lab/
  README.md
  src/
    main.ino
    sensorml_parser.h / .cpp
    features.h / .cpp
    model.h
    inference.h / .cpp
    controller.h / .cpp
```

## Requirements

- Arduino IDE (or PlatformIO)
- Board: ESP32
- Library: **Arduino_TensorFlowLite**
  - Arduino IDE: Library Manager → search `TensorFlowLite_ESP32` or `Arduino_TensorFlowLite` (library names vary by core).
  - If your environment uses a different TFLM package, adjust the include path in `inference.cpp`.

## How to Use

1. Open `src/main.ino` in Arduino IDE.
2. Select your ESP32 board + port.
3. Install the TFLM library.
4. Compile and upload.
5. Serial Monitor @ 115200.

## Replace the model

Convert your trained model to C array and replace `g_model[]` in `src/model.h`.

Common conversion:
- `xxd -i model.tflite > model.h`
- Or use Edge Impulse / TensorFlow tools to export as C array.

Make sure your model:
- Input: **5 float features** (mean, std, min, max, slope)
- Output: **3 classes** (dark, normal, bright)
