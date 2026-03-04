import tensorflow as tf
import numpy as np

# 1. ข้อมูลสำหรับเทรน (0.0-1.0) - More precise and larger dataset
num_samples = 300
np.random.seed(42) # for reproducibility

# Generate X values slightly spread out around the decision boundaries
X = np.concatenate([
    np.random.uniform(0.80, 0.88, num_samples // 3),
    np.random.uniform(0.9, 0.98, num_samples // 3),
    np.random.uniform(0.99, 1.00, num_samples // 3)
]).astype(np.float32).reshape(-1, 1)

# Generate corresponding y labels
y = np.concatenate([
    np.zeros(num_samples // 3, dtype=np.uint8),
    np.ones(num_samples // 3, dtype=np.uint8),
    np.full(num_samples // 3, 2, dtype=np.uint8)
])

# Shuffle the data to mix classes
shuffled_indices = np.random.permutation(num_samples)
X = X[shuffled_indices]
y = y[shuffled_indices]

print(f"Generated X shape: {X.shape}, y shape: {y.shape}")
print(f"First 5 X values: {X[:5].flatten()}")
print(f"First 5 y values: {y[:5]}")

# 2. สร้างโครงสร้างประสาทเทียมจิ๋ว (1-8-3)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(1,)),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.fit(X, y, epochs=100, verbose=0) # Reduced epochs for faster re-training with larger data

# 3. ฟังก์ชัน Export เป็น C++ Array
def print_cpp_array(name, data):
    if len(data.shape) == 1:
        print(f"float {name}[{data.shape[0]}] = {{", end="")
        print(", ".join([f"{x:.6f}" for x in data]), end="")
        print("};\n")
    else:
        print(f"float {name}[{data.shape[0]}][{data.shape[1]}] = {{")
        for row in data:
            print("  {", ", ".join([f"{x:.6f}" for x in row]), "},")
        print("};\n")

w1, b1 = model.layers[0].get_weights()
w2, b2 = model.layers[1].get_weights()

print("// --- COPY TO ARDUINO ---")
print_cpp_array("w1", w1.T)
print_cpp_array("b1", b1)
print_cpp_array("w2", w2.T)
print_cpp_array("b2", b2)
