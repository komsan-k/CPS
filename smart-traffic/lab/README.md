
# Smart City CPS Lab  
## Digital Twin of a Single Smart Traffic Intersection  
*(ESP32 + MQTT + Node-RED + AI Prediction)*

---

# 1. Lab Title
**Cyber-Physical Digital Twin for a Smart Traffic Intersection**

---

# 2. Learning Objectives

After completing this lab, students will be able to:

- Understand the concept of a **Digital Twin in Smart Cities**
- Implement a **Cyber-Physical System (CPS)** for traffic control
- Use **ESP32 sensors** to collect traffic data
- Send real-time data using **MQTT**
- Build a **Node-RED Digital Twin Dashboard**
- Implement **AI prediction** for traffic congestion
- Control traffic lights automatically

---

# 3. CPS Architecture of the Lab

The lab follows the **CPS 5C Architecture**.

| CPS Layer | Implementation |
|----------|----------------|
| Connection | Traffic sensors |
| Conversion | Data preprocessing |
| Cyber | Digital twin dashboard |
| Cognition | AI traffic prediction |
| Configuration | Traffic light control |

---

# 4. System Overview

The system simulates **one smart intersection**.

## Physical System
- Traffic lights
- Vehicle detection sensors
- ESP32 controller

## Cyber System
- MQTT broker
- Node-RED Digital Twin
- AI traffic prediction model

## Control System
- Adaptive traffic signal control

---

# 5. Intersection Model

The intersection has **four roads**.

```
        North
          |
          |
West ---- + ---- East
          |
          |
        South
```

Traffic lights control:

- North–South direction
- East–West direction

---

# 6. Hardware Requirements

| Component | Quantity |
|----------|----------|
| ESP32 DevKit | 1 |
| LED (Red) | 2 |
| LED (Yellow) | 2 |
| LED (Green) | 2 |
| Resistors (220Ω) | 6 |
| IR vehicle sensor | 2 |
| Breadboard | 1 |

---

# 7. Pin Configuration

| Signal | ESP32 Pin |
|------|-----------|
| Red NS | GPIO 23 |
| Yellow NS | GPIO 19 |
| Green NS | GPIO 18 |
| Red EW | GPIO 5 |
| Yellow EW | GPIO 17 |
| Green EW | GPIO 16 |

### Vehicle Sensors

| Sensor | Pin |
|-------|-----|
| North sensor | GPIO 34 |
| East sensor | GPIO 35 |

---

# 8. System Architecture

System data flow:

```
Vehicle Sensors
      ↓
ESP32 Controller
      ↓
MQTT Broker
      ↓
Node-RED Digital Twin
      ↓
AI Traffic Prediction
      ↓
Control Decision
      ↓
Traffic Lights
```

---

# 9. Digital Twin Data Model

Traffic state vector:

```
x_t = [v_NS, v_EW, q_NS, q_EW]
```

Where:

- **v_NS** = vehicles north–south  
- **v_EW** = vehicles east–west  
- **q_NS** = queue length north–south  
- **q_EW** = queue length east–west  

Prediction model:

```
x(t+1) = f(x(t), u(t), d(t))
```

Where:

- **u(t)** = signal timing
- **d(t)** = disturbances (weather / random arrivals)

---

# 10. ESP32 Program

Example Arduino code:

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "WiFi";
const char* password = "password";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

int sensorNS = 34;
int sensorEW = 35;

int redNS = 23;
int yellowNS = 19;
int greenNS = 18;

int redEW = 5;
int yellowEW = 17;
int greenEW = 16;

void setup() {

  pinMode(sensorNS, INPUT);
  pinMode(sensorEW, INPUT);

  pinMode(redNS, OUTPUT);
  pinMode(yellowNS, OUTPUT);
  pinMode(greenNS, OUTPUT);

  pinMode(redEW, OUTPUT);
  pinMode(yellowEW, OUTPUT);
  pinMode(greenEW, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
}

void loop() {

  int trafficNS = digitalRead(sensorNS);
  int trafficEW = digitalRead(sensorEW);

  String payload = String("{\"NS\":") + trafficNS +
                   ",\"EW\":" + trafficEW + "}";

  client.publish("city/intersection1/traffic", payload.c_str());

  delay(1000);
}
```

---

# 11. Node-RED Flow

Node-RED receives sensor data.

```
MQTT Input
      ↓
JSON Parser
      ↓
Traffic Density Calculation
      ↓
Dashboard Gauge
      ↓
AI Prediction
      ↓
Control Logic
      ↓
MQTT Output
```

Dashboard shows:

- Vehicle count
- Congestion level
- Signal timing

---

# 12. Traffic Prediction Algorithm

Simple prediction model:

```
Traffic(t+1) = Traffic(t) + Arrival − Departure
```

Example rule:

```
IF NS traffic > EW traffic
THEN increase NS green time
```

---

# 13. Adaptive Signal Control

Signal phases:

### Phase 1
NS = GREEN  
EW = RED

### Phase 2
NS = RED  
EW = GREEN

Adaptive rule:

```
IF congestion_NS > congestion_EW
increase green_NS
```

---

# 14. Digital Twin Dashboard

Node-RED dashboard displays:

| Parameter | Display |
|----------|---------|
| Vehicle count | Gauge |
| Congestion | Chart |
| Signal phase | Indicator |
| AI prediction | Graph |

---

# 15. Example Operation

Step 1 — Vehicle detected on North road

Step 2 — ESP32 sends data

Topic:

```
city/intersection1/traffic
```

Step 3 — Node-RED updates digital twin

Step 4 — AI predicts congestion

Step 5 — Control logic adjusts signal timing

---

# 16. Experiment Tasks

Students must perform:

1. Build the ESP32 traffic light controller
2. Connect vehicle sensors
3. Publish traffic data using MQTT
4. Create a Node-RED dashboard
5. Implement adaptive signal control

---

# 17. Evaluation Metrics

| Metric | Description |
|------|-------------|
| Average delay | Vehicle waiting time |
| Queue length | Number of vehicles |
| Throughput | Vehicles per minute |
| Signal efficiency | Green utilization |

---

# 18. Expected Results

Students should observe:

- Reduced congestion
- Adaptive traffic lights
- Real-time digital twin dashboard

---

# 19. Extension Exercises

Students can extend the lab by:

- Adding camera-based vehicle detection
- Using AI traffic prediction (TensorFlow Lite)
- Expanding to multi-intersection digital twin
- Adding emergency vehicle priority

---

# 20. Discussion Questions

- What is the role of the Digital Twin in traffic management?
- How does MQTT improve IoT communication?
- Why is predictive control better than fixed signal timing?
- How could AI improve traffic optimization?

---

# 21. Conclusion

This lab demonstrates a **Cyber-Physical Digital Twin for smart traffic control**.

Students build a complete CPS system integrating:

- IoT sensors
- ESP32 edge computing
- MQTT communication
- Node-RED digital twin
- AI-based decision making

The system illustrates how **Digital Twin technology can improve urban traffic management in Smart Cities**.
