#include <WiFi.h>
#include <math.h>

// ======================================================
// WiFi credentials
// ======================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ======================================================
// 4-8-3 Neural Network Parameters
// Replace these example values with your trained weights
// ======================================================

// Input -> Hidden weights: 8 neurons, 4 inputs each
float W1[8][4] = {
  {  0.80, -0.40,  0.90, -0.20 },
  { -0.60,  0.70, -0.50,  0.30 },
  {  1.10, -0.20,  0.60, -0.40 },
  { -0.90,  0.80, -0.70,  0.50 },
  {  0.50,  0.20,  0.40, -0.30 },
  {  0.90, -0.60,  0.70, -0.50 },
  { -1.00,  0.90, -0.80,  0.60 },
  {  0.60, -0.10,  0.50, -0.20 }
};

// Hidden biases
float b1[8] = {
  -0.30, 0.20, -0.40, 0.50,
  -0.10, -0.20, 0.40, -0.30
};

// Hidden -> Output weights: 3 outputs, 8 hidden neurons
float W2[3][8] = {
  {  1.20, -0.50,  1.00, -0.80,  0.60,  0.90, -1.10,  0.70 }, // Strong
  { -0.70,  1.30, -0.90,  1.10,  0.40, -0.60,  1.20, -0.50 }, // Moderate
  {  0.20, -0.80,  0.30, -0.40, -1.00,  0.50,  0.90,  1.10 }  // Weak
};

// Output biases
float b2[3] = { 0.10, -0.10, 0.00 };

// ======================================================
// Signal feature settings
// ======================================================
const int AVG_WINDOW = 10;
float rssiBuffer[AVG_WINDOW];
int bufferIndex = 0;
bool bufferFilled = false;

float prevRSSI = -70.0f;
unsigned long prevTimeMs = 0;

// ======================================================
// Utility functions
// ======================================================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float sigmoid(float z) {
  return 1.0f / (1.0f + expf(-z));
}

void softmax3(float logits[3], float probs[3]) {
  float maxLogit = logits[0];
  if (logits[1] > maxLogit) maxLogit = logits[1];
  if (logits[2] > maxLogit) maxLogit = logits[2];

  float e0 = expf(logits[0] - maxLogit);
  float e1 = expf(logits[1] - maxLogit);
  float e2 = expf(logits[2] - maxLogit);

  float sum = e0 + e1 + e2;
  probs[0] = e0 / sum;
  probs[1] = e1 / sum;
  probs[2] = e2 / sum;
}

int argmax3(float probs[3]) {
  int idx = 0;
  if (probs[1] > probs[idx]) idx = 1;
  if (probs[2] > probs[idx]) idx = 2;
  return idx;
}

const char* className(int classId) {
  switch (classId) {
    case 0: return "Strong";
    case 1: return "Moderate";
    case 2: return "Weak";
    default: return "Unknown";
  }
}

// Normalize RSSI from [-100, -30] to [0, 1]
float normalizeRSSI(float rssi) {
  return clamp01((rssi + 100.0f) / 70.0f);
}

// Normalize slope roughly into [0,1]
// Here we assume slope range about [-20, +20] dBm/s
float normalizeSlope(float slope) {
  return clamp01((slope + 20.0f) / 40.0f);
}

// ======================================================
// Moving average buffer
// ======================================================
void addRSSIToBuffer(float rssi) {
  rssiBuffer[bufferIndex] = rssi;
  bufferIndex++;

  if (bufferIndex >= AVG_WINDOW) {
    bufferIndex = 0;
    bufferFilled = true;
  }
}

float getAverageRSSI() {
  int count = bufferFilled ? AVG_WINDOW : bufferIndex;
  if (count <= 0) return prevRSSI;

  float sum = 0.0f;
  for (int i = 0; i < count; i++) {
    sum += rssiBuffer[i];
  }
  return sum / count;
}

// ======================================================
// Forward pass for 4-8-3 network
// x[4] = {current, previous, average, slope}
// ======================================================
int predict4_8_3(float x[4], float probs[3]) {
  float hidden[8];
  float logits[3];

  for (int j = 0; j < 8; j++) {
    float z = b1[j];
    for (int i = 0; i < 4; i++) {
      z += W1[j][i] * x[i];
    }
    hidden[j] = sigmoid(z);
  }

  for (int k = 0; k < 3; k++) {
    logits[k] = b2[k];
    for (int j = 0; j < 8; j++) {
      logits[k] += W2[k][j] * hidden[j];
    }
  }

  softmax3(logits, probs);
  return argmax3(probs);
}

// ======================================================
// Setup
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < AVG_WINDOW; i++) {
    rssiBuffer[i] = -70.0f;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  prevRSSI = WiFi.RSSI();
  prevTimeMs = millis();
  addRSSIToBuffer(prevRSSI);
}

// ======================================================
// Loop
// ======================================================
void loop() {
  unsigned long nowMs = millis();
  float currentRSSI = WiFi.RSSI();

  addRSSIToBuffer(currentRSSI);
  float avgRSSI = getAverageRSSI();

  float dt = (nowMs - prevTimeMs) / 1000.0f;
  if (dt <= 0.0f) dt = 1.0f;

  float slope = (currentRSSI - prevRSSI) / dt;   // dBm per second

  // Prepare 4 normalized inputs
  float x[4];
  x[0] = normalizeRSSI(currentRSSI); // current
  x[1] = normalizeRSSI(prevRSSI);    // previous
  x[2] = normalizeRSSI(avgRSSI);     // moving average
  x[3] = normalizeSlope(slope);      // slope

  float probs[3];
  int classId = predict4_8_3(x, probs);

  Serial.println("========================================");
  Serial.print("Current RSSI: ");
  Serial.print(currentRSSI);
  Serial.println(" dBm");

  Serial.print("Previous RSSI: ");
  Serial.print(prevRSSI);
  Serial.println(" dBm");

  Serial.print("Average RSSI: ");
  Serial.print(avgRSSI);
  Serial.println(" dBm");

  Serial.print("Slope: ");
  Serial.print(slope);
  Serial.println(" dBm/s");

  Serial.println("---- Normalized Inputs ----");
  Serial.print("x1 current : "); Serial.println(x[0], 4);
  Serial.print("x2 previous: "); Serial.println(x[1], 4);
  Serial.print("x3 average : "); Serial.println(x[2], 4);
  Serial.print("x4 slope   : "); Serial.println(x[3], 4);

  Serial.println("---- Output Probabilities ----");
  Serial.print("P(Strong)  : "); Serial.println(probs[0], 4);
  Serial.print("P(Moderate): "); Serial.println(probs[1], 4);
  Serial.print("P(Weak)    : "); Serial.println(probs[2], 4);

  Serial.print("Predicted Class: ");
  Serial.println(className(classId));

  // Example decision logic
  if (classId == 0) {
    Serial.println("Action: Link strong, normal operation.");
  } else if (classId == 1) {
    Serial.println("Action: Monitor link quality.");
  } else {
    Serial.println("Action: Weak signal, consider reconnect / alert / AP switch.");
  }

  prevRSSI = currentRSSI;
  prevTimeMs = nowMs;

  delay(2000);
}
