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
3.3V ---- LDR ----+---- GPIO36
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

You are an agentic controller for an ESP32 lighting system.

Inputs:
- light_level: {{light_level}}
- adc: {{adc}}
- mode: {{mode}}
- led_state: {{led_state}}

Rules:
1. If mode is MANUAL, do not override.
2. If light_level is dark and mode is AUTO, recommend LED_ON.
3. If light_level is bright and mode is AUTO, recommend LED_OFF.
4. If light_level is medium, recommend HOLD unless visibility is poor.
5. Output only valid JSON.

Required JSON format:
{
  "reasoning": "...",
  "decision": "LED_ON" | "LED_OFF" | "HOLD"
}
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

## 13. Sample Node-RED Function for Guarding Output

```cpp
let decision = msg.payload.decision || "HOLD";

if (decision !== "LED_ON" && decision !== "LED_OFF" && decision !== "HOLD") {
    decision = "HOLD";
}

msg.payload = {
    target: "esp32_lab_01",
    action: decision
};

return msg;
```
## 14. Dashboard Design
### Recommended Dashboard Widgets

- **Gauge** for ADC value
- **Text** for light level
- **Text** for LED state
- **Text** for current mode
- **Chart** for ADC over time
- **Button** for `MANUAL` mode
- **Button** for `AUTO` mode
- **Button** for `LED ON`
- **Button** for `LED OFF`
- **Text box** for agent reasoning
- 
## 15. Lab Procedure
## Lab Procedure

### Part A: Hardware Setup
- Wire the LDR and LED to the ESP32
- Upload the Arduino code
- Confirm that the Serial Monitor works correctly

### Part B: MQTT Test
- Confirm that the ESP32 publishes telemetry
- Confirm that Node-RED subscribes correctly

### Part C: Dashboard
- Build the dashboard widgets
- Verify live sensor and system values

### Part D: Manual Control
- Send `LED_ON` and `LED_OFF` commands from the dashboard
- Confirm the physical LED output

### Part E: Rule-Based Control
- Implement threshold-based control in Node-RED
- Test by covering and uncovering the LDR

### Part F: Agentic AI Control
- Send telemetry context to the LLM
- Receive reasoning and decision
- Validate the output with the safety filter
- Publish the safe command to the ESP32

### Part G: Evaluation
- Compare rule-based and agentic behavior
- Observe delays, correctness, and stability
## Lab Procedure

### Part A: Hardware Setup
- Wire the LDR and LED to the ESP32
- Upload the Arduino code
- Confirm that the Serial Monitor works correctly

### Part B: MQTT Test
- Confirm that the ESP32 publishes telemetry
- Confirm that Node-RED subscribes correctly

### Part C: Dashboard
- Build the dashboard widgets
- Verify live sensor and system values

### Part D: Manual Control
- Send `LED_ON` and `LED_OFF` commands from the dashboard
- Confirm the physical LED output

### Part E: Rule-Based Control
- Implement threshold-based control in Node-RED
- Test by covering and uncovering the LDR

### Part F: Agentic AI Control
- Send telemetry context to the LLM
- Receive reasoning and decision
- Validate the output with the safety filter
- Publish the safe command to the ESP32

### Part G: Evaluation
- Compare rule-based and agentic behavior
- Observe delays, correctness, and stability

## 16. Example Experiments

## Example Experiments

### Scenario A: Dark Room
- **Condition:** Cover the LDR
- **Expected Agent Decision:** `LED_ON`

### Scenario B: Bright Room
- **Condition:** Shine a flashlight on the LDR
- **Expected Agent Decision:** `LED_OFF`

### Scenario C: Medium Light
- **Condition:** Partially cover the LDR
- **Expected Agent Decision:** `HOLD` or stable behavior

### Scenario D: Manual Override
- **Condition:** Set mode to `MANUAL`
- **Expected Agent Decision:** The agent should **not change** the LED state

## 17. Data Collection Table

| Time | ADC  | Light Level | Mode | Agent Decision | Final Action | Correct? |
| ---- | ---- | ----------- | ---- | -------------- | ------------ | -------- |
| t1   | 950  | dark        | AUTO | LED_ON         | LED_ON       | Yes      |
| t2   | 3200 | bright      | AUTO | LED_OFF        | LED_OFF      | Yes      |
| t3   | 2100 | medium      | AUTO | HOLD           | HOLD         | Yes      |

