# Lab Manual: Reinforcement Learning-Based Adaptive Light Control Using ESP32 and LDR

## 1. Objective
This lab demonstrates a simple **Q-learning controller** for adaptive LED brightness control using an **LDR sensor** and **ESP32**.

## 2. Learning Outcomes
After completing this lab, students will be able to:
- explain the concept of reinforcement learning
- implement a Q-learning algorithm on ESP32
- interface an LDR and PWM LED
- evaluate adaptive control performance

## 3. Hardware Required
- ESP32 DevKit
- LDR
- 10 kΩ resistor
- LED
- 220 Ω resistor
- breadboard and jumper wires

## 4. Circuit
```text
3.3V ---- LDR ----+---- GPIO34
                  |
                10kΩ
                  |
                 GND

GPIO2 ---- 220Ω ---- LED ---- GND
```

## 5. RL Model
### States
- 0 = Dark
- 1 = Dim
- 2 = Normal
- 3 = Bright

### Actions
- 0 = OFF
- 1 = LOW
- 2 = MEDIUM
- 3 = HIGH

### Reward
The controller aims to maintain **Normal brightness** while minimizing power.

## 6. Arduino Code
Use the provided Q-learning sketch from the previous message.

## 7. Experiment Procedure
1. Upload the code to ESP32
2. Open Serial Monitor at 115200 baud
3. Change ambient light conditions
4. Observe state, action, reward, and Q-table updates
5. Compare with threshold-based control

## 8. Questions
1. Why is epsilon-greedy exploration important?
2. How does reward affect learning?
3. Compare RL control vs fixed threshold control.
4. How can this be extended to smart lighting systems?

## 9. Expected Results
The Q-table should gradually converge so that:
- dark state -> high brightness
- bright state -> low brightness
- normal state -> minimum required power
