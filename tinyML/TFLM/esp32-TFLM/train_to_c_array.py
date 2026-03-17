import os
import numpy as np
import tensorflow as tf


# =========================================================
# Configuration
# =========================================================
MODEL_NAME = "ldr_model"
OUTPUT_DIR = "build_tflm"
EPOCHS = 30
BATCH_SIZE = 32
NUM_SAMPLES = 3000
USE_INT8 = True   # True = full integer quantization, False = float32 TFLite


# =========================================================
# Step 1: Create dataset
# Replace this with real CSV loading if needed
# =========================================================
def make_dataset(num_samples=3000):
    n = num_samples // 3

    x_dark = np.random.uniform(0.00, 0.33, n)
    x_normal = np.random.uniform(0.33, 0.66, n)
    x_bright = np.random.uniform(0.66, 1.00, n)

    X = np.concatenate([x_dark, x_normal, x_bright]).astype(np.float32).reshape(-1, 1)
    y = np.concatenate([
        np.zeros(n, dtype=np.int32),
        np.ones(n, dtype=np.int32),
        np.full(n, 2, dtype=np.int32)
    ])

    idx = np.random.permutation(len(X))
    return X[idx], y[idx]


# =========================================================
# Step 2: Build 1-8-3 model
# =========================================================
def build_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(1,), name="x"),
        tf.keras.layers.Dense(8, activation="relu", name="dense_8"),
        tf.keras.layers.Dense(3, activation="softmax", name="dense_3")
    ])

    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.01),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"]
    )
    return model


# =========================================================
# Step 3: Convert TFLite -> C header
# =========================================================
def tflite_to_c_array(tflite_path, header_path, var_name="g_model"):
    with open(tflite_path, "rb") as f:
        data = f.read()

    with open(header_path, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        f.write("#include <cstdint>\n\n")
        f.write(f"alignas(16) const unsigned char {var_name}[] = {{\n")

        for i, b in enumerate(data):
            if i % 12 == 0:
                f.write("  ")
            f.write(f"0x{b:02x}, ")
            if i % 12 == 11:
                f.write("\n")

        if len(data) % 12 != 0:
            f.write("\n")

        f.write("};\n")
        f.write(f"const unsigned int {var_name}_len = {len(data)};\n")

    print(f"[OK] C header saved: {header_path}")
    print(f"[OK] C array bytes: {len(data)}")


# =========================================================
# Step 4: Export TFLite
# =========================================================
def export_tflite(model, X, out_path, use_int8=True):
    converter = tf.lite.TFLiteConverter.from_keras_model(model)

    if use_int8:
        def representative_dataset():
            limit = min(len(X), 500)
            for i in range(limit):
                yield [X[i:i+1]]

        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = representative_dataset
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter.inference_input_type = tf.int8
        converter.inference_output_type = tf.int8
    else:
        converter.optimizations = []

    tflite_model = converter.convert()

    with open(out_path, "wb") as f:
        f.write(tflite_model)

    print(f"[OK] TFLite model saved: {out_path}")


# =========================================================
# Main pipeline
# =========================================================
def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    keras_path = os.path.join(OUTPUT_DIR, f"{MODEL_NAME}.keras")
    tflite_path = os.path.join(
        OUTPUT_DIR,
        f"{MODEL_NAME}_{'int8' if USE_INT8 else 'float32'}.tflite"
    )
    header_path = os.path.join(
        OUTPUT_DIR,
        f"{MODEL_NAME}_{'int8' if USE_INT8 else 'float32'}.h"
    )

    print("[1/5] Creating dataset...")
    X, y = make_dataset(NUM_SAMPLES)

    print("[2/5] Building model...")
    model = build_model()

    print("[3/5] Training model...")
    model.fit(X, y, epochs=EPOCHS, batch_size=BATCH_SIZE, validation_split=0.2, verbose=1)

    print("[4/5] Saving Keras model...")
    model.save(keras_path)
    print(f"[OK] Keras model saved: {keras_path}")

    print("[5/5] Exporting TFLite and C header...")
    export_tflite(model, X, tflite_path, use_int8=USE_INT8)
    tflite_to_c_array(tflite_path, header_path, var_name="g_model")

    print("\nDone.")
    print("Generated files:")
    print(f" - {keras_path}")
    print(f" - {tflite_path}")
    print(f" - {header_path}")


if __name__ == "__main__":
    main()
