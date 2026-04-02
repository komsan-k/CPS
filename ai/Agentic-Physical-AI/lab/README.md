# ESP32 + Agentic AI (Node-RED + LLM) + Physical Control Lab 

## 1. Overview

This lab demonstrates a complete **Cyber-Physical System (CPS)** integrating:

* **ESP32** for sensing and actuation
* **MQTT + Node-RED** for orchestration and dashboard
* **LLM-based Agentic AI** for reasoning and decision making
* **Physical control loop** using LED / optional servo

System loop:

**Sense → Analyze → Reason → Decide → Act**

---

## 2. Learning Objectives

* Understand CPS 5C architecture
* Build ESP32 sensor-to-actuator loop
* Use MQTT communication
* Develop Node-RED dashboard
* Integrate LLM for agentic reasoning
* Implement safe physical control

---

## 3. Hardware

* ESP32 DevKit
* LDR
* 10 kΩ resistor
* LED
* 220 Ω resistor
* Breadboard + jumper wires
* Optional push button / servo

---

## 4. Circuit

### LDR

```text
3.3V ---- LDR ----+---- GPIO34
                  |
                10kΩ
                  |
                 GND
```

### LED

```text
GPIO2 ---- 220Ω ---- LED ---- GND
```

---

## 5. MQTT Topics

### Telemetry

```json
{
  "device": "esp32_lab_01",
  "adc": 2100,
  "light_level": "medium",
  "led_state": "OFF",
  "mode": "AUTO"
}
```

Topic:

```text
cps/lab/telemetry
```

### Control

```json
{
  "action": "LED_ON"
}
```

Topic:

```text
cps/lab/cmd
```

---

## 6. ESP32 Arduino Code

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const int LDR_PIN = 34;
const int LED_PIN = 2;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int adc = analogRead(LDR_PIN);
  Serial.println(adc);
  delay(1000);
}
```

---

## 7. Node-RED Flow Logic

1. MQTT in → telemetry
2. JSON parse
3. Dashboard gauge + chart
4. LLM prompt function
5. HTTP request / LLM node
6. Safe command filter
7. MQTT out → ESP32

---

## 8. LLM Prompt

```text
You are an intelligent lighting controller.
If room is dark -> LED_ON
If room is bright -> LED_OFF
If medium -> HOLD
Return JSON only.
```

---

## 9. Safety Filter

Only allow:

* LED_ON
* LED_OFF
* HOLD

---

## 10. Experiments

### Scenario A: Dark

Expected: LED_ON

### Scenario B: Bright

Expected: LED_OFF

### Scenario C: Medium

Expected: HOLD

### Scenario D: Manual Override

Expected: Agent does not override

---

## 11. Report Template

* Objective
* Method
* Results
* Discussion
* Conclusion

---

## 12. CPS Mapping

* Connection → ESP32 + LDR
* Conversion → ADC processing
* Cyber → Node-RED dashboard
* Cognition → LLM agent
* Configuration → LED control
