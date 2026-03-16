# Getting Started with ConfigurableFirmata

## Remote Hardware Control Using Arduino / ESP32

ConfigurableFirmata is a flexible firmware that allows a computer to control microcontroller hardware in real time using the **Firmata protocol**. Instead of writing new Arduino sketches for each project, external software such as **Node-RED, Python, MATLAB, or Processing** can directly control pins and sensors.

This approach is widely used in **IoT prototyping, robotics, and Cyber-Physical Systems (CPS) laboratories**.

---

# 1. What is Firmata?

Firmata is a **communication protocol** that allows software running on a host computer to control microcontroller hardware.

Instead of embedding all logic inside the microcontroller, the computer sends commands such as:

- Set pin mode  
- Turn LEDs on/off  
- Read sensor values  
- Control servos  
- Communicate with I2C devices  

The microcontroller becomes a **hardware interface device**.

---

# 2. What is ConfigurableFirmata?

ConfigurableFirmata is a **modular version of Firmata firmware** that allows developers to enable or disable hardware features at compile time.

This provides:

- smaller firmware size  
- improved efficiency  
- customizable hardware support  

Examples of optional modules include:

| Module | Function |
|------|------|
| Digital I/O | Control digital pins |
| Analog Input | Read sensors |
| PWM | LED brightness control |
| Servo | Servo motor control |
| I2C | Communication with sensors |
| Encoder | Rotary encoder support |
| Stepper | Stepper motor control |

---

# 3. System Architecture


+-----------------------+
| Host Computer |
| Node-RED / Python |
| MATLAB / Processing |
+----------+------------+
|
| Serial / USB / TCP
|
+----------v------------+
| Firmata Protocol |
+----------+------------+
|
+----------v------------+
| ConfigurableFirmata |
| Microcontroller |
| (Arduino / ESP32) |
+----------+------------+
|
+----------v------------+
| Sensors & Actuators |
| LED / Servo / LDR |
| MPU6050 / Buttons |
+-----------------------+


---

# 4. Supported Microcontrollers

ConfigurableFirmata works with many boards.

| Board | Supported |
|------|------|
| Arduino UNO | Yes |
| Arduino Mega | Yes |
| Arduino Nano | Yes |
| ESP32 | Yes |
| ESP8266 | Yes |

---

# 5. Required Software

## Arduino IDE

Download Arduino IDE:

https://www.arduino.cc/en/software

Install and open the IDE.

---

## Install Firmata Library

Open Arduino IDE and install the Firmata library.


Tools → Manage Libraries
Search: Firmata
Install: Firmata by Firmata Developers


---

# 6. Upload ConfigurableFirmata

Open the example firmware.


File
Examples
Firmata
ConfigurableFirmata


Select the board.


Tools → Board → Your board
Tools → Port → COMx


Upload the firmware.

After uploading, the microcontroller waits for **Firmata commands from the computer**.

---

# 7. Basic Pin Control Concept

Once ConfigurableFirmata is running, external software can control pins.

Typical commands include:

| Command | Function |
|------|------|
| pinMode | Configure pin |
| digitalWrite | Set HIGH / LOW |
| analogRead | Read analog sensor |
| servoWrite | Move servo |

---

# 8. Example Hardware Setup

## LED Control Example

Connect an LED to the microcontroller.


Arduino UNO

Pin 13 ---- Resistor ---- LED ---- GND


---

# 9. Example Using Node-RED

Install Node-RED Arduino nodes.


cd ~/.node-red
npm install node-red-node-arduino


Restart Node-RED.

### Example Flow


Inject Node
↓
Arduino Out Node


Configure Arduino Out node:

| Parameter | Value |
|------|------|
| Board | Arduino |
| Port | COM3 |
| Pin | 13 |
| Mode | Digital Output |

Press the inject button → LED turns on.

---

# 10. Example Using Python

Install PyFirmata.


pip install pyfirmata


Example code:

```python
from pyfirmata import Arduino
import time

board = Arduino('COM3')

led = board.get_pin('d:13:o')

while True:
    led.write(1)
    time.sleep(1)
    led.write(0)
    time.sleep(1)

This program blinks an LED connected to pin 13.

11. Enabling Optional Features

ConfigurableFirmata allows enabling specific modules.

Edit the configuration file:

ConfigurableFirmata.h

Example configuration:

#define ENABLE_DIGITAL
#define ENABLE_ANALOG
#define ENABLE_SERVO
#define ENABLE_I2C

Recompile and upload the firmware.

12. Example: Servo Motor Control
Hardware
Servo Motor

Red    → 5V
Brown  → GND
Orange → Pin 9
Python Example
servo = board.get_pin('d:9:s')

servo.write(90)

This moves the servo to 90°.

13. Typical Applications

ConfigurableFirmata is used in many applications:

Rapid Prototyping

Testing hardware without rewriting firmware.

Robotics

Remote control of motors and sensors.

Data Acquisition

Streaming sensor data to Python or MATLAB.

CPS and IoT Labs

Node-RED dashboards and digital twin systems.

14. Advantages
Feature	Benefit
Modular firmware	Customizable features
Easy integration	Works with many software tools
Rapid prototyping	No firmware rewriting
Real-time control	Direct hardware interaction
15. Limitations
Limitation	Description
Requires host computer	Cannot operate independently
Serial latency	Not suitable for strict real-time systems
Limited onboard logic	Most processing runs on the computer
16. Troubleshooting
Board not detected

Check the port:

Tools → Port
Node-RED connection error

Verify:

Correct COM port
Correct baud rate
Firmware uploaded successfully
Servo not responding

Check:

Power supply
Correct pin
Servo enabled in firmware
17. Suggested Learning Extensions

Try the following projects:

LED brightness control using PWM

LDR sensor monitoring

Servo motor dashboard control

MPU6050 motion visualization

IoT digital twin using Node-RED

18. References

Firmata Official Website
https://firmata.org

GitHub Repository
https://github.com/firmata

Arduino IDE
https://arduino.cc

Node-RED
https://nodered.org

License

This material is provided for educational and research use.
