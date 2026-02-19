# 🤖 Dual‑Sensor TinyML Neural Network (ESP32 + LDR + LM73)

### Edge AI / Cyber‑Physical Systems (CPS) Ready README

This project demonstrates a **Dual‑Sensor TinyML Neural Network**
running on an **ESP32**, combining:

-   🌞 LDR (Light Sensor)
-   🌡️ LM73 (High‑Precision Temperature Sensor)

Instead of using simple threshold logic, the ESP32 learns
**context‑aware environmental patterns** such as:

-   Outdoor Sunlight
-   Indoor LED Lighting
-   Night Conditions
-   Warm + Bright vs Cool + Bright environments

This architecture is ideal for:

-   Edge AI
-   TinyML Education
-   Digital Twin Systems
-   Cyber‑Physical Systems (CPS)

------------------------------------------------------------------------

# 📌 System Overview

    [LDR Light Sensor]                     → Neural Network → Prediction → Action
    [LM73 Temperature] /

Dual‑sensor input allows the model to understand environmental context,
not just raw brightness.

------------------------------------------------------------------------

# 🧩 Hardware Setup

## Components

-   ESP32 Dev Board
-   LDR Sensor
-   LM73 Temperature Sensor (I2C)
-   10kΩ Resistor
-   Breadboard

## Wiring

### LDR Voltage Divider

3.3V → LDR\
LDR → GPIO34 (ADC1)\
LDR → 10kΩ → GND

### LM73 I2C

LM73 VCC → 3.3V\
LM73 GND → GND\
LM73 SDA → GPIO21\
LM73 SCL → GPIO22

------------------------------------------------------------------------

# 🧠 Neural Network Architecture

Simple Multilayer Perceptron (TinyML Optimized):

    Input Layer   : 2 nodes  (Light + Temperature)
    Hidden Layer  : 8 neurons (ReLU)
    Output Layer  : 3 neurons (Softmax)

Example Classes:

-   SUNNY_OUTDOOR
-   INDOOR_LED
-   NIGHT

------------------------------------------------------------------------

# 📊 Dataset Format

Example CSV:

    Label,Light,Temperature
    0,0.10,32.5
    0,0.12,33.1
    1,0.45,25.0
    2,0.90,22.0

Normalization:

    light_input = analogRead / 4095.0
    temp_input  = temperature_celsius / 50.0

------------------------------------------------------------------------

# 🧪 Training Script (Python Concept)

``` python
import tensorflow as tf

model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(2,)),
    tf.keras.layers.Dense(3, activation='softmax')
])
```

After training, export weights into Arduino arrays.

------------------------------------------------------------------------

# ⚡ Plain Arduino TinyML Implementation

``` cpp
#include <math.h>

float w1[8][2];   // Example: trained weights
float b1[8];
float w2[3][8];

void inference(float light, float temp){

    float input[2] = {light, temp};

    float hidden[8];
    for(int i=0;i<8;i++){
        float sum = b1[i];
        for(int j=0;j<2;j++){
            sum += input[j]*w1[i][j];
        }
        hidden[i] = (sum>0)?sum:0;
    }

    float scores[3]={0,0,0};
    for(int i=0;i<3;i++){
        for(int j=0;j<8;j++){
            scores[i]+=hidden[j]*w2[i][j];
        }
    }
}
```

------------------------------------------------------------------------

# 🚀 Why Dual‑Sensor TinyML?

  Feature                    Single LDR   Dual Sensor
  -------------------------- ------------ -------------
  Context Awareness          ❌           ✅
  Indoor/Outdoor Detection   Limited      Accurate
  CPS Intelligence           Low          High
  TinyML Capability          Basic        Advanced

------------------------------------------------------------------------

# 📡 CPS Architecture Mapping

    Sense Layer      → LDR + LM73
    Cyber Layer      → Neural Network
    Cognition Layer  → Context Classification
    Actuation Layer  → Smart Lighting / Control

This aligns with CPS 5C Architecture.

------------------------------------------------------------------------

# 🔧 Engineering Tips

## Normalize Inputs

    light = analogRead / 4095.0
    temp  = tempC / 50.0

## Quantization

Use INT8 quantization to reduce model size by \~4×.

## Calibration

Collect real environment samples before deployment.

------------------------------------------------------------------------

# 📂 Suggested Repository Structure

    DualSensor-TinyML-ESP32/
    │
    ├── README.md
    ├── Arduino/
    │   └── dual_tinyml.ino
    ├── Dataset/
    │   └── dual_sensor_data.csv
    └── Training/
        └── train_dual_model.py


