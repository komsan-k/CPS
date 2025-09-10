# Chapter 3: Sensing and Data Acquisition in CPS

## Chapter Overview
Cyber-Physical Systems (CPS) depend on **sensors** to capture the state of the physical world.  
This chapter explores:

- Sensing principles  
- Signal conditioning  
- Data acquisition (DAQ)  
- Sampling and quantization  
- Noise and calibration  
- Distributed sensing systems  

Hands-on lab: integrate a temperature (LM73) or motion sensor (MPU6050) with an ESP32 and visualize data in **Node-RED**.

---

## Learning Outcomes
After this chapter, students will be able to:

1. Classify sensor types relevant to CPS.  
2. Explain the sensing chain: transduction → conditioning → digitization → communication.  
3. Apply sampling theory and identify aliasing/noise issues.  
4. Understand calibration and error sources.  
5. Build a CPS pipeline (ESP32 + sensor + Node-RED dashboard).  

---

## Key Terms
`Transducer, signal conditioning, ADC, sampling rate, Nyquist theorem, quantization, noise, calibration, MQTT, Node-RED`

---

## Role of Sensing in CPS
Sensors = “eyes and ears” of CPS.  

- **Environmental:** temperature, humidity, light, gas  
- **Mechanical:** acceleration, strain, pressure, vibration  
- **Biological:** heart rate, EEG, EMG  

Accurate sensing is critical — errors propagate directly into actuation.

---

## The Sensing Chain
Stages:

1. **Transduction:** physical → electrical signal (e.g., thermistor).  
2. **Conditioning:** amplification, filtering, linearization.  
3. **Digitization:** ADC converts to discrete values.  
4. **Communication:** values sent via I²C, SPI, UART, MQTT.  

---

## Sampling and Quantization

### Nyquist Theorem
```math
f_s ≥ 2 f_max
```
If violated → aliasing distortion.  

### Quantization
- An N-bit ADC gives `2^N` levels.  
- Error ≈ ± ½ LSB.  

**Example:** 12-bit ADC, 3.3 V reference:  
```math
Δ = 3.3 / 2^12 ≈ 0.8 mV
```

---

## Noise and Calibration
- **Noise:** thermal, EMI, quantization  
- **Filtering:** low-pass for smoothing  
- **Calibration:** align sensor output with reference standard  

---

## Distributed Sensing Systems
- Multiple nodes share data via Wi-Fi, BLE, LoRa  
- Synchronization ensures coherence  
- Data fusion (e.g., Kalman filters) improves estimates  

---

## Case Study: Inertial Sensing (MPU6050)
- Provides `(ax, ay, az)` and `(ωx, ωy, ωz)`  
- Used in drones, robotics, wearables  
- Often combined with magnetometer for 9-DOF  

---

## Node-RED Lab 3: Sensor Integration with ESP32

### Objectives
- Connect ESP32 to LM73 (temp) or MPU6050 (motion).  
- Publish via MQTT.  
- Visualize in Node-RED dashboard (gauge + chart).  

### Hardware/Software
- ESP32 DevKit  
- LM73 (I²C) or MPU6050  
- Node-RED + MQTT broker (e.g., Mosquitto)  

### Procedure
1. Wire sensor (I²C: SDA → GPIO21, SCL → GPIO22).  
2. Program ESP32 to publish JSON to `/cps/sensors/data`.  
3. In Node-RED:  
   - Add **MQTT-in** node subscribed to `/cps/sensors/data`  
   - Parse with **JSON** node  
   - Display with **gauge** + **chart**  

### Expected Output
- Dashboard shows temperature (LM73) or acceleration (MPU6050).  
- Chart updates at 1 Hz.  

### Assessment Rubric (10 points)
- [2] Correct wiring  
- [3] ESP32 publishes MQTT  
- [3] Node-RED gauge + chart  
- [2] Debug panel shows JSON  

---

### Starter Node-RED Flow
```json
[{"id":"tab3","type":"tab","label":"Lab3 - Sensor → MQTT → Dashboard"},
{"id":"broker","type":"mqtt-broker","name":"LocalBroker","broker":"localhost","port":"1883"},
{"id":"mqin","type":"mqtt in","z":"tab3","name":"Sensor data",
"topic":"/cps/sensors/data","qos":"0","datatype":"auto","broker":"broker",
"x":180,"y":120,"wires":[["json3","chart3","gauge3"]]},
{"id":"json3","type":"json","z":"tab3","name":"","property":"payload",
"action":"","pretty":false,"x":360,"y":80,"wires":[["gauge3"]]} ,
{"id":"gauge3","type":"ui_gauge","z":"tab3","group":"uigrp3","order":1,
"gtype":"gage","title":"Sensor Value","label":"units",
"format":"{{payload.value}}","min":"0","max":"100","x":560,"y":80,"wires":[]},
{"id":"chart3","type":"ui_chart","z":"tab3","group":"uigrp3","order":2,
"label":"Sensor Time Series","chartType":"line","legend":"false",
"xformat":"HH:mm:ss","interpolate":"linear","ymin":"0","ymax":"100",
"removeOlder":1,"removeOlderPoints":"300","removeOlderUnit":"3600",
"useOneColor":false,"x":380,"y":160,"wires":[[]]},
{"id":"uigrp3","type":"ui_group","name":"Lab3","tab":"uitab3","order":1,
"disp":true,"width":"8","collapse":false},
{"id":"uitab3","type":"ui_tab","name":"CPS Labs","icon":"dashboard"}]
```

---

### Starter ESP32 Code
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "192.168.1.10";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

void wifiConnect(){
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status()!=WL_CONNECTED) delay(300);
}

void mqttConnect(){
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while(!mqtt.connected()){
    mqtt.connect("esp32-sensor");
    delay(500);
  }
}

void setup(){
  Serial.begin(115200);
  Wire.begin();
  wifiConnect();
  mqttConnect();
}

void loop(){
  if(WiFi.status()!=WL_CONNECTED) wifiConnect();
  if(!mqtt.connected()) mqttConnect();

  int sensorVal = analogRead(34); // replace with LM73/MPU6050
  char msg[64];
  snprintf(msg,sizeof(msg), "{\"value\":%d}", sensorVal);
  mqtt.publish("/cps/sensors/data", msg);

  mqtt.loop();
  delay(1000);
}
```

---

## Summary
- Sensing + data acquisition = first step of CPS loop.  
- Key issues: noise, calibration, sampling, distributed coordination.  
- Lab: integrate ESP32 sensor with Node-RED.  

---

## Review Questions
1. What are the four stages of the sensing chain?  
2. Why is Nyquist sampling important? What happens if violated?  
3. Explain quantization error (example: 10-bit ADC, 5V reference).  
4. What is calibration and why is it critical?  

---

## Exercises
1. Modify Lab 3: publish two sensor values (e.g., temp + accel) in JSON.  
2. Add moving-average filter in Node-RED function node.  
3. Sketch sensor fusion for autonomous vehicle (GPS + accel + gyro).  

---

## Further Reading
- J. Wilson, *Sensor Technology Handbook*, Elsevier, 2005.  
- M. Bishop, *Pattern Recognition and Data Fusion in CPS*, IEEE SMC, 2018.  
- H. Kopetz, *Real-Time Systems: Design Principles*, Springer, 2011.  

