# Unsupervised Learning for an LDR Light Sensor  
## ESP32 + MQTT + Python K-means + Node-RED Digital Twin Lab

## 1. Objective

This lab demonstrates how to deploy an **unsupervised learning algorithm** for an **LDR (Light Dependent Resistor)** using an **ESP32**, **Python**, **MQTT**, and **Node-RED**.

The system will:

- read ambient light using an LDR
- collect sensor data with ESP32
- apply **K-means clustering** in Python
- discover natural light groups automatically
- deploy learned thresholds back to ESP32
- visualize the live state in Node-RED

The discovered states are interpreted as:

- **DARK**
- **NORMAL**
- **BRIGHT**

---

## 2. System Overview

The deployment pipeline is:

```text
LDR Sensor
   ↓
ESP32 ADC
   ↓
MQTT / Serial Data Collection
   ↓
Python K-means Clustering
   ↓
Cluster Centroids and Thresholds
   ↓
ESP32 Real-time Classification
   ↓
Node-RED Dashboard / Digital Twin
```

This is a practical example of **edge deployment of unsupervised learning** in a cyber-physical system.

---

## 3. Required Hardware

- ESP32 DevKit
- LDR sensor
- 10 kΩ resistor
- Breadboard
- Jumper wires
- USB cable
- Optional LED with 220 Ω resistor

---

## 4. Circuit Setup

### 4.1 LDR Voltage Divider

```text
3.3V ---- LDR ----+---- GPIO34
                  |
                10kΩ
                  |
                 GND
```

### 4.2 Optional LED

```text
GPIO2 ---- 220Ω ---- LED ---- GND
```

Recommended pins:

- **GPIO34** for LDR analog input
- **GPIO2** for optional LED output

---

## 5. MQTT Topics

Use the following topics:

```text
cps/ldr/raw
cps/ldr/state
cps/ldr/cmd
```

### Topic meanings

- `cps/ldr/raw` → raw telemetry from ESP32
- `cps/ldr/state` → cluster classification and LED state
- `cps/ldr/cmd` → Node-RED commands such as AUTO, MANUAL_ON, MANUAL_OFF

---

## 6. Phase 1: ESP32 Data Acquisition

Upload this simple sketch first to collect raw LDR values.

```cpp
#include <WiFi.h>

const int LDR_PIN = 34;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int adc = analogRead(LDR_PIN);
  Serial.println(adc);
  delay(500);
}
```

### Data collection instructions

Collect data under different lighting conditions:

- dark room
- normal indoor light
- near a window
- direct flashlight
- partial shading

Store the values in a CSV file named:

```text
ldr_data.csv
```

Example:

```csv
adc
120
150
180
950
1020
3100
3250
```

Try to collect at least **500 to 2000 samples**.

---

## 7. Phase 2: Python K-means Training

Use this Python script to train the unsupervised model.

```python
import pandas as pd
import numpy as np
from sklearn.cluster import KMeans
from sklearn.preprocessing import MinMaxScaler
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv("ldr_data.csv")

# Feature
X = df[["adc"]].values

# Normalize
scaler = MinMaxScaler()
X_scaled = scaler.fit_transform(X)

# K-means clustering
kmeans = KMeans(n_clusters=3, random_state=42, n_init=10)
labels = kmeans.fit_predict(X_scaled)

df["cluster"] = labels

# Sort centroids
centroids = np.sort(kmeans.cluster_centers_.flatten())

# Thresholds between clusters
t1 = (centroids[0] + centroids[1]) / 2
t2 = (centroids[1] + centroids[2]) / 2

print("Sorted centroids:", centroids)
print("Threshold 1:", t1)
print("Threshold 2:", t2)

# Convert thresholds back to ADC range
adc_min = df["adc"].min()
adc_max = df["adc"].max()

adc_t1 = adc_min + t1 * (adc_max - adc_min)
adc_t2 = adc_min + t2 * (adc_max - adc_min)

print("ADC min:", adc_min)
print("ADC max:", adc_max)
print("Threshold 1 in ADC:", adc_t1)
print("Threshold 2 in ADC:", adc_t2)

# Plot
plt.figure(figsize=(8, 4))
plt.scatter(range(len(X_scaled)), X_scaled, c=labels, s=12)
plt.axhline(t1, linestyle='--')
plt.axhline(t2, linestyle='--')
plt.xlabel("Sample index")
plt.ylabel("Normalized ADC")
plt.title("K-means clustering for LDR sensor")
plt.show()

# Save labeled data
df.to_csv("ldr_clustered_output.csv", index=False)
```

### Example output

```text
Sorted centroids: [0.11 0.44 0.87]
Threshold 1: 0.275
Threshold 2: 0.655
```

Interpretation:

- below `0.275` → **DARK**
- between `0.275` and `0.655` → **NORMAL**
- above `0.655` → **BRIGHT**

---

## 8. Phase 3: ESP32 MQTT Deployment Sketch

After training, copy the learned thresholds into the sketch below.

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

// ===============================
// User settings
// ===============================
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_HOST     "192.168.1.10"
#define MQTT_PORT     1883

// ===============================
// Pins
// ===============================
const int LDR_PIN = 34;
const int LED_PIN = 2;

// ===============================
// MQTT topics
// ===============================
const char* TOPIC_RAW   = "cps/ldr/raw";
const char* TOPIC_STATE = "cps/ldr/state";
const char* TOPIC_CMD   = "cps/ldr/cmd";

// ===============================
// Learned thresholds from Python
// ===============================
float THRESHOLD1 = 0.275f;
float THRESHOLD2 = 0.655f;

// ===============================
// Normalization
// ===============================
float ADC_MIN = 0.0f;
float ADC_MAX = 4095.0f;

// ===============================
// Smoothing
// ===============================
const int WINDOW_SIZE = 10;
int samples[WINDOW_SIZE];
int sampleIndex = 0;
bool bufferFilled = false;

// ===============================
// Networking
// ===============================
WiFiClient espClient;
PubSubClient client(espClient);

// ===============================
// State
// ===============================
String currentMode = "AUTO";
String currentState = "UNKNOWN";
bool manualLedState = false;

// ===============================
// Helper functions
// ===============================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float getMovingAverage(int newValue) {
  samples[sampleIndex] = newValue;
  sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;

  if (sampleIndex == 0) bufferFilled = true;

  int count = bufferFilled ? WINDOW_SIZE : sampleIndex;
  if (count == 0) count = 1;

  long sum = 0;
  for (int i = 0; i < count; i++) {
    sum += samples[i];
  }
  return (float) sum / count;
}

String classifyLight(float normValue) {
  if (normValue < THRESHOLD1) return "DARK";
  else if (normValue < THRESHOLD2) return "NORMAL";
  else return "BRIGHT";
}

void applyAutoControl(const String& state) {
  if (state == "DARK") digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}

void publishTelemetry(int adcRaw, float adcSmooth, float normValue, const String& state) {
  String rawPayload = "{";
  rawPayload += "\"adc_raw\":" + String(adcRaw) + ",";
  rawPayload += "\"adc_smooth\":" + String(adcSmooth, 2) + ",";
  rawPayload += "\"norm\":" + String(normValue, 4) + ",";
  rawPayload += "\"mode\":\"" + currentMode + "\"";
  rawPayload += "}";

  String statePayload = "{";
  statePayload += "\"state\":\"" + state + "\",";
  statePayload += "\"led\":" + String(digitalRead(LED_PIN) ? 1 : 0) + ",";
  statePayload += "\"threshold1\":" + String(THRESHOLD1, 4) + ",";
  statePayload += "\"threshold2\":" + String(THRESHOLD2, 4);
  statePayload += "}";

  client.publish(TOPIC_RAW, rawPayload.c_str(), true);
  client.publish(TOPIC_STATE, statePayload.c_str(), true);
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "AUTO") {
    currentMode = "AUTO";
  }
  else if (cmd == "MANUAL_ON") {
    currentMode = "MANUAL";
    manualLedState = true;
    digitalWrite(LED_PIN, HIGH);
  }
  else if (cmd == "MANUAL_OFF") {
    currentMode = "MANUAL";
    manualLedState = false;
    digitalWrite(LED_PIN, LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char) payload[i];
  }

  if (String(topic) == TOPIC_CMD) {
    handleCommand(msg);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    String clientId = "ESP32-LDR-" + String((uint32_t) ESP.getEfuseMac(), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(TOPIC_CMD);
    }
    else {
      delay(2000);
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    samples[i] = 0;
  }

  connectWiFi();

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  int adcRaw = analogRead(LDR_PIN);
  float adcSmooth = getMovingAverage(adcRaw);
  float normValue = clamp01((adcSmooth - ADC_MIN) / (ADC_MAX - ADC_MIN));
  String state = classifyLight(normValue);
  currentState = state;

  if (currentMode == "AUTO") {
    applyAutoControl(state);
  }

  publishTelemetry(adcRaw, adcSmooth, normValue, state);

  Serial.print("ADC Raw: ");
  Serial.print(adcRaw);
  Serial.print(" | Smooth: ");
  Serial.print(adcSmooth, 2);
  Serial.print(" | Norm: ");
  Serial.print(normValue, 4);
  Serial.print(" | State: ");
  Serial.print(state);
  Serial.print(" | Mode: ");
  Serial.println(currentMode);

  delay(1000);
}
```

---

## 9. Node-RED Dashboard Design

Create a dashboard with:

- **Gauge** for smooth ADC value
- **Chart** for live light trend
- **Text** for cluster state
- **Text** for control mode
- **Text** for LED state
- **Buttons** for:
  - AUTO
  - MANUAL ON
  - MANUAL OFF

### Suggested flow

```text
MQTT IN (cps/ldr/raw)   → JSON → gauge, chart, mode text
MQTT IN (cps/ldr/state) → JSON → state text, LED text, threshold text
Buttons                 → MQTT OUT (cps/ldr/cmd)
```

---

## 10. Deployment Procedure

### Step 1
Wire the LDR voltage divider to GPIO34.

### Step 2
Start the MQTT broker.

### Step 3
Upload the simple data-acquisition sketch and collect samples.

### Step 4
Save the values to `ldr_data.csv`.

### Step 5
Run the Python K-means script.

### Step 6
Copy `THRESHOLD1` and `THRESHOLD2` into the ESP32 MQTT sketch.

### Step 7
Upload the deployment sketch to ESP32.

### Step 8
Create the Node-RED dashboard and subscribe to the MQTT topics.

### Step 9
Test under different light conditions:
- fully covered LDR
- room light
- direct flashlight
- window light

---

## 11. Expected Behavior

### DARK
- cluster state = DARK
- LED turns ON in AUTO mode

### NORMAL
- cluster state = NORMAL
- LED stays OFF

### BRIGHT
- cluster state = BRIGHT
- LED stays OFF

### MANUAL mode
- dashboard buttons override automatic LED behavior

---

## 12. CPS 5C Interpretation

### Connection
LDR acquires ambient light.

### Conversion
ADC smoothing and normalization convert raw data into usable digital features.

### Cyber
Python clustering discovers data structure and generates thresholds.

### Cognition
The system interprets current light level as DARK, NORMAL, or BRIGHT.

### Configuration
The LED and dashboard behavior adapt according to the detected state.

---

## 13. Suggested Student Tasks

1. Build the LDR voltage divider and verify analog readings.
2. Collect at least 500 samples.
3. Run K-means with `n_clusters = 3`.
4. Interpret the discovered clusters.
5. Deploy thresholds to ESP32.
6. Observe behavior in Node-RED.
7. Test AUTO versus MANUAL modes.
8. Change the moving-average window size and compare stability.
9. Try `n_clusters = 4` and discuss the result.
10. Propose a real application such as smart lighting or greenhouse monitoring.

---

## 14. Discussion Questions

1. Why is K-means suitable for this problem?
2. What is the difference between training and deployment here?
3. Why is threshold deployment lighter than running K-means directly on ESP32?
4. How does smoothing affect cluster stability?
5. What happens if the light environment changes over time?
6. How could this system be extended to online adaptive clustering?

---

## 15. Extensions

You can extend this lab in several ways:

- **DBSCAN** for anomaly detection
- **Hierarchical clustering** for exploratory analysis
- **4-state clustering** such as dark, dim, normal, bright
- **RGB LED or PWM lamp control**
- **historical logging in Node-RED**
- **greenhouse or classroom digital twin**
- **multimodal clustering using LDR + RSSI + temperature**

---


## 16. Key Practical Note

After training, the main values to update are:

```cpp
float THRESHOLD1 = 0.275f;
float THRESHOLD2 = 0.655f;
```

Also update:

- Wi-Fi SSID and password
- MQTT broker IP
- GPIO pins if needed

---

## 17. Conclusion

This lab shows a practical path for deploying **unsupervised learning on an LDR light sensor** in an **ESP32-based cyber-physical system**. The training phase is handled in Python with K-means, while the deployment phase uses lightweight threshold inference on the microcontroller. Combined with MQTT and Node-RED, the result is a simple but effective digital twin platform for intelligent light-state monitoring.

