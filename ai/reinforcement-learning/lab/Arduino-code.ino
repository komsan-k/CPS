#include <Arduino.h>

// =============================
// Pin configuration
// =============================
const int LDR_PIN = 34;     // ADC pin
const int LED_PIN = 2;      // PWM LED pin

// =============================
// PWM configuration for ESP32
// =============================
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;   // 8-bit => 0..255

// =============================
// RL configuration
// =============================
const int NUM_STATES = 4;
const int NUM_ACTIONS = 4;

// Q-learning parameters
float alpha = 0.2;    // learning rate
float gamma = 0.8;    // discount factor
float epsilon = 0.2;  // exploration probability

// Q-table
float Q[NUM_STATES][NUM_ACTIONS];

// Action values mapped to PWM
int actionPWM[NUM_ACTIONS] = {0, 85, 170, 255};

// =============================
// Light thresholds
// Adjust for your own hardware
// ESP32 ADC range ~ 0..4095
// =============================
int th1 = 1000;
int th2 = 2000;
int th3 = 3000;

// Target state
// We want the room around "Normal"
const int TARGET_STATE = 2;

// =============================
// Helper functions
// =============================

// Convert raw ADC to state
int getState(int ldrValue) {
  if (ldrValue < th1) return 0;       // Dark
  else if (ldrValue < th2) return 1;  // Dim
  else if (ldrValue < th3) return 2;  // Normal
  else return 3;                      // Bright
}

// Reward design
float computeReward(int state, int action) {
  // Base reward for being near target brightness
  int error = abs(TARGET_STATE - state);

  // Energy penalty based on action brightness
  float energyPenalty = 0.0;
  if (action == 1) energyPenalty = 0.5;
  else if (action == 2) energyPenalty = 1.0;
  else if (action == 3) energyPenalty = 1.5;

  // Strong reward if target state is achieved
  if (state == TARGET_STATE) {
    return 10.0 - energyPenalty;
  }

  // Penalty grows with error
  return -3.0 * error - energyPenalty;
}

// Epsilon-greedy action selection
int chooseAction(int state) {
  float r = random(0, 1000) / 1000.0;

  if (r < epsilon) {
    // Explore
    return random(NUM_ACTIONS);
  } else {
    // Exploit
    int bestAction = 0;
    float bestQ = Q[state][0];
    for (int a = 1; a < NUM_ACTIONS; a++) {
      if (Q[state][a] > bestQ) {
        bestQ = Q[state][a];
        bestAction = a;
      }
    }
    return bestAction;
  }
}

// Find max Q for next state
float maxQ(int nextState) {
  float m = Q[nextState][0];
  for (int a = 1; a < NUM_ACTIONS; a++) {
    if (Q[nextState][a] > m) m = Q[nextState][a];
  }
  return m;
}

// Apply LED brightness
void applyAction(int action) {
  ledcWrite(PWM_CHANNEL, actionPWM[action]);
}

// Print Q-table
void printQTable() {
  Serial.println("Q-table:");
  for (int s = 0; s < NUM_STATES; s++) {
    Serial.print("State ");
    Serial.print(s);
    Serial.print(": ");
    for (int a = 0; a < NUM_ACTIONS; a++) {
      Serial.print(Q[s][a], 2);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  randomSeed(analogRead(35));

  // PWM init
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);

  // Initialize Q-table to zero
  for (int s = 0; s < NUM_STATES; s++) {
    for (int a = 0; a < NUM_ACTIONS; a++) {
      Q[s][a] = 0.0;
    }
  }

  Serial.println("ESP32 LDR + LED Q-Learning Demo");
}

void loop() {
  // Step 1: observe current state
  int ldrNow = analogRead(LDR_PIN);
  int state = getState(ldrNow);

  // Step 2: choose action
  int action = chooseAction(state);

  // Step 3: apply action
  applyAction(action);

  // Wait for environment to respond
  delay(500);

  // Step 4: observe next state
  int ldrNext = analogRead(LDR_PIN);
  int nextState = getState(ldrNext);

  // Step 5: compute reward
  float reward = computeReward(nextState, action);

  // Step 6: Q-learning update
  Q[state][action] =
      Q[state][action] +
      alpha * (reward + gamma * maxQ(nextState) - Q[state][action]);

  // Serial monitoring
  Serial.print("LDR(now): ");
  Serial.print(ldrNow);
  Serial.print("  State: ");
  Serial.print(state);

  Serial.print("  Action: ");
  Serial.print(action);
  Serial.print("  PWM: ");
  Serial.print(actionPWM[action]);

  Serial.print("  LDR(next): ");
  Serial.print(ldrNext);
  Serial.print("  NextState: ");
  Serial.print(nextState);

  Serial.print("  Reward: ");
  Serial.print(reward, 2);

  Serial.print("  Q[");
  Serial.print(state);
  Serial.print("][");
  Serial.print(action);
  Serial.print("] = ");
  Serial.println(Q[state][action], 2);

  static int counter = 0;
  counter++;
  if (counter % 10 == 0) {
    printQTable();
  }

  delay(1000);
}
