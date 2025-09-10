# Chapter 4: Actuation and Control Interfaces in CPS

## Chapter Overview
Sensors provide the “eyes and ears” of CPS, while **actuators** are their “hands and muscles,” enabling CPS to influence the environment.  

This chapter covers:
- Types of actuators  
- Interfacing methods  
- Feedback control principles  
- Real-time constraints  
- Safety interlocks  

Hands-on: Use **Node-RED dashboards** to control actuators (LEDs, motors, servos) via **MQTT and ESP32**.

---

## Learning Outcomes
Students will be able to:

1. Classify actuators (electrical, mechanical, pneumatic, hydraulic).  
2. Explain the interfacing chain from cyber commands to physical actuation.  
3. Discuss feedback loops and timing constraints.  
4. Implement Node-RED dashboards to control actuators via ESP32.  
5. Recognize safety/reliability concerns in CPS actuation.  

---

## Key Terms
`Actuator, driver, PWM, H-bridge, servo, relay, feedback loop, latency, jitter, fail-safe`

---

## Actuator Types
- **Electrical:** LEDs, DC motors, stepper, servo  
- **Mechanical:** valves, pumps, robotic arms  
- **Electro-mechanical:** relays, solenoids  
- **Others:** piezoelectric, MEMS, pneumatic, hydraulic  

**Example:** A DC motor converts current into torque; a servo motor uses PWM to set angle.

---

## The Actuation Chain
1. **Cyber Command:** Node-RED/controller issues command  
2. **Interface & Driver:** PWM, H-Bridge, Relay  
3. **Actuator:** Motor, LED, Servo  

---

## Feedback and Control Loops
Actuation typically uses **closed-loop control**:

```math
u(t) = f(x_desired(t) - x_measured(t))
```

- **On/Off (Bang-Bang):** e.g., thermostat  
- **Proportional (P):** proportional to error  
- **PID:** proportional + integral + derivative  

---

## Real-Time Constraints
- **Latency:** must meet deadlines  
- **Jitter:** variability in timing → instability risk  
- **Resolution:** e.g., PWM duty cycle steps  

---

## Safety and Reliability
- Emergency stop interlocks  
- Watchdog timers  
- Fail-safe defaults (OFF state)  
- Redundancy for critical actuators  

---

## Case Study: Smart Lighting
- **Sensor:** LDR measures brightness  
- **Actuator:** LED dimmer (PWM)  
- **Control:** Node-RED compares setpoint vs. measured light → adjusts PWM  

---

## Node-RED Lab 4: Actuator Control Dashboard

### Objectives
- Use dashboard to send actuator commands  
- Control LED (ON/OFF) and servo motor (angle) via ESP32  
- Understand MQTT-based actuation  

### Hardware/Software
- ESP32 DevKit  
- LED + resistor  
- Servo motor (SG90 or similar)  
- Node-RED + MQTT broker  

### Procedure
1. In Node-RED:  
   - Add **UI switch** → `/cps/cmd/led`  
   - Add **UI slider** → `/cps/cmd/servo` (0–180°)  
   - Connect to **MQTT-out**  
2. ESP32 subscribes to topics, controls LED + servo  
3. Deploy and test  

### Expected Output
- Dashboard controls LED ON/OFF  
- Servo angle updates in real time  

### Assessment Rubric (10 points)
- [2] Dashboard UI created correctly  
- [3] MQTT messages sent Node-RED → ESP32  
- [3] LED + servo respond accurately  
- [2] Debug confirms payload  

---

### Starter Node-RED Flow
```json
[{"id":"tab4","type":"tab","label":"Lab4 - Actuator Control"},
{"id":"broker4","type":"mqtt-broker","name":"LocalBroker","broker":"localhost","port":"1883"},
{"id":"uiSwitch","type":"ui_switch","z":"tab4","name":"LED","label":"LED",
"group":"uigrp4","order":1,"topic":"/cps/cmd/led","onvalue":"ON","offvalue":"OFF",
"x":150,"y":120,"wires":[["mqout4"]]},
{"id":"slider","type":"ui_slider","z":"tab4","name":"Servo","label":"Servo",
"group":"uigrp4","order":2,"min":0,"max":"180","step":1,"x":150,"y":180,"wires":[["fnServo"]]} ,
{"id":"fnServo","type":"function","z":"tab4","name":"pack angle",
"func":"msg.topic=\"/cps/cmd/servo\";\nmsg.payload={angle:Number(msg.payload)};\nreturn msg;",
"x":340,"y":180,"wires":[["mqoutServo"]]},
{"id":"mqout4","type":"mqtt out","z":"tab4","name":"LED cmd","topic":"/cps/cmd/led",
"broker":"broker4","x":360,"y":120,"wires":[]},
{"id":"mqoutServo","type":"mqtt out","z":"tab4","name":"Servo cmd","topic":"/cps/cmd/servo",
"broker":"broker4","x":540,"y":180,"wires":[]},
{"id":"uigrp4","type":"ui_group","name":"Lab4","tab":"uitab4","order":1,"disp":true,"width":"6"},
{"id":"uitab4","type":"ui_tab","name":"CPS Labs","icon":"dashboard"}]
```

---

### Starter ESP32 Code
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";
const char* MQTT_HOST = "192.168.1.10";
const int MQTT_PORT = 1883;

const int LED_PIN = 2;
const int SERVO_PIN = 14;

Servo myServo;
WiFiClient espClient;
PubSubClient mqtt(espClient);

void wifiConnect(){ WiFi.begin(WIFI_SSID, WIFI_PASS); while(WiFi.status()!=WL_CONNECTED) delay(300); }
void mqttConnect(){ mqtt.setServer(MQTT_HOST, MQTT_PORT); while(!mqtt.connected()){ mqtt.connect("esp32-actors"); mqtt.subscribe("/cps/cmd/led"); mqtt.subscribe("/cps/cmd/servo"); delay(500);} }

void callback(char* topic, byte* payload, unsigned int len){
  String data((char*)payload,len);
  if(String(topic)=="/cps/cmd/led"){
    digitalWrite(LED_PIN, data=="ON"?HIGH:LOW);
  } else if(String(topic)=="/cps/cmd/servo"){
    StaticJsonDocument<128> doc;
    if(deserializeJson(doc,data)==DeserializationError::Ok){
      int angle = doc["angle"] | 90;
      myServo.write(constrain(angle,0,180));
    }
  }
}

void setup(){
  pinMode(LED_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);
  wifiConnect();
  mqtt.setCallback(callback);
  mqttConnect();
}

void loop(){
  if(WiFi.status()!=WL_CONNECTED) wifiConnect();
  if(!mqtt.connected()) mqttConnect();
  mqtt.loop();
}
```

---

## Summary
- Actuators = CPS effectors (hands/muscles)  
- Actuation chain: commands → drivers → actuators  
- Feedback loops ensure performance  
- Real-time constraints critical  
- Safety interlocks prevent harm  
- Lab: control LED + servo via Node-RED and ESP32  

---

## Review Questions
1. List three actuator categories with examples.  
2. What is the role of PWM in actuator control?  
3. Why do latency and jitter matter?  
4. Give one fail-safe mechanism.  

---

## Exercises
1. Add buzzer actuator via MQTT.  
2. Implement bang-bang LED controller in Node-RED (ON if temp >30°C).  
3. Sketch PID motor control loop.  

---

## Further Reading
- H. K. Khalil, *Nonlinear Systems*, Prentice Hall, 2002.  
- K. Ogata, *Modern Control Engineering*, 2010.  
- D. E. Seborg et al., *Process Dynamics and Control*, 2016.  

