# Fundamentals of Reinforcement Learning

## 1. Introduction
Reinforcement Learning (RL) is a branch of machine learning in which an **agent learns an optimal decision-making strategy through interaction with an environment**.

The main objective is to **maximize cumulative long-term reward**.

---

## 2. Core Learning Principle
The interaction loop is:

State → Action → Reward → Next State

This cycle continues over time, allowing the agent to learn by trial and error.

---

## 3. Fundamental Components

### Agent
The learner or controller.

Examples:
- robot
- ESP32 controller
- traffic signal controller

### Environment
Everything external to the agent.

Examples:
- physical room lighting
- traffic system
- wireless network

### State
The current condition of the environment.

### Action
The decision taken by the agent.

### Reward
A numerical feedback signal indicating how good or bad an action is.

### Policy
A mapping from state to action.

---

## 4. Objective Function
The goal is to maximize cumulative reward:

G_t = sum(gamma^k * r_{t+k+1})

Where gamma is the discount factor.

---

## 5. Exploration vs Exploitation

### Exploration
Try new actions to discover better strategies.

### Exploitation
Use the best known action.

A common strategy is epsilon-greedy.

---

## 6. Value Function
V(s): expected long-term reward from state s.

## 7. Action-Value Function
Q(s,a): expected reward for taking action a in state s.

---

## 8. Bellman Equation
Q(s,a) = r + gamma * max Q(s',a')

---

## 9. Q-Learning Update Rule
Q(s,a) = Q(s,a) + alpha * [r + gamma * max Q(s',a') - Q(s,a)]

---

## 10. Example: LDR Adaptive Control
State: LDR brightness level  
Action: LED PWM level  
Reward: maintain target brightness with minimum power

r = -|L_target - L_actual| - lambda * P

---

## 11. Applications
- robotics
- CPS
- smart traffic
- smart lighting
- adaptive routing
- beam steering
