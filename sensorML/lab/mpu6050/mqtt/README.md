# MPU6050 IMU Lab: MQTT + Node-RED Dashboard

This package adds **MQTT publishing** and a **Node-RED Dashboard** for visualizing MPU6050 IMU data.

## 1) Arduino (.ino)

File: `ESP32_SensorML_MPU6050_MQTT_NodeRED.ino`

### Configure before upload
Edit:
- `WIFI_SSID`, `WIFI_PASS`
- `MQTT_HOST` (broker IP/hostname)

### Topics
- **Metadata (retained, once at boot):** `device/imu01/metadata`
- **Telemetry (stream):** `device/imu01/telemetry`

## 2) Node-RED Dashboard Flow

File: `NodeRED_Flow_IMU_MPU6050_Dashboard.json`

### Import
Node-RED → Menu → *Import* → select the JSON file.

### Configure MQTT Broker
In the imported flow, open the **MQTT Broker** config node and change:
- Broker host/IP
- Port (default 1883)

### Dashboard
Open:
- `http://<node-red-host>:1880/ui`

You will see:
- **Metadata summary**
- **Accel magnitude gauge** and **Accel XYZ chart**
- **Gyro magnitude gauge** and **Gyro XYZ chart**
- **Temperature gauge** and trend chart

## 3) Notes
- Gyro values are published in **deg/s** (converted from rad/s).
- The flow plots **ax, ay, az** and **gx, gy, gz** as separate series in each chart.
