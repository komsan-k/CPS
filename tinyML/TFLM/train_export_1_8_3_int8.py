import numpy as np
import tensorflow as tf

# ------------------ 1) Dataset (replace with your real LDR samples if you have them) ------------------
num_samples = 3000

x_dark   = np.random.uniform(0.00, 0.33, num_samples // 3)
x_normal = np.random.uniform(0.33, 0.66, num_samples // 3)
x_bright = np.random.uniform(0.66, 1.00, num_samples // 3)

X = np.concatenate([x_dark, x_normal, x_bright]).astype(np.float32).reshape(-1, 1)
y = np.concatenate([
    np.zeros(num_samples // 3, dtype=np.int32),
    np.ones(num_samples // 3, dtype=np.int32),
    np.full(num_samples // 3, 2, dtype=np.int32),
])

idx = np.random.permutation(len(X))
X, y = X[idx], y[idx]

# ------------------ 2) Model 1-8-3 ------------------
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(1,), name="x"),
    tf.keras.layers.Dense(8, activation="relu", name="dense_8"),
    tf.keras.layers.Dense(3, activation="softmax", name="dense_3")
])

model.compile(optimizer=tf.keras.optimizers.Adam(0.01),
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])
model.fit(X, y, epochs=30, batch_size=32, validation_split=0.2, verbose=1)

# ------------------ 3) Full Integer Quantization ------------------
def representative_dataset():
    # Use real samples if possible. Here we use training samples as a placeholder.
    for i in range(300):
        yield [X[i:i+1]]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset

# Force full-int8 model (input/output int8 too)
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_int8 = converter.convert()

with open("model_int8.tflite", "wb") as f:
    f.write(tflite_int8)

print("Saved: model_int8.tflite")
