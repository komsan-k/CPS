# Lab 2 – Sensor Interfacing and Data Acquisition (C1: Connection)

## 🎯 Objective

The objectives of this laboratory are to:

- Interface real-world sensors with an ESP32 edge device
- Understand the Connection (C1) layer of the 5C CPS model
- Acquire time-series sensor data from physical systems
- Stream sensor data to the Edge Impulse platform
- Create and label datasets suitable for CPS intelligence

---

## 🧠 Background Theory

### 1. Connection Layer in CPS (C1)

The Connection layer (C1) forms the foundation of a Cyber-Physical System. It links the physical world to the cyber domain through sensor-based data acquisition.

At this layer:
- Physical quantities are measured
- Signals are digitized
- Data is time-stamped and transmitted

Reliable data acquisition is critical for higher CPS layers such as Conversion, Cognition, and Configuration.

---

### 2. Sensors in Cyber-Physical Systems

Sensors provide awareness of physical phenomena. Common CPS sensors include:
- Motion sensors (accelerometers, gyroscopes)
- Environmental sensors (temperature, humidity, gas)
- Electrical sensors (current, voltage)
- Acoustic sensors (microphones)

---

### 3. ESP32 Sensor Interfaces

The ESP32 supports multiple sensor interfaces:
- Analog-to-Digital Converter (ADC)
- I2C digital interface
- SPI interface
- UART interface

This flexibility makes the ESP32 suitable for CPS sensing applications.

---

### 4. Data Acquisition for Edge Intelligence

Machine learning–enabled CPS requires sensor data that is:
- Representative of real operating conditions
- Properly sampled and segmented
- Correctly labeled

Edge Impulse provides structured tools for dataset creation and management.

---

## 🧱 CPS Focus of This Lab

This laboratory focuses on:
- Physical sensing
- Data acquisition
- Dataset integrity

**5C CPS Mapping:**  
- C1 – Connection: Primary focus  
- C2–C5: Not yet implemented

---

## ⚙️ Edge Impulse Focus

- Device connection and management
- Sensor data ingestion
- Dataset creation
- Data labeling and visualization

---

## 🧰 Hardware Requirements

- ESP32 development board
- At least one sensor (IMU, temperature, light, or vibration sensor)
- USB cable (USB-UART interface)

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- ESP32 Board Support Package
- Edge Impulse CLI
- Web browser (Edge Impulse Studio)

---

## 🔧 Experimental Setup

### Example: I2C Sensor Wiring

| Sensor Pin | ESP32 Pin |
|-----------|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

⚠️ Ensure voltage compatibility before connecting sensors.

---

## 🧪 Experimental Procedure

### Step 1: Sensor Interfacing
1. Power off the ESP32.
2. Connect the sensor according to the interface type.
3. Power on the ESP32.

---

### Step 2: Sensor Reading Test
1. Load a basic sensor example sketch.
2. Initialize the sensor interface.
3. Print readings via Serial Monitor.
4. Verify response to physical changes.

---

### Step 3: Edge Impulse Project Preparation
1. Open the Edge Impulse project from Lab 1.
2. Confirm ESP32 is connected.
3. Navigate to Data Acquisition.

---

### Step 4: Streaming Sensor Data
1. Run the Edge Impulse daemon.
2. Select the ESP32 device.
3. Stream live sensor data.
4. Observe waveform plots.

---

### Step 5: Dataset Creation and Labeling
1. Set sampling duration (e.g., 2–5 seconds).
2. Assign class labels (e.g., normal, motion).
3. Record multiple samples per class.
4. Verify dataset balance.

---

## 📊 Results and Observations

- Stable sensor readings obtained
- Sensor waveforms visible in Edge Impulse Studio
- Labeled dataset successfully created

---

## 📝 Discussion Questions

1. Why is reliable data acquisition essential in CPS?
2. How does sensor noise affect CPS intelligence?
3. How do you choose an appropriate sampling rate?
4. Which CPS layers depend directly on the Connection layer?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Interface sensors with ESP32
- Acquire and stream CPS sensor data
- Create labeled datasets using Edge Impulse
- Explain the importance of the C1 layer

---

## 🧾 Conclusion

This lab established the Connection layer of a CPS by interfacing sensors with an ESP32 and acquiring real-world data using Edge Impulse. The resulting dataset will be used in subsequent signal processing and machine learning labs.

---

## 📚 References

1. Edge Impulse Documentation  
2. ESP32 Technical Reference Manual  
3. Sensor Datasheets  
4. Cyber-Physical Systems Literature  

---

### ▶️ Next Lab

**Lab 3 – Signal Processing and Feature Extraction (C2: Conversion)**
