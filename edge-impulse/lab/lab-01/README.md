# Lab 1 – Introduction to Cyber-Physical Systems and ESP32 Edge Devices

## 🎯 Objective

The objectives of this laboratory are to:

- Understand the fundamental concepts of **Cyber-Physical Systems (CPS)**
- Identify the role of **edge devices** in CPS architectures
- Become familiar with **ESP32 hardware** and its capabilities
- Set up the **ESP32 development environment**
- Create an initial project on the **Edge Impulse** platform
- Successfully connect and program an **ESP32 edge device**

---

## 🧠 Background Theory

### 1. Cyber-Physical Systems (CPS)

A **Cyber-Physical System (CPS)** is a system in which **physical processes** are tightly integrated with **computational and networking components**. Sensors measure physical phenomena, embedded processors analyze the data, and actuators affect the physical world through feedback loops.

Examples include smart factories, autonomous robots, wearable health systems, and intelligent transportation systems.

---

### 2. Physical–Cyber Interaction

CPS are defined by **bidirectional interaction**:
- **Physical → Cyber:** Sensors convert real-world signals into digital data.
- **Cyber → Physical:** Control algorithms generate actions that affect the physical system.

This interaction often operates under **real-time and safety constraints**.

---

### 3. Edge Devices in CPS

Edge devices:
- Interface directly with sensors and actuators
- Perform local computation
- Reduce latency and network dependency
- Improve privacy and reliability

---

### 4. ESP32 as a CPS Edge Device

The ESP32 microcontroller is well suited for CPS because it offers:
- Dual-core 32-bit processor
- Integrated Wi-Fi and Bluetooth
- Rich peripheral interfaces (ADC, I2C, SPI, UART, PWM)
- Low power consumption

---

### 5. Introduction to Edge Impulse

**Edge Impulse** is a platform for developing **embedded machine learning (TinyML)** applications. It provides tools for data acquisition, signal processing, model training, and deployment to edge devices such as ESP32.

In this lab, Edge Impulse is introduced at a **setup and conceptual level only**.

---

## 🧱 CPS Focus of This Lab

This lab emphasizes:
- Overall CPS system structure
- Physical–cyber data flow
- Role of the ESP32 within CPS

This corresponds mainly to the **Connection (C1)** layer of the 5C CPS model.

---

## ⚙️ Edge Impulse Focus

- Platform overview
- Account creation and login
- Project creation
- Device connection verification

---

## 🧰 Hardware Requirements

- ESP32 development board
- USB cable (USB-UART interface)
- Computer (Windows / macOS / Linux)

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- ESP32 Board Support Package
- Edge Impulse CLI
- Web browser (Edge Impulse Studio)

---

## 🔧 Experimental Procedure

### Step 1: ESP32 Development Environment Setup
1. Install Arduino IDE.
2. Add ESP32 board support using Boards Manager.
3. Connect ESP32 via USB.
4. Select correct board and port.

---

### Step 2: Basic ESP32 Programming Test
1. Open a simple example (Blink or Serial).
2. Upload to ESP32.
3. Verify successful execution.

---

### Step 3: Edge Impulse Project Creation
1. Create an Edge Impulse account.
2. Log in to Edge Impulse Studio.
3. Create a project named:
   ```
   CPS_Lab01_ESP32_Introduction
   ```

---

### Step 4: Install Edge Impulse CLI
1. Install Node.js.
2. Install Edge Impulse CLI.
3. Verify installation.

---

### Step 5: Connect ESP32 to Edge Impulse
1. Run Edge Impulse daemon.
2. Authenticate your account.
3. Confirm ESP32 is detected.
4. Assign device to project.

---

## 📊 Results and Observations

- ESP32 successfully programmed
- ESP32 detected by Edge Impulse
- Project correctly initialized

---

## 📝 Discussion Questions

1. What distinguishes CPS from traditional embedded systems?
2. Why are edge devices important in CPS?
3. What advantages does ESP32 provide?
4. Which CPS layers does Edge Impulse support?

---

## 🎓 Learning Outcomes

After this lab, students can:
- Explain CPS fundamentals
- Configure ESP32 hardware
- Set up Edge Impulse projects
- Prepare for CPS intelligence development

---

## 🧾 Conclusion

This lab established the foundation for CPS experimentation using **ESP32 edge devices** and **Edge Impulse**. Students are now ready to build intelligent CPS applications in subsequent labs.

---

## 📚 References

1. Edge Impulse Documentation  
2. ESP32 Technical Reference Manual  
3. Cyber-Physical Systems Literature  
4. TinyML and Edge AI Resources  

---

### ▶️ Next Lab

**Lab 2 – Sensor Interfacing and Data Acquisition (C1: Connection)**
