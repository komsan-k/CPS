import numpy as np
import pandas as pd

np.random.seed(42)

# Bright: 0–200
bright = np.random.randint(0, 201, 300)

# Normal: 201–600
normal = np.random.randint(201, 601, 350)

# Dark: 601–1000
dark = np.random.randint(601, 1001, 350)

# Combine
adc = np.concatenate([bright, normal, dark])

# Shuffle
np.random.shuffle(adc)

# Save CSV
df = pd.DataFrame({"adc": adc})
df.to_csv("ldr_data.csv", index=False)

print(df.head())
print("Saved ldr_data.csv")
