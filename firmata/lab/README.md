# ESP32 + ConfigurableFirmata + Node-RED Dashboard Lab

## Remote Hardware Control in a Cyber-Physical System

This lab demonstrates how to control an **ESP32 microcontroller** from
**Node-RED** using the **ConfigurableFirmata firmware**. Students will
build a simple **Cyber‑Physical System (CPS)** where a **Node‑RED
dashboard controls a physical LED connected to the ESP32**.

This experiment shows how software dashboards can directly control
hardware devices, enabling rapid prototyping for **IoT and CPS
applications**.

------------------------------------------------------------------------

# 1. Lab Objectives

By the end of this lab, students will be able to:

-   Install and configure **ConfigurableFirmata**
-   Connect **ESP32 to Node-RED**
-   Control **physical hardware from a web dashboard**
-   Understand **host-controlled microcontroller architecture**

------------------------------------------------------------------------

# 2. CPS Architecture

    Node-RED Dashboard
            │
            │ Serial Communication
            │
       Firmata Protocol
            │
       ESP32 (ConfigurableFirmata)
            │
          LED Actuator

------------------------------------------------------------------------

# 3. Required Hardware

  Component        Quantity
  ---------------- ----------
  ESP32 DevKit     1
  LED              1
  220 Ω resistor   1
  Breadboard       1
  USB cable        1

------------------------------------------------------------------------

# 4. Circuit Diagram

Connect the LED to **ESP32 GPIO2**.

    ESP32 GPIO2 ---- Resistor ---- LED ---- GND

Pin summary:

  ESP32 Pin   Function
  ----------- ------------
  GPIO2       LED Output
  GND         Ground

------------------------------------------------------------------------

# 5. Software Requirements

## 1. Arduino IDE

Download:

https://www.arduino.cc/en/software

------------------------------------------------------------------------

## 2. Firmata Library

Open Arduino IDE:

    Tools → Manage Libraries
    Search: Firmata
    Install: Firmata by Firmata Developers

------------------------------------------------------------------------

## 3. Node-RED

Install Node-RED:

    npm install -g node-red

Run Node‑RED:

    node-red

Open:

    http://localhost:1880

------------------------------------------------------------------------

## 4. Install Node-RED Arduino Nodes

    cd ~/.node-red
    npm install node-red-node-arduino

Restart Node‑RED.

------------------------------------------------------------------------

# 6. Upload ConfigurableFirmata to ESP32

Open Arduino IDE:

    File
    Examples
    Firmata
    ConfigurableFirmata

Select board:

    Tools → Board → ESP32 Dev Module

Select port:

    Tools → Port → COMx

Upload firmware.

The ESP32 will now wait for **Firmata commands from Node‑RED**.

------------------------------------------------------------------------

# 7. Create Node‑RED Flow

Open Node‑RED:

    http://localhost:1880

Add nodes:

    Inject → Arduino Out

Configure Arduino Out node:

  Parameter   Value
  ----------- ----------------
  Board       Arduino
  Port        COM3
  Pin         2
  Mode        Digital Output

Deploy the flow.

------------------------------------------------------------------------

# 8. Test the System

Click the **Inject button**.

Result:

    LED ON

Send LOW signal:

    LED OFF

The Node‑RED interface now controls the **physical ESP32 hardware**.

------------------------------------------------------------------------

# 9. Node‑RED Dashboard Version

Install dashboard:

    cd ~/.node-red
    npm install node-red-dashboard

Restart Node‑RED.

Add nodes:

    Dashboard Switch → Arduino Out

Flow:

    Dashboard Switch
            ↓
        Arduino Out

Open dashboard:

    http://localhost:1880/ui

Toggle switch to control the LED.

------------------------------------------------------------------------

# 10. Example Node‑RED Flow JSON

``` json
[
{
"id":"led_switch",
"type":"ui_switch",
"name":"LED Control",
"topic":"",
"group":"default",
"order":1,
"onvalue":"1",
"offvalue":"0"
},
{
"id":"arduino_out",
"type":"arduino out",
"pin":"2"
}
]
```

------------------------------------------------------------------------

# 11. CPS 5C Interpretation

  CPS Layer       Implementation
  --------------- --------------------------
  Connection      ESP32 hardware interface
  Conversion      Digital I/O signal
  Cyber           Node‑RED platform
  Cognition       User dashboard decisions
  Configuration   LED actuation

------------------------------------------------------------------------

# 12. Expected Output

  Action       Result
  ------------ ---------------
  Switch ON    LED lights
  Switch OFF   LED turns off

------------------------------------------------------------------------

# 13. Troubleshooting

### LED not working

Check:

-   Correct GPIO pin\
-   LED polarity\
-   Resistor connection

### Node‑RED cannot connect

Verify:

-   Correct COM port\
-   ESP32 firmware uploaded\
-   Node‑RED Arduino nodes installed

------------------------------------------------------------------------

# 14. Lab Extension

Students can extend this lab to include:

### Sensor monitoring

Examples:

-   LDR\
-   Temperature sensor\
-   MPU6050

### Actuator control

Examples:

-   Servo motor\
-   Relay module\
-   Buzzer

### CPS Digital Twin

Visualize device status on dashboard.

------------------------------------------------------------------------

# 15. Challenge Tasks

Try the following:

-   Add LED brightness control using PWM\
-   Display LED state on dashboard\
-   Add multiple LEDs\
-   Create automatic light control using LDR sensor

------------------------------------------------------------------------

# 16. Learning Outcome

Students will understand:

-   Hardware‑software interaction
-   Remote device control
-   CPS system architecture
-   Dashboard‑based actuation

------------------------------------------------------------------------

# 17. References

Firmata\
https://firmata.org

Node‑RED\
https://nodered.org

Arduino\
https://arduino.cc
