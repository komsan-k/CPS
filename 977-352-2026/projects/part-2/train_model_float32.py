import os
import numpy as np
import tensorflow as tf

# =========================================================
# Configuration
# =========================================================
MODEL_NAME = "traffic_model_float32"
OUTPUT_DIR = "build_tflm_float32"
EPOCHS = 40
BATCH_SIZE = 32
NUM_SAMPLES = 5000

np.random.seed(42)
tf.random.set_seed(42)

# =========================================================
# Synthetic dataset generation
# =========================================================
def generate_sample():
    # 4 sensor inputs
    ew1 = np.random.choice([0.0, 1.0], p=[0.35, 0.65])
    ew2 = np.random.choice([0.0, 1.0], p=[0.40, 0.60])

    ns1 = np.random.choice([0.0, 1.0], p=[0.60, 0.40])
    ns2 = np.random.choice([0.0, 1.0], p=[0.65, 0.35])

    # queue scores
    ew_queue = 0.4 * ew1 + 0.6 * ew2
    ns_queue = 0.4 * ns1 + 0.6 * ns2

    # phase: 0 = EW, 1 = NS
    phase = np.random.choice([0.0, 1.0])

    # normalized green time
    green_time = np.random.uniform(0.0, 1.0)

    # =====================================================
    # Rule-based label generation
    # =====================================================
    if phase == 0.0:
        # EW currently green
        if green_time < 0.2:
            label = 0
        elif ns_queue > ew_queue + 0.2:
            label = 1
        else:
            label = 0
    else:
        # NS currently green
        if green_time < 0.2:
            label = 2
        elif ew_queue > ns_queue + 0.2:
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
        green_time
    ], dtype=np.float32)

    return x, label


def make_dataset(n=5000):
    X = []
    y = []

    for _ in range(n):
        x, label = generate_sample()
        X.append(x)
        y.append(label)

    return np.array(X, dtype=np.float32), np.array(y)


# =========================================================
# Build model
# =========================================================
def build_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(8,)),
        tf.keras.layers.Dense(8, activation='relu'),
        tf.keras.layers.Dense(8, activation='relu'),
        tf.keras.layers.Dense(4, activation='softmax')
    ])

    model.compile(
        optimizer='adam',
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )

    return model


# =========================================================
# Convert to float32 TFLite
# =========================================================
def convert_to_tflite_float32(model):
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    return tflite_model


# =========================================================
# Save as C header
# =========================================================
def save_c_array(model_bytes, filename):
    with open(filename, "w") as f:
        f.write("#pragma once\n\n")
        f.write("alignas(16) const unsigned char g_model[] = {\n")

        for i, byte in enumerate(model_bytes):
            if i % 12 == 0:
                f.write("    ")

            f.write(f"0x{byte:02x}, ")

            if i % 12 == 11:
                f.write("\n")

        f.write("\n};\n\n")
        f.write(f"const unsigned int g_model_len = {len(model_bytes)};\n")


# =========================================================
# Main
# =========================================================
def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    X, y = make_dataset(NUM_SAMPLES)

    split = int(0.8 * len(X))

    X_train = X[:split]
    y_train = y[:split]

    X_test = X[split:]
    y_test = y[split:]

    model = build_model()
    model.summary()

    model.fit(
        X_train,
        y_train,
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        validation_split=0.2
    )

    loss, acc = model.evaluate(X_test, y_test)
    print(f"Test Accuracy = {acc:.4f}")

    # save keras
    keras_path = os.path.join(OUTPUT_DIR, MODEL_NAME + ".keras")
    model.save(keras_path)

    # save float32 tflite
    tflite_model = convert_to_tflite_float32(model)

    tflite_path = os.path.join(OUTPUT_DIR, MODEL_NAME + ".tflite")
    with open(tflite_path, "wb") as f:
        f.write(tflite_model)

    # save C header
    header_path = os.path.join(OUTPUT_DIR, "traffic_model_float32.h")
    save_c_array(tflite_model, header_path)

    print("Saved files:")
    print(keras_path)
    print(tflite_path)
    print(header_path)


if __name__ == "__main__":
    main()
