"""
train_model.py
------------------------------------------------------------
Train a tiny neural network for a smart traffic intersection:
- EW major road
- NS minor road
- 2 sensors per road
- 4 output classes

Inputs:
[ew1, ew2, ns1, ns2, ew_queue, ns_queue, phase, green_time]

Output classes:
0 = keep EW green
1 = switch to NS
2 = keep NS green
3 = switch to EW

This script:
1) generates a synthetic dataset
2) trains a small Keras model
3) converts it to INT8 TFLite
4) writes model.tflite
5) optionally writes model_data.h as a C array
"""

import os
import numpy as np
import tensorflow as tf

MODEL_NAME = "traffic_tflm"
OUT_DIR = "build_traffic_tflm"
EPOCHS = 40
BATCH_SIZE = 32
NUM_SAMPLES = 5000
SEED = 42

np.random.seed(SEED)
tf.random.set_seed(SEED)


def clamp01(x):
    return np.clip(x, 0.0, 1.0)


def generate_sample():
    # Binary or soft occupancy values for 4 sensors
    ew1 = np.random.choice([0.0, 1.0], p=[0.35, 0.65])  # major road usually busier
    ew2 = np.random.choice([0.0, 1.0], p=[0.40, 0.60])
    ns1 = np.random.choice([0.0, 1.0], p=[0.60, 0.40])
    ns2 = np.random.choice([0.0, 1.0], p=[0.65, 0.35])

    ew_queue = clamp01(0.4 * ew1 + 0.6 * ew2)
    ns_queue = clamp01(0.4 * ns1 + 0.6 * ns2)

    # phase: 0 = EW green, 1 = NS green
    phase = np.random.choice([0.0, 1.0])

    # green time normalized 0..1
    green_time = np.random.uniform(0.0, 1.0)

    # Rule-based label generation for initial dataset
    if phase == 0.0:  # currently EW green
        if green_time < 0.18:
            label = 0  # minimum green area -> keep EW
        elif ns_queue > 0.75 and ew_queue < 0.8:
            label = 1  # switch to NS
        elif ns_queue > ew_queue + 0.25 and green_time > 0.30:
            label = 1
        else:
            label = 0
    else:  # currently NS green
        if green_time < 0.18:
            label = 2  # keep NS
        elif ew_queue > 0.75 and ns_queue < 0.8:
            label = 3  # switch to EW
        elif ew_queue > ns_queue + 0.25 and green_time > 0.30:
            label = 3
        else:
            label = 2

    features = np.array([
        ew1, ew2, ns1, ns2, ew_queue, ns_queue, phase, green_time
    ], dtype=np.float32)

    return features, label


def make_dataset(num_samples=5000):
    X = []
    y = []
    for _ in range(num_samples):
        xi, yi = generate_sample()
        X.append(xi)
        y.append(yi)
    return np.array(X, dtype=np.float32), np.array(y, dtype=np.int32)


def build_model(input_dim=8, num_classes=4):
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(input_dim,)),
        tf.keras.layers.Dense(8, activation="relu"),
        tf.keras.layers.Dense(8, activation="relu"),
        tf.keras.layers.Dense(num_classes, activation="softmax")
    ])
    model.compile(
        optimizer="adam",
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"]
    )
    return model


def convert_to_tflite_int8(model, X_calib):
    def representative_dataset():
        for i in range(min(200, len(X_calib))):
            yield [X_calib[i:i+1].astype(np.float32)]

    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    return converter.convert()


def save_c_array(tflite_model, header_path, var_name="g_model"):
    with open(header_path, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        f.write("#include <cstdint>\n\n")
        f.write(f"alignas(16) const unsigned char {var_name}[] = {{\n")

        for i, b in enumerate(tflite_model):
            if i % 12 == 0:
                f.write("  ")
            f.write(f"0x{b:02x}")
            if i != len(tflite_model) - 1:
                f.write(", ")
            if i % 12 == 11:
                f.write("\n")

        if len(tflite_model) % 12 != 0:
            f.write("\n")

        f.write("};\n\n")
        f.write(f"const unsigned int {var_name}_len = {len(tflite_model)};\n")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    X, y = make_dataset(NUM_SAMPLES)

    # Shuffle and split
    idx = np.arange(len(X))
    np.random.shuffle(idx)
    X = X[idx]
    y = y[idx]

    n_train = int(0.8 * len(X))
    X_train, X_test = X[:n_train], X[n_train:]
    y_train, y_test = y[:n_train], y[n_train:]

    model = build_model()
    model.summary()

    model.fit(
        X_train, y_train,
        validation_split=0.2,
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        verbose=1
    )

    loss, acc = model.evaluate(X_test, y_test, verbose=0)
    print(f"Test loss: {loss:.4f}")
    print(f"Test acc : {acc:.4f}")

    # Save Keras model
    keras_path = os.path.join(OUT_DIR, MODEL_NAME + ".keras")
    model.save(keras_path)
    print("Saved:", keras_path)

    # Convert to INT8 TFLite
    tflite_model = convert_to_tflite_int8(model, X_train)
    tflite_path = os.path.join(OUT_DIR, MODEL_NAME + "_int8.tflite")
    with open(tflite_path, "wb") as f:
        f.write(tflite_model)
    print("Saved:", tflite_path)

    # Save C header for Arduino
    header_path = os.path.join(OUT_DIR, "model_data.h")
    save_c_array(tflite_model, header_path)
    print("Saved:", header_path)

    # Show sample predictions
    preds = model.predict(X_test[:10], verbose=0)
    pred_labels = np.argmax(preds, axis=1)
    print("True labels :", y_test[:10])
    print("Pred labels :", pred_labels)


if __name__ == "__main__":
    main()
