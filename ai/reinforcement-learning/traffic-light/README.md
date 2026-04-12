# README: Reinforcement Learning for Traffic Light Control using ESP32

## 1. Objective
This lab demonstrates a **Q-learning based adaptive traffic light controller** for a two-road intersection using **ESP32**.

## 2. Learning Outcomes
Students will be able to:
- explain reinforcement learning for control systems
- implement Q-learning on ESP32
- interface LEDs as traffic lights
- analyze queue-based decision logic
- compare adaptive control with fixed-time control

## 3. Hardware Required
- ESP32 DevKit
- 6 LEDs (2 x Red, 2 x Yellow, 2 x Green)
- 6 × 220 Ω resistors
- 2 potentiometers or IR sensors
- breadboard and jumper wires

## 4. GPIO Mapping
### East-West
- GPIO16 → Red
- GPIO17 → Yellow
- GPIO18 → Green

### North-South
- GPIO19 → Red
- GPIO21 → Yellow
- GPIO22 → Green

### Sensors
- GPIO34 → EW traffic input
- GPIO35 → NS traffic input

## 5. RL Model
### States
Queue levels:
- 0 = Low
- 1 = Medium
- 2 = High

Combined state:
state = qEW * 3 + qNS

Total states = 9

### Actions
- 0 = EW green
- 1 = NS green

### Reward
Reward favors serving the heavier queue while penalizing congestion.

## 6. Experiment Procedure
1. Connect LEDs and sensors
2. Upload Arduino code
3. Open Serial Monitor at 115200 baud
4. Adjust potentiometers to simulate traffic load
5. Observe learned actions
6. Record Q-table evolution

## 7. Questions
1. Why does the Q-table converge?
2. How does epsilon affect exploration?
3. Compare with fixed-time traffic light control
4. How would you add pedestrian crossing?

## 8. Expected Results
The controller should gradually prefer:
- EW green when EW queue > NS
- NS green when NS queue > EW
- balanced switching when both queues are similar
