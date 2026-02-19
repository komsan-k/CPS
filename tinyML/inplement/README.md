# 🤖 TinyML Neural Network for ESP32 + LDR

### Edge Impulse + TensorFlow Lite + Plain Arduino TinyML Guide

This project demonstrates how to implement **TinyML (Tiny Machine
Learning)** on an **ESP32** using an **LDR (Light Dependent Resistor)**
sensor.

Instead of fixed threshold logic like:

    if(light < 500) turn_on_led();

TinyML allows the ESP32 to **learn lighting patterns** such as:

-   Natural sunset
-   Artificial indoor light
-   Hand shadows
-   Sunny / Office / Night classification

This README provides a full workflow using:

-   Edge Impulse (Beginner Friendly)
-   TensorFlow Lite Micro (Neural Network)
-   Plain Arduino TinyML (Hardcoded Neural Network)

------------------------------------------------------------------------

# 🧩 Hardware Setup

## Components

-   ESP32
-   LDR Sensor
-   10kΩ Resistor

## Voltage Divider Wiring

ESP32 ADC reads voltage, not resistance.

    3.3V → LDR pin 1
    LDR pin 2 → GPIO34 (ADC1)
    LDR pin 2 → 10kΩ → GND

------------------------------------------------------------------------

# 🧠 TinyML Workflow

## Step A --- Data Collection

Create a dataset of raw LDR values.

Example Classes:

-   Dark
-   Dim
-   Indoor Light
-   Flashlight

Simple Arduino data logger:

    Serial.println(analogRead(34));
    delay(100);

Use **Edge Impulse Data Forwarder** to upload data to the cloud.

------------------------------------------------------------------------

## Step B --- Training (Edge Impulse)

Inside Edge Impulse Studio:

1.  Create Impulse → Time Series (1000 ms window)
2.  Processing Block → Spectral Analysis
3.  Learning Block → Classification (Keras)

The AI learns the signal signature of lighting conditions.

------------------------------------------------------------------------

## Step C --- Deployment

1.  Go to Deployment tab
2.  Select **Arduino Library**
3.  Click Build
4.  Download .zip library

This generates a quantized TinyML model optimized for ESP32.

------------------------------------------------------------------------

# ⚡ ESP32 Inference Code (Edge Impulse)

``` cpp
#include <Your_Project_Inferencing.h>

void setup() {
    Serial.begin(115200);
}

void loop() {

    float raw_value = analogRead(34);

    float buffer[1] = { raw_value };
    signal_t signal;
    numpy::signal_from_buffer(buffer, 1, &signal);

    ei_impulse_result_t result = { 0 };
    run_classifier(&signal, &result, false);

    Serial.print("Prediction: ");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.printf("%s: %.2f  ",
            result.classification[ix].label,
            result.classification[ix].value);
    }
    Serial.println();

    delay(200);
}
```

------------------------------------------------------------------------

# 🧠 Neural Network TinyML (TensorFlow Lite)

## Architecture

    Input Layer : 1 node (LDR value)
    Hidden Layer: 8–16 neurons (ReLU)
    Output Layer: 3 neurons (Softmax)

### Training Example (Python)

``` python
import tensorflow as tf
from tensorflow.keras import layers

model = tf.keras.Sequential([
    layers.Dense(16, activation='relu', input_shape=(1,)),
    layers.Dense(8, activation='relu'),
    layers.Dense(3, activation='softmax')
])
```

Quantization reduces model size by \~4× for ESP32 deployment.

------------------------------------------------------------------------

# 🚀 Plain Arduino TinyML (Hardcoded Neural Network)

No TensorFlow Lite required.

``` cpp
#include <math.h>

const int LDR_PIN = 34;

float w1[8] = {-0.5,0.8,1.2,-0.3,0.6,-1.1,0.4,0.9};
float b1[8] = {0.1,-0.1,0.2,0.0,0.1,-0.2,0.1,0.1};

float w2[3][8] = {
 {0.5,-0.2,0.8,-0.1,0.4,-0.6,0.2,0.1},
 {-0.1,0.6,-0.2,0.9,-0.3,0.2,0.5,-0.4},
 {-0.4,-0.5,-0.1,-0.2,0.1,0.9,-0.7,0.8}
};
```

Advantages:

-   Very low memory usage (\~2KB)
-   Extremely fast inference
-   No external dependencies

------------------------------------------------------------------------

# 📊 Why TinyML Instead of Threshold Logic?

  Feature             Threshold   TinyML
  ------------------- ----------- --------
  Context Awareness   ❌          ✅
  Noise Robustness    ❌          ✅
  Pattern Learning    ❌          ✅
  Edge AI Ready       ❌          ✅

------------------------------------------------------------------------

# 🔧 Engineering Tips

## Normalization

    input = analogRead / 4095.0

## Multi-Sensor Fusion

Add LM73 temperature sensor:

    Input Layer = 2 nodes (Light + Temperature)

------------------------------------------------------------------------

# 📡 CPS Pipeline Concept

    Sense → Learn → Predict → Act

Perfect for:

-   Cyber-Physical Systems (CPS)
-   Edge AI
-   Smart Lighting
-   Digital Twin Labs

------------------------------------------------------------------------

# 📜 License

Educational and research usage recommended.
