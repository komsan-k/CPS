# Lab 2: Analog Sensor Interfacing and Data Acquisition  
## From Physical Signals to Clean Data  
### CPS Focus: C2 — Conversion

---

## 1. Lab Overview

This laboratory focuses on **analog sensors and data acquisition**, corresponding to **CPS Level C2: Conversion**.

Students will interface an **analog sensor** (e.g., LDR or potentiometer) with the **ESP32**, convert physical signals into digital values using the **ADC**, and log **clean datasets suitable for TinyML training**.

This lab marks the transition from **event-based sensing (digital)** to **continuous-valued sensing (analog)**, which is the dominant data type in most TinyML applications.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Interface analog sensors with ESP32 ADC pins  
- Understand ADC resolution, range, and quantization  
- Scale and normalize analog sensor values  
- Apply basic filtering to reduce noise  
- Log sensor data for TinyML dataset preparation  

---

## 3. Background Theory

### 3.1 Analog Sensors in CPS

Analog sensors output a **continuous voltage** proportional to a physical quantity.

| Sensor | Physical Quantity |
|------|-------------------|
| LDR | Light intensity |
| Potentiometer | Position |
| Thermistor | Temperature |
| Gas sensor | Gas concentration |

In CPS, analog sensing is essential for:
- Environmental monitoring  
- Human–machine interaction  
- Condition-based maintenance  

---

### 3.2 ADC on ESP32

The ESP32 features a **12-bit Analog-to-Digital Converter (ADC)**:

- ADC range: `0 – 4095`  
- Input voltage range: `0 – 3.3 V`  
- Ideal resolution: ≈ `0.8 mV / step`  

**Key Challenge:**  
Analog signals are inherently noisy, therefore **preprocessing is required before ML**.

---

## 4. CPS Perspective (C2: Conversion)

### CPS Mapping for Lab 2

| CPS Layer | Role |
|---------|------|
| Physical | Light / position / temperature |
| Conversion | ADC sampling |
| Cyber | Scaling and filtering |
| Decision | Threshold / future ML |
| Actuation | LED / indicator |

This lab emphasizes **accurate conversion and data quality**, not decision-making yet.

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- LDR (or potentiometer)  
- 10 kΩ resistor (voltage divider)  
- LED + resistor (optional)  
- Breadboard and jumper wires  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Serial Monitor / Serial Plotter  

---

## 6. Circuit Description

### Voltage Divider Setup (LDR Example)

```
3.3V ── LDR ──┬── ADC GPIO
               |
             10kΩ
               |
              GND
```

As light intensity changes, the voltage at the ADC pin changes proportionally.

---

## 7. System Architecture

```
[Analog Sensor]
       ↓
 Voltage Divider
       ↓
     ADC
       ↓
 Scaling & Filtering
       ↓
   Data Logging
```

This pipeline mirrors the **input stage of a TinyML system**.

---

## 8. Lab Tasks

### Task 1: Basic Analog Reading

```cpp
#define SENSOR_PIN 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int rawADC = analogRead(SENSOR_PIN);
  Serial.println(rawADC);
  delay(100);
}
```

Students should observe values between **0–4095**.

---

### Task 2: Voltage Scaling

Convert ADC values to voltage:

```cpp
float voltage = (rawADC / 4095.0) * 3.3;
```

This makes sensor data **physically interpretable**.

---

### Task 3: Noise Reduction (Moving Average)

```cpp
#define WINDOW 10
int buffer[WINDOW];
int index = 0;

float movingAverage(int newValue) {
  buffer[index++] = newValue;
  if (index >= WINDOW) index = 0;

  long sum = 0;
  for (int i = 0; i < WINDOW; i++)
    sum += buffer[i];

  return sum / (float)WINDOW;
}
```

Filtering significantly improves **data quality for ML**.

---

### Task 4: Dataset Logging

Output formatted data for dataset creation:

```cpp
Serial.print(millis());
Serial.print(",");
Serial.print(rawADC);
Serial.print(",");
Serial.println(voltage);
```

This produces **CSV-style logs** suitable for ML training.

---

## 9. Observations and Discussion

Students should analyze:
- Raw vs filtered signals  
- Effects of lighting or sensor variation  
- Sampling rate vs noise trade-offs  

**Important Insight:**  
> TinyML performance depends more on **good data** than on complex models.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Circuit | Correct analog wiring | 20% |
| ADC Reading | Stable sensor values | 20% |
| Scaling | Correct voltage conversion | 20% |
| Filtering | Noise visibly reduced | 20% |
| Logging | Proper dataset format | 20% |

---

## 11. Student Reflection Questions

- Why must analog data be filtered before ML?  
- How does ADC resolution affect ML accuracy?  
- What sampling issues could degrade a TinyML model?  

---

## 12. Preparation for Next Lab

In **Lab 3**, students will:
- Transform raw sensor data into **features**  
- Compute **mean, variance, min/max**  
- Build **feature vectors** for TinyML models  

---

## 13. Key Takeaway

**Lab 2 converts the physical world into data:**  
> Accurate ADC conversion and clean datasets are the foundation of every TinyML system.
