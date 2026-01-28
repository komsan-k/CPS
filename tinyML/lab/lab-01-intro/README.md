# Lab 0: Introduction to TinyML and ESP32  
## From Sensor to Decision on a Microcontroller

---

## 1. Lab Overview

This laboratory introduces students to **TinyML**, focusing on how **machine learning–like inference can run directly on an ESP32 microcontroller** without any cloud connectivity.

Lab 0 is intentionally **simple and conceptual**, serving as the foundation for all subsequent TinyML laboratories.

Students will implement a **basic decision model (threshold-based classifier)** that mimics ML behavior and observe how sensor data (simulated) leads to physical actions via an actuator.

---

## 2. Learning Objectives

After completing this lab, students will be able to:

- Explain the concept of **TinyML** and its constraints  
- Describe the **Sensor → Decision → Actuator** CPS loop  
- Program an **ESP32 using the Arduino environment**  
- Implement a **simple on-device decision function**  
- Measure basic system behavior (latency and memory awareness)

---

## 3. Background Theory

### 3.1 What is TinyML?

**TinyML** is the deployment of machine learning inference on **resource-constrained microcontrollers**, such as ESP32, enabling:

- Real-time decision-making  
- Ultra-low power operation  
- Offline intelligence  
- Privacy-preserving computation  

In TinyML systems, models are typically:
- Small and lightweight  
- Quantized  
- Compiled into **C/C++ code** that runs directly on the device  

---

### 3.2 Sensor → Decision → Actuator (CPS Loop)

A TinyML system follows the **Cyber-Physical System (CPS) loop**:

1. **Sensor** – captures physical data  
2. **Decision** – processes data and performs inference  
3. **Actuator** – responds physically to the decision  

In this lab, the “decision” is a **simple threshold classifier**, preparing students for real ML models in later labs.

---

## 4. Required Hardware and Software

### 4.1 Hardware

- ESP32 development board  
- USB cable  
- Onboard LED (or external LED with resistor)

### 4.2 Software

- Arduino IDE  
- ESP32 Board Support Package  
- Arduino Serial Monitor

---

## 5. System Architecture

### Logical Flow

```
Input Value → Decision Function → Digital Output (LED)
```

### CPS / ML Analogy

| CPS Element | TinyML Analogy |
|------------|---------------|
| Input data | Sensor feature |
| Decision logic | ML inference |
| Output action | Prediction-driven actuation |

---

## 6. Lab Task Description

### Task 1: ESP32 Environment Verification

1. Connect the ESP32 to the computer  
2. Upload a simple **Blink** example  
3. Verify correct board selection and COM port  

---

### Task 2: Implement a Decision Function

Instead of a trained ML model, a **threshold-based function** is used:

```cpp
int tinyDecision(int x) {
  if (x > 50)
    return 1;   // ON
  else
    return 0;   // OFF
}
```

This function represents:
- A **binary classifier**
- A **placeholder for future TinyML inference**

---

### Task 3: Actuator Control

The output of the decision function controls an LED:

```cpp
void loop() {
  int inputValue = random(0, 100);   // Simulated sensor
  int decision = tinyDecision(inputValue);

  digitalWrite(LED_BUILTIN, decision ? HIGH : LOW);

  Serial.print("Input: ");
  Serial.print(inputValue);
  Serial.print(" | Decision: ");
  Serial.println(decision);

  delay(500);
}
```

---

## 7. Observations and Discussion

Students should observe that:

- Decisions occur **locally on the ESP32**
- No cloud or internet connection is required  
- The system responds within **milliseconds**  
- Logic-based decisions closely resemble ML inference structure  

This demonstrates the core idea of **TinyML**:

> **Intelligence at the edge**

---

## 8. Evaluation Criteria

| Item | Description | Score |
|----|-----------|------|
| Setup | ESP32 programmed successfully | 20% |
| Logic | Correct decision behavior | 30% |
| Output | LED reacts correctly | 20% |
| Explanation | CPS loop explained | 20% |
| Report | Short reflection (½ page) | 10% |

---

## 9. Learning Reflection (Student Report)

Students should answer the following questions:

- How is this decision function similar to ML inference?  
- Why is running intelligence on-device useful?  
- What limitations might ESP32 have compared to cloud AI?  

---

## 10. Preparation for Next Lab

In **Lab 1**, students will:

- Replace simulated input with **real sensors**  
- Learn **reliable GPIO and data acquisition**  
- Prepare datasets for future ML tasks  

---

## 11. Key Takeaway

**Lab 0 establishes the mindset of TinyML:**

> **Intelligence does not need the cloud — it can live directly inside tiny devices.**
