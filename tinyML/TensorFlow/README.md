# 🧠 TensorFlow

TensorFlow is an open-source **Machine Learning (ML)** and **Deep
Learning (DL)** framework developed by Google Brain and released by
Google in 2015.

It enables researchers, engineers, and developers to build, train, and
deploy AI models from cloud systems down to embedded edge devices.

------------------------------------------------------------------------

## 1️⃣ Why is it called TensorFlow?

-   **Tensor** → Multi-dimensional array (vector, matrix, 3D array,
    etc.)
-   **Flow** → Data flows through computational graphs

Example tensor shapes:

-   Scalar → `()`
-   Vector → `(n,)`
-   Matrix → `(m, n)`
-   Image → `(height, width, channels)`

------------------------------------------------------------------------

## 2️⃣ Core Components

### 🔹 Tensors

Basic data structure (similar to NumPy arrays).

``` python
import tensorflow as tf

x = tf.constant([[1, 2], [3, 4]])
print(x)
```

------------------------------------------------------------------------

### 🔹 Operations

Mathematical functions applied to tensors.

``` python
y = tf.matmul(x, x)
```

------------------------------------------------------------------------

### 🔹 Computational Graph

Conceptual flow:

Input → Layer → Activation → Output

In TensorFlow 2.x, execution runs in **eager mode** by default.

------------------------------------------------------------------------

### 🔹 Keras API

TensorFlow integrates Keras as its official high-level API.

``` python
model = tf.keras.Sequential([
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='categorical_crossentropy',
    metrics=['accuracy']
)
```

------------------------------------------------------------------------

## 3️⃣ TensorFlow Ecosystem

  Component         Purpose
  ----------------- -----------------------------------
  TensorFlow Core   Model building & training
  TensorFlow Lite   Edge & microcontroller deployment
  TensorFlow.js     Run ML in browsers
  TensorBoard       Visualization & debugging
  TF Serving        Model deployment

------------------------------------------------------------------------

## 4️⃣ Typical Workflow

### Step 1 --- Collect Data

Sensors (LDR, LM73, MPU6050, etc.)

### Step 2 --- Preprocess

Normalization, filtering, feature extraction

### Step 3 --- Build Model

``` python
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])
```

### Step 4 --- Train

``` python
model.fit(X_train, y_train, epochs=50)
```

### Step 5 --- Convert to TensorFlow Lite

``` python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
```

### Step 6 --- Deploy to Edge

-   ESP32
-   Raspberry Pi
-   Android
-   IoT devices

------------------------------------------------------------------------

## 5️⃣ TensorFlow in Cyber-Physical Systems (CPS)

  CPS Layer             TensorFlow Role
  --------------------- --------------------
  Smart Connection      Data collection
  Data-to-Information   Feature extraction
  Cyber                 Model training
  Cognition             Prediction
  Configuration         Adaptive control

------------------------------------------------------------------------

## 6️⃣ Advantages

✔ Scalable (Edge → Cloud)\
✔ GPU/TPU acceleration\
✔ Strong ecosystem\
✔ Research + production ready

------------------------------------------------------------------------

## 7️⃣ Framework Comparison

  Framework      Strength
  -------------- -------------------------
  TensorFlow     Production + deployment
  PyTorch        Research flexibility
  Scikit-learn   Classical ML

------------------------------------------------------------------------

# 🔎 Summary

TensorFlow is a complete AI framework that allows you to:

**Build → Train → Optimize → Deploy**

From cloud infrastructure down to microcontrollers (TinyML).

------------------------------------------------------------------------

📘 Suitable for: - Smart Manufacturing - Smart Cities - Predictive
Maintenance - Edge AI - Cyber-Physical Systems
