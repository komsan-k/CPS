# ESP32 + TensorFlow Lite for Microcontrollers (TFLM) Lab Manual  
## TinyML for Cyber-Physical Systems (CPS)

---

## 📘 1. Introduction

TinyML enables machine learning inference on resource-constrained embedded devices such as microcontrollers.

**TensorFlow Lite for Microcontrollers (TFLM)** is a lightweight runtime designed for such platforms.

Using **ESP32 + TFLM**, we can build **edge AI systems** that operate locally without cloud dependency.

### This lab demonstrates:
- Sensor data collection
- Neural network training
- Model conversion to TFLite
- Deployment on ESP32
- Real-time inference
- CPS integration

---

## 🎯 2. Learning Objectives

After completing this lab, students will be able to:

- Understand the TinyML pipeline  
- Train neural networks using TensorFlow  
- Convert models to TensorFlow Lite  
- Deploy models on ESP32  
- Perform real-time inference  
- Integrate TinyML into CPS  

---

## 🔌 3. Hardware Requirements

| Component | Description |
|----------|------------|
| ESP32 DevKit | Main microcontroller |
| LDR Sensor | Light measurement |
| 10kΩ Resistor | Voltage divider |
| Breadboard | Prototyping |
| USB Cable | Programming |

---

## 💻 4. Software Requirements

| Software | Purpose |
|---------|--------|
| Arduino IDE | ESP32 programming |
| Python 3 | Model training |
| TensorFlow | Neural network |
| TensorFlow Lite | Model conversion |
| Node-RED (optional) | CPS dashboard |

---

## 🏗️ 5. System Architecture

```
Sensor → ADC → Preprocessing → TFLM → Prediction → Actuator / MQTT / Dashboard
```

---

## 🔧 6. Hardware Setup

```
3.3V
 |
[LDR]
 |
 +------> GPIO36 (ADC)
 |
[10kΩ]
 |
GND
```

---

## 📊 7. Data Collection

Example CSV:

```
timestamp,adc,normalized,label
100,200,0.05,dark
200,1200,0.29,normal
300,3500,0.85,bright
```

### Recommended Dataset Size

| Class | Samples |
|------|--------|
| Dark | 300 |
| Normal | 300 |
| Bright | 300 |

---

## 🧠 8. Neural Network Model

**1–8–3 MLP**

- Input: 1 neuron  
- Hidden: 8 neurons  
- Output: 3 neurons  

| Output | Meaning |
|--------|--------|
| 0 | Dark |
| 1 | Normal |
| 2 | Bright |

---

## 🐍 9. Python Training

```python
import tensorflow as tf
import numpy as np

X = np.random.rand(1000,1)

y = np.zeros((1000,))
y[X[:,0] > 0.66] = 2
y[(X[:,0] > 0.33) & (X[:,0] <= 0.66)] = 1

model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(1,)),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(optimizer='adam',
              loss='sparse_categorical_crossentropy',
              metrics=['accuracy'])

model.fit(X,y,epochs=20)
model.save("model.h5")
```

---

## 🔄 10. Convert to TensorFlow Lite

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

open("model.tflite","wb").write(tflite_model)
```

---

## 🧩 11. Convert to C Array

```bash
xxd -i model.tflite > model_data.h
```

---

## ⚙️ 12. TFLM Program Flow

1. Load model  
2. Create tensor arena  
3. Create interpreter  
4. Allocate tensors  
5. Read sensor  
6. Run inference  
7. Output prediction  

---

## ⚡ 13. ESP32 Inference Example

```cpp
float input_value = analogRead(36) / 4095.0;

input->data.f[0] = input_value;
interpreter->Invoke();

float dark = output->data.f[0];
float normal = output->data.f[1];
float bright = output->data.f[2];
```

---

## 📈 14. Expected Output

```
ADC: 345
Prediction: DARK
Latency: 450 us
```

---

## 🧩 15. CPS Integration

| CPS Layer | Role |
|----------|-----|
| Connection | LDR sensor |
| Conversion | Normalization |
| Cyber | Neural network |
| Cognition | Classification |
| Configuration | LED / dashboard |

---

## 🧪 16. Experiment Tasks

1. Build LDR circuit  
2. Collect dataset  
3. Train model  
4. Convert to TFLite  
5. Deploy on ESP32  
6. Test predictions  

---

## ❓ 17. Evaluation Questions

- What is TinyML?  
- Why is quantization important?  
- What is Tensor Arena?  
- Difference between TFLM and TFLite?  
- Role of Cognition in CPS?  

---

## 🚀 18. Advanced Extensions

- MQTT integration  
- Node-RED digital twin  
- Multi-sensor fusion  
- INT8 quantization  
- Smart lighting system  

---

## ✅ 19. Conclusion

This lab demonstrates how **TinyML enables AI on embedded systems**.

By combining:
- ESP32  
- Sensors  
- TFLM  

we build **real-time intelligent Cyber-Physical Systems (CPS)** capable of sensing, prediction, and control.

---

## 🧠 Summary

**Physical AI Pipeline:**

```
Sensor → Edge AI → Decision → Action → CPS Loop
```
