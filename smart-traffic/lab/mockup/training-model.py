import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow import keras

df = pd.read_csv("traffic_dataset.csv")

X = df[["ns0","ew0","ns1","ew1","ns2","ew2"]].values.astype(np.float32)
y = df["y"].values.astype(np.int32)

# --- simple normalization (min-max) ---
# เก็บค่าสเกลไว้ใช้บน ESP32 ด้วย
x_min = X.min(axis=0)
x_max = X.max(axis=0)
Xn = (X - x_min) / (x_max - x_min + 1e-6)

np.save("x_min.npy", x_min)
np.save("x_max.npy", x_max)

model = keras.Sequential([
    keras.layers.Input(shape=(6,)),
    keras.layers.Dense(8, activation="relu"),
    keras.layers.Dense(3, activation="softmax")
])

model.compile(optimizer="adam",
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])

model.fit(Xn, y, epochs=40, batch_size=32, validation_split=0.2)

# --- int8 quantization for TFLM ---
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

def rep():
    for i in range(min(200, len(Xn))):
        yield [Xn[i:i+1]]

converter.representative_dataset = rep
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflm = converter.convert()
open("traffic_cls_int8.tflite", "wb").write(tflm)
print("Saved: traffic_cls_int8.tflite")
