import os
import numpy as np
import tensorflow as tf

# =========================================================
# Configuration
# =========================================================
MODEL_NAME = "traffic_model_float32_ped"
OUTPUT_DIR = "build_tflm_float32_ped"
EPOCHS = 40
BATCH_SIZE = 32
NUM_SAMPLES = 6000
SEED = 42

np.random.seed(SEED)
tf.random.set_seed(SEED)

# =========================================================
# Helper
# =========================================================
def clamp01(x):
    return np.clip(x, 0.0, 1.0)

# =========================================================
# Synthetic dataset generation with pedestrian request
# =========================================================
def generate_sample():
    # -----------------------------------------------------
    # Sensor inputs
    # Major road (EW) is usually busier
    # -----------------------------------------------------
    ew1 = np.random.choice([0.0, 1.0], p=[0.35, 0.65])
    ew2 = np.random.choice([0.0, 1.0], p=[0.40, 0.60])

    ns1 = np.random.choice([0.0, 1.0], p=[0.60, 0.40])
    ns2 = np.random.choice([0.0, 1.0], p=[0.65, 0.35])

    # Weighted queue scores
    ew_queue = clamp01(0.4 * ew1 + 0.6 * ew2)
    ns_queue = clamp01(0.4 * ns1 + 0.6 * ns2)

    # Current phase: 0 = EW green, 1 = NS green
    phase = np.random.choice([0.0, 1.0])

    # Normalized green time in range 0..1
    green_time = np.random.uniform(0.0, 1.0)

    # Pedestrian request: 0 = none, 1 = active
    ped_request = np.random.choice([0.0, 1.0], p=[0.75, 0.25])

    # -----------------------------------------------------
    # Rule-based labeling
    #
    # Output classes:
    # 0 = keep EW green
    # 1 = switch to NS
    # 2 = keep NS green
    # 3 = switch to EW
    #
    # Assumption:
    # - A pedestrian request encourages switching after
    #   minimum green is satisfied.
    # - We still keep the 4-class structure.
    # -----------------------------------------------------
    min_green_threshold = 0.20

    if phase == 0.0:
        # Currently EW is green
        if green_time < min_green_threshold:
            label = 0  # keep EW during minimum green

        elif ped_request == 1.0:
            # If pedestrian requests service, encourage switch
            # unless EW is very heavily loaded
            if ew_queue > 0.95 and ns_queue < 0.40:
                label = 0
            else:
                label = 1

        elif ns_queue > ew_queue + 0.20:
            label = 1

        else:
            label = 0

    else:
        # Currently NS is green
        if green_time < min_green_threshold:
            label = 2  # keep NS during minimum green

        elif ped_request == 1.0:
            # Pedestrian request encourages switching after min green
            if ns_queue > 0.95 and ew_queue < 0.40:
                label = 2
            else:
                label = 3

        elif ew_queue > ns_queue + 0.20:
            label = 3

        else:
            label = 2

    x = np.array([
        ew1,
        ew2,
        ns1,
        ns2,
        ew_queue,
        ns_queue,
        phase,
        green_time,
        ped_request
    ], dtype=np.float32)

    return x, label

def make_dataset(n=6000):
    X = []
    y = []

    for _ in range(n):
        x, label = generate_sample()
        X.append(x)
        y.append(label)

    return np.array(X, dtype=np.float32), np.array(y, dtype=np.int32)

# =========================================================
# Build model
# =========================================================
def build_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(9,)),
        tf.keras.layers.Dense(8, activation="relu"),
        tf.keras.layers.Dense(8, activation="relu"),
        tf.keras.layers.Dense(4, activation="softmax")
    ])

    model.compile(
        optimizer="adam",
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"]
    )
    return model

# =========================================================
# Convert to FLOAT32 TFLite
# =========================================================
def convert_to_tflite_float32(model):
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    return tflite_model

# =========================================================
# Save as C header
# =========================================================
def save_c_array(model_bytes, filename):
    with open(filename, "w", encoding="utf-8") as f:
        f.write("#pragma once\n\n")
        f.write("#include <cstdint>\n\n")
        f.write("alignas(16) const unsigned char g_model[] = {\n")

        for i, byte in enumerate(model_bytes):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x}")
            if i != len(model_bytes) - 1:
                f.write(", ")
            if i % 12 == 11:
                f.write("\n")

        if len(model_bytes) % 12 != 0:
            f.write("\n")

        f.write("};\n\n")
        f.write(f"const unsigned int g_model_len = {len(model_bytes)};\n")

# =========================================================
# Main
# =========================================================
def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Create dataset
    X, y = make_dataset(NUM_SAMPLES)

    # Shuffle
    indices = np.arange(len(X))
    np.random.shuffle(indices)
    X = X[indices]
    y = y[indices]

    # Split
    split = int(0.8 * len(X))
    X_train = X[:split]
    y_train = y[:split]

    X_test = X[split:]
    y_test = y[split:]

    # Build and train
    model = build_model()
    model.summary()

    model.fit(
        X_train,
        y_train,
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        validation_split=0.2,
        verbose=1
    )

    # Evaluate
    loss, acc = model.evaluate(X_test, y_test, verbose=0)
    print(f"Test Loss     = {loss:.4f}")
    print(f"Test Accuracy = {acc:.4f}")

    # Save Keras model
    keras_path = os.path.join(OUTPUT_DIR, MODEL_NAME + ".keras")
    model.save(keras_path)

    # Convert and save float32 TFLite
    tflite_model = convert_to_tflite_float32(model)
    tflite_path = os.path.join(OUTPUT_DIR, MODEL_NAME + ".tflite")
    with open(tflite_path, "wb") as f:
        f.write(tflite_model)

    # Save C header
    header_path = os.path.join(OUTPUT_DIR, "traffic_model_float32_ped.h")
    save_c_array(tflite_model, header_path)

    print("\nSaved files:")
    print(keras_path)
    print(tflite_path)
    print(header_path)

    # Show a few predictions
    preds = model.predict(X_test[:10], verbose=0)
    pred_labels = np.argmax(preds, axis=1)

    print("\nSample predictions:")
    print("True labels :", y_test[:10])
    print("Pred labels :", pred_labels)

if __name__ == "__main__":
    main()
