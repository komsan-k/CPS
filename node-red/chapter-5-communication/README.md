# Chapter 5: Communication and Networking in CPS

## Chapter Overview
Communication networks are the **nervous system of CPS**, linking sensors, actuators, controllers, and interfaces.  

This chapter covers:
- Networking principles in CPS  
- Requirements: latency, reliability, scalability, security  
- IoT protocols: MQTT, CoAP, OPC UA, BLE, LoRa  
- Case study: Smart grid communication  
- Lab: MQTT-based CPS pipeline with ESP32 + Node-RED  

---

## Learning Outcomes
Students will be able to:

1. Explain the role of communication in CPS closed-loop operation.  
2. Describe wired and wireless options.  
3. Compare IoT protocols (MQTT, CoAP, OPC UA, BLE, LoRa).  
4. Identify networking challenges (latency, jitter, reliability, scalability).  
5. Implement MQTT-based CPS network with Node-RED.  

---

## Key Terms
`Latency, jitter, bandwidth, QoS, MQTT, CoAP, OPC UA, BLE, LoRa, publish/subscribe, broker`

---

## Role of Communication in CPS
CPS loop latency:
```math
D = d_sense + d_net + d_compute + d_act
```
- $d_net$ often dominates  
- Reliability + timing critical for safety  

---

## Wired vs. Wireless
- **Wired:** Ethernet, CAN bus, Modbus  
  - ✅ Deterministic, reliable  
  - ❌ Costly, poor mobility  
- **Wireless:** Wi-Fi, BLE, ZigBee, LoRa, 5G  
  - ✅ Mobility, scalability  
  - ❌ Interference, variable latency  

---

## IoT Protocols in CPS
- **MQTT:** lightweight pub/sub, ideal for many devices  
- **CoAP:** RESTful over UDP, for constrained devices  
- **OPC UA:** rich semantics, industrial standard  
- **BLE:** low-power, short range  
- **LoRa/LoRaWAN:** long-range, low-power  

---

## Networking Challenges
- **Latency & Jitter:** must meet deadlines  
- **Reliability:** avoid unsafe states  
- **Scalability:** handle 100s–1000s devices  
- **Security:** encryption, authentication, intrusion detection  

---

## Case Study: Smart Grid
- Smart meters → gateways (ZigBee/LoRa)  
- Gateways → utility (LTE/5G)  
- Utility → actuators (circuit breakers, inverters)  
- Requirements: <100 ms latency, high reliability, strong security  

---

## Node-RED Lab 5: MQTT IoT Network

### Objectives
- Set up MQTT broker  
- Publish ESP32 sensor data  
- Visualize in Node-RED  

### Requirements
- ESP32 board (LM73 or simulated input)  
- MQTT broker (Mosquitto)  
- Node-RED + dashboard  

### Procedure
1. Install Mosquitto broker  
2. ESP32 publishes `/cps/temp/room`  
3. In Node-RED:  
   - **MQTT-in** node subscribe `/cps/temp/room`  
   - **JSON** node parse  
   - Display on **gauge + chart**  

### Expected Output
- Real-time temperature displayed on dashboard gauge & chart  

### Rubric (10 points)
- [2] Broker installed  
- [3] ESP32 publishes  
- [3] Node-RED dashboard works  
- [2] Debug shows JSON payload  

---

### Starter Node-RED Flow
```json
[{"id":"tab5","type":"tab","label":"Lab5 - MQTT Network"},
{"id":"broker5","type":"mqtt-broker","name":"LocalBroker","broker":"localhost","port":"1883"},
{"id":"mqin","type":"mqtt in","z":"tab5","name":"Room Temp","topic":"/cps/temp/room",
"qos":"0","datatype":"auto","broker":"broker5","x":150,"y":120,"wires":[["json5"]]},
{"id":"json5","type":"json","z":"tab5","name":"","property":"payload","action":"",
"pretty":false,"x":340,"y":120,"wires":[["gauge5","chart5","dbg5"]]},
{"id":"gauge5","type":"ui_gauge","z":"tab5","group":"uigrp5","order":1,
"gtype":"gage","title":"Room Temperature","label":"°C","format":"{{payload.t}}",
"min":0,"max":"50","x":540,"y":100,"wires":[]},
{"id":"chart5","type":"ui_chart","z":"tab5","group":"uigrp5","order":2,
"label":"Temperature Trend","chartType":"line","legend":"false","xformat":"HH:mm:ss",
"interpolate":"linear","ymin":"0","ymax":"50","removeOlder":1,"removeOlderPoints":"300",
"removeOlderUnit":"3600","x":540,"y":160,"wires":[[]]},
{"id":"dbg5","type":"debug","z":"tab5","name":"debug","active":true,"tosidebar":true,
"complete":"true","x":540,"y":200,"wires":[]},
{"id":"uigrp5","type":"ui_group","name":"Lab5","tab":"uitab5","order":1,"disp":true,"width":"8"},
{"id":"uitab5","type":"ui_tab","name":"CPS Labs","icon":"dashboard"}]
```

---

### Starter ESP32 Code
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

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
    mqtt.connect("esp32-room");
    delay(500);
  }
}

void setup(){
  Serial.begin(115200);
  wifiConnect();
  mqttConnect();
}

void loop(){
  if(WiFi.status()!=WL_CONNECTED) wifiConnect();
  if(!mqtt.connected()) mqttConnect();

  float temp = 25.0 + (millis()%5000)/1000.0; // demo temperature
  char buf[64];
  snprintf(buf,sizeof(buf),"{\"t\":%.2f}",temp);
  mqtt.publish("/cps/temp/room", buf);

  mqtt.loop();
  delay(1000);
}
```

---

## Summary
- Communication = nervous system of CPS  
- Latency, jitter, reliability, security critical  
- MQTT = lightweight, scalable pub/sub protocol  
- Lab: ESP32 + Node-RED MQTT pipeline  

---

## Review Questions
1. Wired vs wireless: pros/cons in CPS?  
2. Compare MQTT vs CoAP.  
3. How do latency + jitter affect CPS loops?  
4. What role does broker play in pub/sub?  

---

## Exercises
1. Extend lab with 2 ESP32 nodes publishing to different topics → one chart.  
2. Experiment with MQTT QoS levels (0/1/2).  
3. Discuss MQTT security in CPS.  

---

## Further Reading
- A. Banks, R. Gupta, *MQTT v3.1.1*, OASIS, 2014  
- OPC Foundation, *OPC UA Spec*  
- E. Kovacs, “Standards and IoT Protocols for CPS,” IEEE, 2019  
- H. Kopetz, *Real-Time Systems: Design Principles*, Springer, 2011  

