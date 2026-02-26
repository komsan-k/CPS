import sys

infile = "model.tflite"
outfile = "model_data.h"
if len(sys.argv) >= 2: infile = sys.argv[1]
if len(sys.argv) >= 3: outfile = sys.argv[2]

data = open(infile, "rb").read()

with open(outfile, "w") as f:
    f.write("#pragma once\n\n")
    f.write("#include <cstdint>\n\n")
    f.write("alignas(16) const unsigned char g_model[] = {\n")
    for i, b in enumerate(data):
        if i % 12 == 0:
            f.write("  ")
        f.write(f"0x{b:02x}, ")
        if i % 12 == 11:
            f.write("\n")
    f.write("\n};\n")
    f.write(f"const unsigned int g_model_len = {len(data)};\n")

print("Saved:", outfile, "bytes:", len(data))
