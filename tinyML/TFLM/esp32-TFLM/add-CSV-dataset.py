import pandas as pd

def make_dataset_from_csv(csv_path="ldr_dataset.csv"):
    df = pd.read_csv(csv_path)

    label_map = {"dark": 0, "normal": 1, "bright": 2}
    X = df["normalized"].astype(np.float32).values.reshape(-1, 1)
    y = df["label"].map(label_map).astype(np.int32).values

    idx = np.random.permutation(len(X))
    return X[idx], y[idx]
