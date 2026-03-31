# ESP32 AP + LDR Setup

## Access Point-Based Light Monitoring System Using an LDR Sensor

---

## 1. Concept

This is a simple **ESP32 AP + LDR setup**.

The idea is:

* ESP32 creates its own Wi-Fi network in **AP mode**
* LDR is connected to an **analog pin**
* a phone or laptop connects to the ESP32 hotspot
* the browser reads **live light data** from the ESP32

---

## 2. System Architecture

```text id="h8gn5d"
LDR Sensor → ESP32 → Wi-Fi AP → Phone / Laptop Browser
```

This is useful for:

* CPS lab demos without a router
* local sensor monitoring
* digital twin basics
* mobile sensor dashboards

---

## 3. Hardware Required

* ESP32 DevKit
* LDR
* 10 kΩ resistor
* breadboard
* jumper wires

---

## 4. Circuit Diagram

Use a **voltage divider**.

```text id="2gw87a"
3.3V ---- LDR ----+---- GPIO34 (ADC)
                  |
                10kΩ
                  |
                 GND
```

GPIO34 is a good analog input pin on ESP32.

---

## 5. Arduino Code

This code:

* starts ESP32 in AP mode
* hosts a webpage
* shows the LDR value
* auto-refreshes every 2 seconds

```cpp id="0d7oqj"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_LDR_AP";
const char* password = "12345678";

WebServer server(80);

const int LDR_PIN = 34;

String makePage(int ldrValue, float voltage) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 LDR Monitor</title></head><body>";
  html += "<h1>ESP32 AP + LDR</h1>";
  html += "<p><b>LDR ADC Value:</b> " + String(ldrValue) + "</p>";
  html += "<p><b>Voltage:</b> " + String(voltage, 2) + " V</p>";

  if (ldrValue < 1000) {
    html += "<p><b>Light Level:</b> Bright</p>";
  } else if (ldrValue < 2500) {
    html += "<p><b>Light Level:</b> Medium</p>";
  } else {
    html += "<p><b>Light Level:</b> Dark</p>";
  }

  html += "</body></html>";
  return html;
}

void handleRoot() {
  int ldrValue = analogRead(LDR_PIN);
  float voltage = (ldrValue / 4095.0) * 3.3;
  server.send(200, "text/html", makePage(ldrValue, voltage));
}

void handleData() {
  int ldrValue = analogRead(LDR_PIN);
  float voltage = (ldrValue / 4095.0) * 3.3;

  String json = "{";
  json += "\"adc\":" + String(ldrValue) + ",";
  json += "\"voltage\":" + String(voltage, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("ESP32 AP started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
```

---

## 6. How to Use

After uploading the code:

1. Open **Serial Monitor**
2. Find the AP IP (usually `192.168.4.1`)
3. Connect phone/laptop to Wi-Fi

**SSID:** `ESP32_LDR_AP`
**Password:** `12345678`

4. Open browser:

```text id="9vb70p"
http://192.168.4.1
```

---

## 7. Output

The webpage will show:

* ADC value from **0 to 4095**
* approximate voltage
* simple light classification

Example:

* Bright
* Medium
* Dark

---

## 8. Notes About LDR Behavior

Depending on how the divider is wired:

* ADC may increase in darkness
* or ADC may increase in brightness

If the behavior is reversed, swap:

* LDR position
* resistor position

---

## 9. Better Version with Live JSON

You can also access:

```text id="yjlwmr"
http://192.168.4.1/data
```

This returns:

```json id="9cq0n7"
{"adc":2134,"voltage":1.72}
```

This is useful for:

* Node-RED
* Python
* digital twin dashboard
* mobile app

---

## 10. CPS Interpretation

This setup maps well to CPS layers:

* **Connection:** LDR connected to ESP32
* **Conversion:** ADC converts light to digital value
* **Cyber:** ESP32 web server shares sensor data
* **Cognition:** browser/user interprets brightness
* **Configuration:** future control action such as LED

---

## 11. Optional Upgrade: AP + LDR + LED

You can extend this so that:

* when it gets dark, ESP32 turns on an LED
* webpage shows both LDR and LED state

Example logic:

```cpp id="24ty9k"
if (ldrValue > 2500) {
  digitalWrite(2, HIGH);   // dark -> LED on
} else {
  digitalWrite(2, LOW);
}
```

---

## 12. Suggested Lab Title

**ESP32 Access Point-Based Light Monitoring System Using an LDR Sensor**

---

## 13. Summary

This experiment is excellent for:

* CPS laboratory exercises
* IoT dashboard demonstrations
* digital twin introduction
* mobile sensing experiments
* standalone classroom demos

---
