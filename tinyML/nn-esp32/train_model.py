
# train_model.py
# TinyML Neural Network Training Script for ESP32 + LDR

import tensorflow as tf
import numpy as np
import pandas as pd

# Load Dataset
data = pd.read_csv("ldr_training.csv")

# Normalize ADC values (ESP32 ADC range 0-4095)
X = (data["Value"].values.astype(np.float32) / 4095.0).reshape(-1,1)
y = data["Label"].values.astype(np.uint8)

# Build Model (1-8-3 Neural Network)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(1,)),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

# Train
model.fit(X, y, epochs=500, verbose=1)

# Helper Function to Print Arduino Arrays
def print_cpp_array(name, data):
    if len(data.shape) == 1:
        print(f"float {name}[{data.shape[0]}] = {{", end="")
        print(", ".join([f"{x:.6f}" for x in data]), end="")
        print("};\n")
    else:
        print(f"float {name}[{data.shape[0]}][{data.shape[1]}] = {{")
        for row in data:
            print("  {", ", ".join([f"{x:.6f}" for x in row]), "},")
        print("};\n")

# Extract Weights
w1, b1 = model.layers[0].get_weights()
w2, b2 = model.layers[1].get_weights()

print("\n// ===== COPY THESE ARRAYS INTO ARDUINO CODE =====\n")
print_cpp_array("w1", w1.T)
print_cpp_array("b1", b1)
print_cpp_array("w2", w2.T)
print_cpp_array("b2", b2)

print("Training complete. Copy arrays into Arduino sketch.")
