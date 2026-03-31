# ESP32 Access Point (AP) Mode

## Wi-Fi Hotspot Mode for CPS / IoT Labs

---

## 1. Introduction

**ESP32 AP mode** means the ESP32 works as a **Wi-Fi Access Point (hotspot)**.

Instead of joining an existing router, it **creates its own Wi-Fi network** that other devices can connect to.

This is very useful in **Cyber-Physical Systems (CPS)** and **IoT labs**, especially when no external router is available.

---

## 2. ESP32 Access Point (AP) Mode

In AP mode, the ESP32 behaves like a **mini Wi-Fi router**.

Devices such as:

* smartphones
* laptops
* tablets
* another ESP32

can directly connect to it.

---

## 3. System Architecture

```text id="svdcju"
Phone / Laptop / Node-RED Client
           ⇅
      Wi-Fi Network
   (created by ESP32)
           ⇅
        ESP32 AP
```

---

## 4. What AP Mode Does

The ESP32:

* broadcasts its own **SSID (Wi-Fi name)**
* uses a **password**
* assigns local IP addresses to connected devices
* can host a **web server / dashboard**
* can receive **control commands directly**

Typical use cases:

* local dashboard control
* sensor monitoring
* lab demos without internet
* digital twin experiments
* direct phone-to-ESP32 communication

---

## 5. Basic Arduino Code

```cpp id="y48f7r"
#include <WiFi.h>

const char* ssid = "ESP32_CPS_Lab";
const char* password = "12345678";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);   // set AP mode
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
}
```

---

## 6. Important Command

The key command is:

```cpp id="4kkr3a"
WiFi.mode(WIFI_AP);
```

This sets the ESP32 to **Access Point mode**.

---

## 7. Create Hotspot

Use:

```cpp id="jlwmrm"
WiFi.softAP(ssid, password);
```

This creates the Wi-Fi network.

Example:

* **SSID:** `ESP32_CPS_Lab`
* **Password:** `12345678`

---

## 8. Default IP Address

Usually the ESP32 AP IP is:

```text id="8v5h6u"
192.168.4.1
```

So after connecting your phone/laptop to the ESP32 Wi-Fi, you can access:

```text id="m9knvx"
http://192.168.4.1
```

if you run a web server.

---

## 9. Example: AP Mode + Web Control

This is common for LED control.

```cpp id="0af3yv"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_AP";
const char* password = "12345678";

WebServer server(80);
const int LED_PIN = 2;

void handleRoot() {
  server.send(200, "text/html",
              "<h1>ESP32 AP Mode</h1>"
              "<a href='/on'>LED ON</a><br>"
              "<a href='/off'>LED OFF</a>");
}

void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "LED ON");
}

void handleOff() {
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "LED OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();

  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
```

---

## 10. When to Use AP Mode

Use AP mode when:

* no router is available
* classroom / lab setup
* local CPS demo
* phone controls ESP32 directly
* fast prototyping

Excellent for:

* smart traffic light labs
* digital twin dashboard
* smartphone control systems
* servo / pan-tilt labs

---

## 11. Difference from Station Mode

| Mode     | Purpose                     |
| -------- | --------------------------- |
| `STA`    | Connects to existing router |
| `AP`     | Creates its own Wi-Fi       |
| `AP+STA` | Both                        |

---

## 12. Best CPS Example

A great CPS example is:

```text id="ns63fr"
Smartphone → ESP32 AP → LED / Servo / Sensor
```

This allows direct physical control **without internet**.

---

## 13. Summary

ESP32 AP mode is ideal for:

* standalone lab experiments
* CPS demonstrations
* direct device control
* smart classroom prototypes
* digital twin interfaces

It is one of the best modes for **offline CPS and IoT education labs**.

---
