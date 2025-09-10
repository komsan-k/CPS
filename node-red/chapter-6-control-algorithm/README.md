# Chapter 6: Discrete Control Algorithms in CPS

## Chapter Overview
Control algorithms are the **brains of CPS**, mapping sensor data to actuator commands.  

This chapter covers:
- Discrete control principles  
- Rule-based (bang-bang) logic  
- Finite State Machines (FSMs)  
- Classical controllers (P, PI, PID) in digital implementations  
- Lab: Rule-based control with Node-RED + ESP32  

---

## Learning Outcomes
Students will be able to:

1. Describe the role of discrete control in CPS.  
2. Implement rule-based controllers with logic conditions.  
3. Model system behavior using FSMs.  
4. Apply P/PI/PID controllers in discrete time.  
5. Build automated Node-RED flows with ESP32 sensors/actuators.  

---

## Key Terms
`Control loop, setpoint, error, FSM, bang-bang, proportional, integral, derivative, sampling period, stability`

---

## From Continuous to Discrete Control
- Continuous:
```math
u(t) = f(x_desired(t) - x_measured(t))
```
- Discrete:
```math
u[k] = f(x_d[k] - x_m[k])
```
- Constraint: $T_s \le D$ (deadline)

---

## Rule-Based (Bang-Bang) Control
**Concept:**
```math
u[k] =
  ON,  if x[k] > x_high
  OFF, if x[k] < x_low
```

**Example:** Fan ON if $T>30°C$, OFF if $T<25°C$

---

## Finite State Machines (FSMs)
- **States:** OFF, IDLE, ACTIVE  
- **Transitions:** sensor thresholds, timers  
- **Actions:** entry/exit behaviors  

Example: thermostat FSM (OFF ↔ ON)

---

## Classical Controllers
- **P:** $u[k] = K_p e[k]$  
- **PI:** $u[k] = K_p e[k] + K_i \sum e[i] T_s$  
- **PID:** $u[k] = K_p e[k] + K_i \sum e[i] T_s + K_d rac{e[k]-e[k-1]}{T_s}$  

Applications: motor speed, temperature, position  

---

## Timing & Stability
- Slow sampling → instability  
- Jitter → oscillations  
- Quantization → dead zones  

---

## Case Study: Room Temp Control
- **Sensor:** ESP32 measures temperature  
- **Controller:** Node-RED IF logic  
- **Actuator:** Fan toggled ON/OFF  
- Outcome: Temp stabilized between 25–30°C  

---

## Node-RED Lab 6: Automated CPS Control

### Objectives
- Implement threshold-based controller  
- Auto-trigger actuators from sensor data  
- Visualize on dashboard  

### Requirements
- ESP32 + temperature sensor  
- LED or fan actuator  
- Node-RED + MQTT broker  

### Procedure
1. ESP32 publishes `/cps/temp/room`  
2. Node-RED flow:  
   - **MQTT-in** → **JSON** → **Function (IF-logic)** → **MQTT-out**  
   - Dashboard UI shows status  

**Function Node Code:**
```js
let t = Number(msg.payload.t);
msg.topic = "/cps/cmd/fan";
msg.payload = (t > 30) ? "ON" : "OFF";
return msg;
```

### Expected Output
- Temp >30°C → fan ON  
- Temp <30°C → fan OFF  
- Dashboard shows fan status  

### Rubric (10 points)
- [2] ESP32 publishes valid data  
- [3] Node-RED logic correct  
- [3] Actuator responds  
- [2] Dashboard displays correctly  

---

### Starter Node-RED Flow
```json
[{"id":"tab6","type":"tab","label":"Lab6 - Auto Control"},
{"id":"broker6","type":"mqtt-broker","name":"LocalBroker","broker":"localhost","port":"1883"},
{"id":"mqin6","type":"mqtt in","z":"tab6","name":"Temp in",
"topic":"/cps/temp/room","qos":"0","datatype":"auto","broker":"broker6",
"x":120,"y":120,"wires":[["json6"]]},
{"id":"json6","type":"json","z":"tab6","name":"","property":"payload",
"action":"","pretty":false,"x":290,"y":120,"wires":[["fn6"]]},
{"id":"fn6","type":"function","z":"tab6","name":"IF temp>30",
"func":"let t = Number(msg.payload.t);\nmsg.topic = \"/cps/cmd/fan\";\nmsg.payload = (t>30)?\"ON\":\"OFF\";\nreturn msg;",
"outputs":1,"x":470,"y":120,"wires":[["mqout6","status6"]]},
{"id":"mqout6","type":"mqtt out","z":"tab6","name":"Fan cmd",
"topic":"/cps/cmd/fan","qos":"","retain":"","broker":"broker6","x":650,"y":120,"wires":[]},
{"id":"status6","type":"ui_text","z":"tab6","group":"uigrp6","order":1,
"label":"Fan status","format":"{{msg.payload}}","x":650,"y":170,"wires":[]},
{"id":"uigrp6","type":"ui_group","name":"Lab6","tab":"uitab6","order":1,"disp":true,"width":"6"},
{"id":"uitab6","type":"ui_tab","name":"CPS Labs","icon":"dashboard"}]
```

---

### Starter ESP32 Code
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID="YOUR_WIFI";
const char* WIFI_PASS="YOUR_PASS";
const char* MQTT_HOST="192.168.1.10";
const int MQTT_PORT=1883;

const int FAN_PIN = 5;

WiFiClient espClient;
PubSubClient mqtt(espClient);

void wifiConnect(){ WiFi.begin(WIFI_SSID, WIFI_PASS); while(WiFi.status()!=WL_CONNECTED) delay(300); }
void mqttConnect(){ mqtt.setServer(MQTT_HOST,MQTT_PORT); while(!mqtt.connected()){ mqtt.connect("esp32-room"); mqtt.subscribe("/cps/cmd/fan"); delay(500);} }

void callback(char* topic, byte* payload, unsigned int len){
  String data((char*)payload,len);
  if(String(topic)=="/cps/cmd/fan"){
    digitalWrite(FAN_PIN, data=="ON"?HIGH:LOW);
  }
}

float readRoomTemp(){ return 25.0 + 10.0*sin(millis()/5000.0); } // demo

void setup(){
  pinMode(FAN_PIN,OUTPUT);
  wifiConnect();
  mqtt.setCallback(callback);
  mqttConnect();
}

void loop(){
  if(WiFi.status()!=WL_CONNECTED) wifiConnect();
  if(!mqtt.connected()) mqttConnect();

  float t=readRoomTemp();
  char buf[64];
  snprintf(buf,sizeof(buf),"{\"t\":%.2f}",t);
  mqtt.publish("/cps/temp/room",buf);

  mqtt.loop();
  delay(1000);
}
```

---

## Summary
- Discrete control = computational brain of CPS  
- Rule-based + FSM handle events  
- P/PI/PID regulate continuous processes  
- Lab: Auto actuation via Node-RED + ESP32  

---

## Review Questions
1. Difference between continuous vs discrete control?  
2. How does an FSM represent CPS modes?  
3. Write discrete PID formula and explain terms.  
4. Why are timing constraints important?  

---

## Exercises
1. Add hysteresis (fan ON >30°C, OFF <28°C).  
2. FSM for smart light: OFF, DIM, BRIGHT.  
3. Simulate proportional controller in Node-RED with $K_p$ slider.  

---

## Further Reading
- K. Ogata, *Discrete-Time Control Systems*, 1995  
- E. A. Lee & S. Tripakis, *Foundations of CPS Modeling*, 2012  
- H. K. Khalil, *Nonlinear Systems*, 2002  

