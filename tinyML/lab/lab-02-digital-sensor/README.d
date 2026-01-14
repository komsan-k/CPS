# Lab 1: Digital Sensor Interfacing on ESP32  
## Reliable Sensor → Decision → Actuator  
### CPS Focus: C1 — Connection

---

## 1. Lab Overview

This laboratory introduces **digital sensor interfacing on the ESP32** and establishes the **physical connection layer (C1: Connection)** of a Cyber-Physical System (CPS).

Unlike **Lab 0**, which relied on simulated inputs, **Lab 1 works with real physical sensors** and emphasizes:
- Reliability  
- Debouncing  
- Signal stability  

Although no trained ML model is used yet, this lab is **critical for TinyML**, because **poor sensor data inevitably leads to poor ML performance**.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Interface digital sensors with ESP32 GPIO pins  
- Understand pull-up and pull-down resistor configurations  
- Implement debouncing for reliable sensor readings  
- Control actuators based on sensor-driven decisions  
- Explain why data quality is essential for TinyML systems  

---

## 3. Background Theory

### 3.1 Digital Sensors in CPS

A **digital sensor** outputs discrete logic values:

- `0` → LOW  
- `1` → HIGH  

**Examples:**
- Push button  
- PIR motion sensor  
- Tilt switch  
- Limit switch  

In CPS, digital sensors are commonly used for:
- Event detection  
- State-change detection  
- Trigger-based control  

---

### 3.2 Why Lab 1 Matters for TinyML

TinyML models assume that:
- Inputs are stable  
- Noise is controlled  
- Sampling is consistent  

However, real digital sensors often suffer from:
- Mechanical contact bounce  
- Electrical noise  
- False triggering  

If these issues are not solved **before ML**, the model will learn **incorrect or unstable patterns**.

---

## 4. CPS Perspective (C1: Connection)

### CPS Layer Mapping

| CPS Layer | Role in Lab 1 |
|---------|---------------|
| Physical | Button / PIR sensor |
| Cyber | GPIO reading + logic |
| Decision | Threshold / state logic |
| Actuation | LED / buzzer |
| Communication | Serial monitor |

**Lab 1 focuses entirely on physical–cyber connection reliability (C1).**

---

## 5. Required Hardware and Software

### 5.1 Hardware

- ESP32 development board  
- Push button (or PIR / tilt switch)  
- LED + 220 Ω resistor (if not using onboard LED)  
- Breadboard and jumper wires  

### 5.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Arduino Serial Monitor  

---

## 6. Circuit Description

### Recommended Configuration

- Button connected to a GPIO pin  
- **Internal pull-up resistor enabled**  
- Button pressed → `LOW`  
- Button released → `HIGH`  

This configuration improves **noise immunity** and simplifies wiring.

---

## 7. System Architecture

```
[Button / Digital Sensor]
           ↓
        GPIO Input
           ↓
     Debounce Logic
           ↓
     Decision Logic
           ↓
        LED Output
```

### TinyML Analogy

- Clean input → good features  
- Stable decision → reliable inference  

---

## 8. Lab Tasks

### Task 1: Basic Digital Input Reading

```cpp
#define BUTTON_PIN  15
#define LED_PIN     2

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  digitalWrite(LED_PIN, !buttonState); // active LOW

  Serial.println(buttonState);
  delay(100);
}
```

---

### Task 2: Understanding Contact Bounce

Students should observe:
- Multiple transitions from a single button press  
- Unstable serial output  
- LED flickering  

This demonstrates why **raw sensor data cannot be trusted directly**.

---

### Task 3: Software Debouncing

```cpp
unsigned long lastTime = 0;
int lastState = HIGH;

void loop() {
  int currentState = digitalRead(BUTTON_PIN);

  if (currentState != lastState) {
    lastTime = millis();
  }

  if ((millis() - lastTime) > 50) {
    digitalWrite(LED_PIN, !currentState);
  }

  lastState = currentState;
}
```

This ensures **stable decisions**, which are a **prerequisite for TinyML systems**.

---

## 9. Observations and Discussion

Students should analyze:
- Differences between raw and debounced signals  
- Impact of noise on decision logic  
- Importance of clean data for ML models  

**Key Insight:**  
> TinyML accuracy starts with reliable sensors, not complex models.

---

## 10. Evaluation Criteria

| Item | Description | Score |
|----|------------|------|
| Wiring | Correct sensor and LED connection | 20% |
| GPIO Logic | Correct input/output behavior | 20% |
| Debouncing | Stable sensor reading | 30% |
| Explanation | CPS & TinyML relevance | 20% |
| Report | Short reflection | 10% |

---

## 11. Student Reflection Questions

- Why is debouncing important for ML-based systems?  
- What happens if noisy data is used for training?  
- How does this lab relate to future TinyML inference?  

---

## 12. Preparation for Next Lab

In **Lab 2**, students will:
- Move from digital to **analog sensors**  
- Learn **ADC, scaling, and normalization**  
- Begin real dataset collection for TinyML  

---

## 13. Key Takeaway

**Lab 1 builds the foundation of TinyML:**  
> Reliable physical connections and clean digital signals are mandatory before intelligence can emerge.
