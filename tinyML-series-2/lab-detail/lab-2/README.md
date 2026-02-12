# 🔹 Lab 2 --- Signal Conditioning & Filtering

## ESP32 + LDR + LM73 + MPU6050

## 🎯 Objective

Convert raw sensor signals into stable signals using:

-   Moving average (LDR)
-   Temperature smoothing (LM73)
-   Offset correction (MPU6050)

Dashboard comparison: - Raw vs Filtered signals

------------------------------------------------------------------------

## 📡 MQTT Topic

ESP32 → Node-RED: tinymlcps/lab2/telemetry

------------------------------------------------------------------------

## 📦 Example JSON

{ "lux_raw": 530, "lux_filtered": 512, "temp_raw": 29.8,
"temp_filtered": 29.6, "ax_raw": 0.05, "ax_corrected": 0.01 }

------------------------------------------------------------------------

## CPS Mapping

Connection → Sensor acquisition\
Conversion → Filtering & correction\
Cyber → Dashboard comparison
