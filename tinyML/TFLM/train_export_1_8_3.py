import numpy as np
import tensorflow as tf

# ---------- 1) Create synthetic dataset (you can replace with real LDR samples) ----------
# Input x is normalized in [0, 1], where 0=dark (low ADC), 1=bright (high ADC)
num_samples = 3000

# Make 3 ranges for 3 classes
x_dark   = np.random.uniform(0.00, 0.33, num_samples // 3)
x_normal = np.random.uniform(0.33, 0.66, num_samples // 3)
x_bright = np.random.uniform(0.66, 1.00, num_samples // 3)

X = np.concatenate([x_dark, x_normal, x_bright]).astype(np.float32).reshape(-1, 1)
y = np.concatenate([
    np.zeros(num_samples // 3, dtype=np.int32),      # dark -> 0
    np.ones(num_samples // 3, dtype=np.int32),       # normal -> 1
    np.full(num_samples // 3, 2, dtype=np.int32)     # bright -> 2
])

# Shuffle
idx = np.random.permutation(len(X))
X, y = X[idx], y[idx]

# ---------- 2) Build 1-8-3 model ----------
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(1,), name="x"),
    tf.keras.layers.Dense(8, activation="relu", name="dense_8"),
    tf.keras.layers.Dense(3, activation="softmax", name="dense_3")
])

model.compile(optimizer=tf.keras.optimizers.Adam(learning_rate=0.01),
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])

model.fit(X, y, epochs=30, batch_size=32, validation_split=0.2, verbose=1)

# ---------- 3) Convert to TFLite (float32) ----------
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = []   # keep float for simplest ESP32 demo
tflite_model = converter.convert()

with open("model.tflite", "wb") as f:
    f.write(tflite_model)

print("Saved: model.tflite")
