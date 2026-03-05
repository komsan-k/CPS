import numpy as np
import tensorflow as tf
from tensorflow import keras

# X: shape (N, 6)  -> [c_ns, c_ew, q_ns, q_ew, green_ns, green_ew] normalized
# y: shape (N,)    -> 0/1/2

X = np.load("X.npy").astype(np.float32)
y = np.load("y.npy").astype(np.int32)

model = keras.Sequential([
    keras.layers.Input(shape=(6,)),
    keras.layers.Dense(8, activation="relu"),
    keras.layers.Dense(3, activation="softmax")
])

model.compile(optimizer="adam",
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])

model.fit(X, y, epochs=40, batch_size=32, validation_split=0.2)

# Export TFLite (quantized to int8) for TinyML
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

def representative_dataset():
    for i in range(200):
        yield [X[i:i+1]]

converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()
open("traffic_tinyml_int8.tflite", "wb").write(tflite_model)
print("Saved: traffic_tinyml_int8.tflite")
