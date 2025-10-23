# 🧩 Lab 6: Cloud-Connected Cyber-Physical Systems (CPS) and IoT Integration

### 🎯 Objective
Students will learn to:
1. Stream CPS simulation data to the cloud or IoT broker.
2. Emulate real-time sensor publishing using Modelica signals.
3. Implement MQTT/REST integration and dashboard visualization.

---

## 1. Prerequisites
- Completion of Lab 5: Digital Twin and Visualization.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
Cloud connectivity extends CPS to IoT dashboards for remote monitoring and analytics.

### CPS–IoT Architecture
```
[Physical System] → [Modelica Simulation] → [Data Export/Stream]
                              ↓
                       [MQTT Broker / REST API]
                              ↓
                    [IoT Dashboard / Cloud App]
```

---

## 3. CPS System Model
```modelica
model CloudCPS
  import MyLibrary.*;
  PhysicalPlant plant;
  PIDController pid(Kp=3, Ki=0.5, Kd=0.2);
  DigitalTwin twin(alpha=3.0);
  Real x_ref = 1;
  Real x_meas;
  Real u_cmd;
equation
  twin.x_true = plant.x;
  twin.v_true = plant.v;
  x_meas = twin.x_pred;
  pid.x_ref = x_ref;
  pid.x_meas = x_meas;
  u_cmd = pid.u;
  plant.act.u = u_cmd;
end CloudCPS;
```

---

## 4. Data Publisher (CSV Export)
```modelica
model DataPublisher
  CloudCPS sys;
  parameter Real Ts = 0.5 "Sampling interval (s)";
  String filename = "cps_data.csv";
  Integer fileID;
initial algorithm
  fileID := OpenModelica.Scripting.openFile(filename, "w");
  OpenModelica.Scripting.writeFile(fileID, "time,x_meas,u_cmd\n");
equation
  when sample(0, Ts) then
    OpenModelica.Scripting.writeFile(fileID,
      String(time) + "," + String(sys.plant.x) + "," + String(sys.u_cmd) + "\n");
  end when;
end DataPublisher;
```

✅ CSV file represents sensor + actuator data ready for IoT upload.

---

## 5. Python MQTT Publisher (External)
```python
import paho.mqtt.client as mqtt
import csv, time

broker = "test.mosquitto.org"
topic = "cps/lab6/data"
client = mqtt.Client()
client.connect(broker, 1883, 60)

with open("cps_data.csv", "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        payload = f"{row['time']},{row['x_meas']},{row['u_cmd']}"
        client.publish(topic, payload)
        print("Published:", payload)
        time.sleep(0.5)

client.disconnect()
```

✅ Verify data on [HiveMQ Web MQTT Client](https://www.hivemq.com/demos/websocket-client/) — topic `cps/lab6/data`.

---

## 6. IoT Dashboard Integration
| Platform | Type | Example |
|-----------|------|----------|
| Node-RED | Local/Cloud | Gauge + Chart (MQTT input node) |
| ThingSpeak | REST API | Upload CSV to visualize signals |
| AWS IoT Core | Cloud | MQTT via certificate |
| Google Colab | Python | Plotly-based animation |

---

### Node-RED Flow Example
```json
[
  {"id":"mqtt-in","type":"mqtt in","topic":"cps/lab6/data"},
  {"id":"chart","type":"ui_chart","group":"CPS Lab 6","x":10,"y":20}
]
```

---

## 7. Communication Delay Simulation
```modelica
when sample(0, Ts) then
  OpenModelica.Scripting.writeFile(fileID,
    String(time) + "," + String(delay(sys.plant.x, 0.1)) + "," +
    String(delay(sys.u_cmd, 0.1)) + "\n");
end when;
```

✅ Emulates 100 ms network delay.

---

## 8. Example Output Data
| time (s) | x_meas | u_cmd |
|-----------|--------|--------|
| 0.0 | 0.000 | 3.000 |
| 0.5 | 0.212 | 2.345 |
| 1.0 | 0.487 | 1.122 |
| 1.5 | 0.802 | 0.583 |
| 2.0 | 0.972 | 0.045 |

---

## 9. Discussion
- Latency degrades control precision.
- Sampling rate affects dashboard smoothness.
- Cloud delay can destabilize real-time CPS.
- Predictive digital twin compensates delay.

---

## 10. Exercises
1. Change Ts = 0.2 and compare data density.
2. Introduce random packet loss: `if random(0)>0.2 then write...`.
3. Display data in Node-RED using gauge + chart.
4. Compute moving average in Python before publishing.
5. Simulate multiple publishers for CPS network.

---

## 11. Learning Outcomes
- Integrate CPS simulation with IoT cloud platforms.
- Stream real-time data using `sample()` and scripting functions.
- Visualize CPS signals on MQTT/ThingSpeak dashboards.
- Analyze network latency and sampling trade-offs.

---

## 12. Next Lab
**Lab 7:** Digital Twin Synchronization and Predictive Maintenance.

