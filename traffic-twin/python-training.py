"""
Traffic Twin — Python training script (flow -> class)
- Trains a tiny MLP (1-8-3) to classify congestion level from traffic flow (veh/min)
- Exports: Keras .h5 + TensorFlow Lite .tflite (+ optional C header for TFLite Micro)

Classes (default):
  0 = LOW     (flow < 20)
  1 = MEDIUM  (20 <= flow < 35)
  2 = HIGH    (flow >= 35)

You can run this even without real data (it generates synthetic training samples).
If you have real CSV, set USE_REAL_CSV=True and point to your file.

Requirements:
  pip install tensorflow numpy pandas
"""

import os
import numpy as np

# Optional: only needed if you use real CSV
try:
    import pandas as pd
except Exception:
    pd = None

import tensorflow as tf

# -----------------------------
# User settings
# -----------------------------
SEED = 7
np.random.seed(SEED)
tf.random.set_seed(SEED)

USE_REAL_CSV = False
CSV_PATH = "traffic_flow.csv"  # expects column: flow_vpm (vehicles per minute)

# Thresholds for class labels (same as Node-RED demo)
TH_LOW = 20.0
TH_HIGH = 35.0

# Model + training
EPOCHS = 60
BATCH_SIZE = 64
LR = 1e-3

# Export paths
OUT_DIR = "out_traffic_tinyml"
KERAS_PATH = os.path.join(OUT_DIR, "flow_to_class_mlp_1_8_3.h5")
TFLITE_PATH = os.path.join(OUT_DIR, "flow_to_class_mlp_1_8_3_int8.tflite")
HEADER_PATH = os.path.join(OUT_DIR, "flow_to_class_mlp_1_8_3_int8.h")


# -----------------------------
# Helpers
# -----------------------------
def flow_to_class(flow_vpm: np.ndarray) -> np.ndarray:
    """Convert flow (veh/min) to class labels 0/1/2 using thresholds."""
    y = np.zeros_like(flow_vpm, dtype=np.int32)
    y[(flow_vpm >= TH_LOW) & (flow_vpm < TH_HIGH)] = 1
    y[flow_vpm >= TH_HIGH] = 2
    return y


def make_synthetic_dataset(n=6000):
    """
    Synthetic flow distribution:
    - low:   0..20
    - med:   20..35
    - high:  35..60
    Adds a bit of noise and boundary emphasis.
    """
    n0 = n // 3
    n1 = n // 3
    n2 = n - n0 - n1

    # Emphasize decision boundaries a bit (helps robustness)
    low = np.concatenate([
        np.random.uniform(0, 15, int(n0 * 0.7)),
        np.random.uniform(15, 22, int(n0 * 0.3)),
    ])
    med = np.concatenate([
        np.random.uniform(18, 28, int(n1 * 0.6)),
        np.random.uniform(28, 38, int(n1 * 0.4)),
    ])
    high = np.concatenate([
        np.random.uniform(33, 45, int(n2 * 0.5)),
        np.random.uniform(45, 60, int(n2 * 0.5)),
    ])

    X = np.concatenate([low, med, high]).astype(np.float32)

    # Add small measurement noise
    X = X + np.random.normal(loc=0.0, scale=0.8, size=X.shape).astype(np.float32)
    X = np.clip(X, 0.0, 60.0)

    y = flow_to_class(X)
    return X.reshape(-1, 1), y


def load_real_csv(csv_path: str):
    if pd is None:
        raise RuntimeError("pandas not installed. Run: pip install pandas")
    df = pd.read_csv(csv_path)
    if "flow_vpm" not in df.columns:
        raise ValueError("CSV must contain a column named 'flow_vpm'")
    X = df["flow_vpm"].to_numpy(dtype=np.float32)
    X = np.clip(X, 0.0, 60.0)
    y = flow_to_class(X)
    return X.reshape(-1, 1), y


def train_val_test_split(X, y, val=0.15, test=0.15):
    n = len(X)
    idx = np.arange(n)
    np.random.shuffle(idx)
    X, y = X[idx], y[idx]

    n_test = int(n * test)
    n_val = int(n * val)
    X_test, y_test = X[:n_test], y[:n_test]
    X_val, y_val = X[n_test:n_test + n_val], y[n_test:n_test + n_val]
    X_train, y_train = X[n_test + n_val:], y[n_test + n_val:]
    return (X_train, y_train), (X_val, y_val), (X_test, y_test)


def build_model():
    """
    1-8-3 MLP
    Input: 1 feature (normalized flow)
    Hidden: 8 ReLU
    Output: 3 softmax classes
    """
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(1,)),
        tf.keras.layers.Dense(8, activation="relu"),
        tf.keras.layers.Dense(3, activation="softmax"),
    ])
    opt = tf.keras.optimizers.Adam(learning_rate=LR)
    model.compile(optimizer=opt,
                  loss="sparse_categorical_crossentropy",
                  metrics=["accuracy"])
    return model


def representative_dataset_gen(X_train_norm, n_samples=300):
    """Representative dataset for INT8 calibration (TFLite)."""
    # Pick random rows
    idx = np.random.choice(len(X_train_norm), size=min(n_samples, len(X_train_norm)), replace=False)
    for i in idx:
        # Must yield a list/tuple of input arrays
        yield [X_train_norm[i:i+1].astype(np.float32)]


def tflite_to_c_header(tflite_path: str, header_path: str, var_name="g_flow_to_class_model"):
    """Convert .tflite binary into a C header array (for TFLite Micro)."""
    with open(tflite_path, "rb") as f:
        data = f.read()
    arr = np.frombuffer(data, dtype=np.uint8)

    with open(header_path, "w") as f:
        f.write("#pragma once\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"// Model size: {len(arr)} bytes\n")
        f.write(f"const unsigned int {var_name}_len = {len(arr)};\n")
        f.write(f"const unsigned char {var_name}[] = {{\n  ")

        # format 12 bytes per line
        for i, b in enumerate(arr):
            f.write(f"0x{b:02x}, ")
            if (i + 1) % 12 == 0:
                f.write("\n  ")
        f.write("\n};\n")


# -----------------------------
# Main
# -----------------------------
def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # 1) Data
    if USE_REAL_CSV:
        X, y = load_real_csv(CSV_PATH)
    else:
        X, y = make_synthetic_dataset(n=6000)

    (X_train, y_train), (X_val, y_val), (X_test, y_test) = train_val_test_split(X, y)

    # 2) Normalize (important for stable tiny model)
    # Use training-set statistics only
    x_min = float(np.min(X_train))
    x_max = float(np.max(X_train))
    eps = 1e-9

    def norm(z):
        return (z - x_min) / (x_max - x_min + eps)

    X_train_n = norm(X_train).astype(np.float32)
    X_val_n = norm(X_val).astype(np.float32)
    X_test_n = norm(X_test).astype(np.float32)

    print(f"Train size: {len(X_train_n)} | Val: {len(X_val_n)} | Test: {len(X_test_n)}")
    print(f"Flow min/max (train): {x_min:.2f} .. {x_max:.2f}")
    print(f"Thresholds: LOW<{TH_LOW}, MED<{TH_HIGH}, else HIGH")

    # 3) Model
    model = build_model()
    model.summary()

    # 4) Train
    cb = [
        tf.keras.callbacks.EarlyStopping(monitor="val_accuracy", patience=8, restore_best_weights=True)
    ]
    history = model.fit(
        X_train_n, y_train,
        validation_data=(X_val_n, y_val),
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        callbacks=cb,
        verbose=2
    )

    # 5) Evaluate
    test_loss, test_acc = model.evaluate(X_test_n, y_test, verbose=0)
    print(f"\nTest accuracy: {test_acc*100:.2f}%")

    # 6) Save Keras model
    model.save(KERAS_PATH)
    print(f"Saved Keras model: {KERAS_PATH}")

    # 7) Export INT8 TFLite (for TFLite Micro)
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    # Full INT8 quantization (inputs + outputs int8)
    converter.representative_dataset = lambda: representative_dataset_gen(X_train_n, n_samples=300)
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    tflite_model = converter.convert()
    with open(TFLITE_PATH, "wb") as f:
        f.write(tflite_model)
    print(f"Saved TFLite INT8 model: {TFLITE_PATH}")

    # 8) Export a C header (optional, but handy for Arduino/TFLM)
    tflite_to_c_header(TFLITE_PATH, HEADER_PATH, var_name="g_flow_to_class_model")
    print(f"Saved C header: {HEADER_PATH}")

    # 9) Also export normalization constants (needed on ESP32)
    # You must apply the SAME normalization before inference.
    print("\n=== Copy these constants to ESP32 / Node-RED (if you do on-device inference) ===")
    print(f"x_min = {x_min:.6f}")
    print(f"x_max = {x_max:.6f}")
    print("normalize: x_norm = (x - x_min) / (x_max - x_min + 1e-9)")

    # 10) Quick sanity test with some values
    sample = np.array([[5.0], [25.0], [50.0]], dtype=np.float32)
    sample_n = norm(sample)
    pred = model.predict(sample_n, verbose=0)
    print("\nSanity test (flow -> probs -> class):")
    for fval, p in zip(sample.flatten(), pred):
        cls = int(np.argmax(p))
        print(f"  flow={fval:>5.1f} -> probs={np.round(p,3)} -> class={cls}")


if __name__ == "__main__":
    main()
