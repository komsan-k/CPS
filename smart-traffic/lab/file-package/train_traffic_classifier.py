#!/usr/bin/env python3
# Train a 3-class congestion predictor (LOW/MEDIUM/HIGH) for 10s windows
# Model: MLP 6 → 8 → 3 (int8 quantized for TFLite Micro)
#
# Expected CSV columns:
#   ns0,ew0,ns1,ew1,ns2,ew2,y
#
# where y is the next-window class (0/1/2).
#
# Normalization must match ESP32 firmware:
#   x_norm = clamp(count, 0, MAX_COUNT_PER_WINDOW) / MAX_COUNT_PER_WINDOW

import argparse
import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow import keras

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", required=True, help="traffic_dataset.csv")
    ap.add_argument("--out", default="traffic_cls_int8.tflite", help="output tflite path")
    ap.add_argument("--max_count", type=float, default=20.0, help="MAX_COUNT_PER_WINDOW used in ESP32")
    ap.add_argument("--epochs", type=int, default=40)
    ap.add_argument("--batch", type=int, default=32)
    args = ap.parse_args()

    df = pd.read_csv(args.csv)

    cols = ["ns0","ew0","ns1","ew1","ns2","ew2"]
    X = df[cols].values.astype(np.float32)
    y = df["y"].values.astype(np.int32)

    # Normalize (must match device)
    X = np.clip(X, 0, args.max_count) / args.max_count

    model = keras.Sequential([
        keras.layers.Input(shape=(6,)),
        keras.layers.Dense(8, activation="relu"),
        keras.layers.Dense(3, activation="softmax")
    ])

    model.compile(optimizer="adam",
                  loss="sparse_categorical_crossentropy",
                  metrics=["accuracy"])

    model.fit(X, y, epochs=args.epochs, batch_size=args.batch, validation_split=0.2, verbose=2)

    # Export int8 quantized TFLite
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    def rep():
        n = min(300, len(X))
        for i in range(n):
            yield [X[i:i+1]]

    converter.representative_dataset = rep
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    tflite_model = converter.convert()
    with open(args.out, "wb") as f:
        f.write(tflite_model)

    print(f"Saved: {args.out}")
    print("Next:")
    print(f"  xxd -i {args.out} > traffic_model_data.cpp")

if __name__ == "__main__":
    main()
