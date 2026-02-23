import tensorflow as tf
import numpy as np

# 1. ข้อมูลสำหรับเทรน (0.0-1.0)
X = np.array([0.05, 0.1, 0.15, 0.4, 0.5, 0.6, 0.85, 0.9, 0.95], dtype=np.float32).reshape(-1, 1)
y = np.array([0, 0, 0, 1, 1, 1, 2, 2, 2], dtype=np.uint8)

# 2. สร้างโครงสร้างประสาทเทียมจิ๋ว (1-8-3)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(1,)),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.fit(X, y, epochs=500, verbose=0)

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
