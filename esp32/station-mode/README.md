# ESP32 Station Mode

## Wi-Fi Client Mode for CPS / IoT Applications

---

## 1. Introduction

**ESP32 Station Mode (STA mode)** means the ESP32 works like a **Wi-Fi client**.

It connects to an existing **Wi-Fi router or access point**, just like:

* a smartphone
* a laptop
* a tablet
* another IoT device

In this mode, the ESP32 joins an existing wireless network.

---

## 2. What It Does

In station mode, the ESP32 can:

* join a Wi-Fi network using **SSID and password**
* obtain an **IP address** from the router
* communicate with:

  * internet servers
  * MQTT brokers
  * Node-RED
  * web servers
  * cloud APIs
  * other devices on the same network

---

## 3. Typical Use in CPS / IoT

Station mode is commonly used when the ESP32 must send or receive data through an existing network.

Typical examples include:

* publishing sensor data to MQTT
* sending HTTP requests
* connecting to Node-RED dashboard
* receiving commands from cloud or local servers
* IoT monitoring systems
* CPS digital twin communication

---

## 4. Basic Architecture

```text
Router / Wi-Fi AP  ⇄  ESP32 (STA Mode)
```

For CPS systems:

```text
Sensor → ESP32 → Wi-Fi Router → MQTT / Node-RED / Cloud
```

---

## 5. Arduino Example Code

```cpp
#include <WiFi.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);     // set ESP32 as station
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // your code here
}
```

---

## 6. Important Command

The key command is:

```cpp
WiFi.mode(WIFI_STA);
```

This sets the ESP32 to **station mode**.

---

## 7. Common Wi-Fi Modes in ESP32

| Mode          | Description                   |
| ------------- | ----------------------------- |
| `WIFI_STA`    | Station mode                  |
| `WIFI_AP`     | Access Point mode             |
| `WIFI_AP_STA` | Both station and access point |

---

## 8. When to Use Station Mode

Use station mode when:

* you already have a Wi-Fi router
* the ESP32 needs internet access
* LAN communication is required
* you want MQTT connectivity
* Node-RED communication is needed
* cloud API access is required

---

## 9. When Not to Use It

Do **not use only station mode** if:

* there is **no router available**
* you want the ESP32 itself to provide Wi-Fi
* phones/laptops need to connect directly to ESP32

In that case, use:

```cpp
WIFI_AP
```

which is **Access Point mode**.

---

## 10. Example Applications

Typical CPS / IoT applications include:

* ESP32 sends temperature to Node-RED
* ESP32 publishes LDR data to MQTT broker
* ESP32 communicates with cloud API
* ESP32 receives ON/OFF commands for LED control
* RSSI monitoring systems
* smart traffic CPS systems
* digital twin dashboards

---

## 11. Summary

ESP32 station mode is the most common mode for IoT and CPS applications where the device must connect to an existing Wi-Fi infrastructure.

It is ideal for:

* MQTT
* Node-RED
* cloud systems
* dashboards
* sensor networking

---
