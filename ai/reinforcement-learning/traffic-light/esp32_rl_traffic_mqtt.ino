#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// =====================================================
// ESP32 Q-Learning Traffic Light Controller + MQTT
// Publishes JSON for Node-RED digital twin
// Topic: traffic/rl/state
// =====================================================

#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASSWORD"

#define MQTT_HOST   "192.168.1.10"
#define MQTT_PORT   1883

const char* TOPIC_STATE = "traffic/rl/state";

// East-West lights
const int EW_RED    = 16;
const int EW_YELLOW = 17;
const int EW_GREEN  = 18;

// North-South lights
const int NS_RED    = 19;
const int NS_YELLOW = 21;
const int NS_GREEN  = 22;

// Sensor pins
const int SENSOR_EW = 34;
const int SENSOR_NS = 35;

const int NUM_STATES = 9;
const int NUM_ACTIONS = 2;

float Q[NUM_STATES][NUM_ACTIONS];

float alpha = 0.2f;
float gamma = 0.8f;
float epsilon = 0.2f;

const int GREEN_TIME = 5000;
const int YELLOW_TIME = 2000;

int th_low = 1200;
int th_high = 2800;

int currentAction = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

int getQueueLevel(int sensorValue) {
  if (sensorValue < th_low) return 0;
  else if (sensorValue < th_high) return 1;
  else return 2;
}

int getStateIndex(int qEW, int qNS) {
  return qEW * 3 + qNS;
}

int chooseAction(int state) {
  float r = random(0, 1000) / 1000.0f;
  if (r < epsilon) return random(NUM_ACTIONS);

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

float maxQ(int nextState) {
  float m = Q[nextState][0];
  for (int a = 1; a < NUM_ACTIONS; a++) {
    if (Q[nextState][a] > m) m = Q[nextState][a];
  }
  return m;
}

float computeReward(int qEW, int qNS, int action) {
  float reward = -(qEW + qNS);

  if (qEW > qNS && action == 0) reward += 3.0f;
  if (qNS > qEW && action == 1) reward += 3.0f;

  if (qEW > qNS && action == 1) reward -= 3.0f;
  if (qNS > qEW && action == 0) reward -= 3.0f;

  if (qEW == qNS) reward += 1.0f;

  return reward;
}

void allRed() {
  digitalWrite(EW_RED, HIGH);
  digitalWrite(EW_YELLOW, LOW);
  digitalWrite(EW_GREEN, LOW);

  digitalWrite(NS_RED, HIGH);
  digitalWrite(NS_YELLOW, LOW);
  digitalWrite(NS_GREEN, LOW);
}

void setEWGreen() {
  digitalWrite(EW_RED, LOW);
  digitalWrite(EW_YELLOW, LOW);
  digitalWrite(EW_GREEN, HIGH);

  digitalWrite(NS_RED, HIGH);
  digitalWrite(NS_YELLOW, LOW);
  digitalWrite(NS_GREEN, LOW);
}

void setNSGreen() {
  digitalWrite(EW_RED, HIGH);
  digitalWrite(EW_YELLOW, LOW);
  digitalWrite(EW_GREEN, LOW);

  digitalWrite(NS_RED, LOW);
  digitalWrite(NS_YELLOW, LOW);
  digitalWrite(NS_GREEN, HIGH);
}

void transitionToEW() {
  if (currentAction == 1) {
    digitalWrite(NS_GREEN, LOW);
    digitalWrite(NS_YELLOW, HIGH);
    delay(YELLOW_TIME);
    digitalWrite(NS_YELLOW, LOW);
  }
  setEWGreen();
  currentAction = 0;
}

void transitionToNS() {
  if (currentAction == 0) {
    digitalWrite(EW_GREEN, LOW);
    digitalWrite(EW_YELLOW, HIGH);
    delay(YELLOW_TIME);
    digitalWrite(EW_YELLOW, LOW);
  }
  setNSGreen();
  currentAction = 1;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wi-Fi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32-RL-Traffic-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 2 seconds");
      delay(2000);
    }
  }
}

void publishState(int rawEW, int rawNS, int qEW, int qNS, int state, int action, float reward, float updatedQ) {
  char payload[256];
  snprintf(
    payload, sizeof(payload),
    "{\"rawEW\":%d,\"rawNS\":%d,\"qEW\":%d,\"qNS\":%d,\"state\":%d,\"action\":%d,\"reward\":%.2f,\"updatedQ\":%.2f}",
    rawEW, rawNS, qEW, qNS, state, action, reward, updatedQ
  );

  mqttClient.publish(TOPIC_STATE, payload, true);
  Serial.print("Published: ");
  Serial.println(payload);
}

void printQTable() {
  Serial.println("\nQ-table:");
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

  randomSeed(analogRead(32));

  pinMode(EW_RED, OUTPUT);
  pinMode(EW_YELLOW, OUTPUT);
  pinMode(EW_GREEN, OUTPUT);

  pinMode(NS_RED, OUTPUT);
  pinMode(NS_YELLOW, OUTPUT);
  pinMode(NS_GREEN, OUTPUT);

  pinMode(SENSOR_EW, INPUT);
  pinMode(SENSOR_NS, INPUT);

  allRed();
  delay(1000);
  setEWGreen();

  for (int s = 0; s < NUM_STATES; s++) {
    for (int a = 0; a < NUM_ACTIONS; a++) {
      Q[s][a] = 0.0f;
    }
  }

  connectWiFi();
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println("ESP32 RL Traffic Light Controller + MQTT");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  int rawEW = analogRead(SENSOR_EW);
  int rawNS = analogRead(SENSOR_NS);

  int qEW = getQueueLevel(rawEW);
  int qNS = getQueueLevel(rawNS);

  int state = getStateIndex(qEW, qNS);

  int action = chooseAction(state);

  if (action == 0) {
    transitionToEW();
  } else {
    transitionToNS();
  }

  delay(GREEN_TIME);

  int rawEW2 = analogRead(SENSOR_EW);
  int rawNS2 = analogRead(SENSOR_NS);

  int qEW2 = getQueueLevel(rawEW2);
  int qNS2 = getQueueLevel(rawNS2);

  int nextState = getStateIndex(qEW2, qNS2);

  float reward = computeReward(qEW2, qNS2, action);

  Q[state][action] = Q[state][action] +
                     alpha * (reward + gamma * maxQ(nextState) - Q[state][action]);

  float updatedQ = Q[state][action];

  Serial.print("RawEW=");
  Serial.print(rawEW);
  Serial.print(" qEW=");
  Serial.print(qEW);
  Serial.print(" | RawNS=");
  Serial.print(rawNS);
  Serial.print(" qNS=");
  Serial.print(qNS);
  Serial.print(" | State=");
  Serial.print(state);
  Serial.print(" | Action=");
  Serial.print(action == 0 ? "EW_GREEN" : "NS_GREEN");
  Serial.print(" | NextState=");
  Serial.print(nextState);
  Serial.print(" | Reward=");
  Serial.print(reward, 2);
  Serial.print(" | UpdatedQ=");
  Serial.println(updatedQ, 2);

  publishState(rawEW, rawNS, qEW, qNS, state, action, reward, updatedQ);

  static int counter = 0;
  counter++;
  if (counter % 5 == 0) {
    printQTable();
  }
}
