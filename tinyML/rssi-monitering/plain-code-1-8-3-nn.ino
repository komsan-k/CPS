#include <WiFi.h>
#include <math.h>

// =====================================================
// WiFi credentials
// =====================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// =====================================================
// Neural Network: 1-8-3
// Example weights (replace with trained weights later)
// =====================================================

// Input -> Hidden weights (8x1)
float W1[8] = {
  2.1, -1.8, 3.0, -2.5,
  1.2, 2.8, -3.1, 1.7
};

// Hidden biases (8)
float b1[8] = {
  -0.8, 0.6, -1.2, 1.0,
  -0.2, -1.0, 1.4, -0.5
};

// Hidden -> Output weights (3x8)
float W2[3][8] = {
  { 1.2, -0.5,  1.8, -1.0,  0.6,  1.1, -1.3,  0.7 },  // Strong
  {-0.8,  1.5, -1.0,  1.2,  0.5, -0.7,  1.4, -0.6 },  // Moderate
  { 0.3, -0.9,  0.2, -0.4, -1.1,  0.4,  0.8,  1.3 }   // Weak
};

// Output biases (3)
float b2[3] = {0.2, -0.1, 0.0};

// =====================================================
// Utility functions
// =====================================================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float sigmoid(float z) {
  return 1.0f / (1.0f + expf(-z));
}

// Stable softmax for 3 outputs
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

// Normalize RSSI from [-100, -30] to [0, 1]
float normalizeRSSI(float rssi) {
  float x = (rssi + 100.0f) / 70.0f;
  return clamp01(x);
}

// =====================================================
// Forward pass: 1-8-3 network
// =====================================================
int predictRSSIClass(float x, float probs[3]) {
  float hidden[8];
  float logits[3];

  // Hidden layer
  for (int j = 0; j < 8; j++) {
    float z = W1[j] * x + b1[j];
    hidden[j] = sigmoid(z);
  }

  // Output layer
  for (int k = 0; k < 3; k++) {
    logits[k] = b2[k];
    for (int j = 0; j < 8; j++) {
      logits[k] += W2[k][j] * hidden[j];
    }
  }

  // Softmax
  softmax3(logits, probs);

  // Return predicted class
  return argmax3(probs);
}

const char* className(int classId) {
  switch (classId) {
    case 0: return "Strong";
    case 1: return "Moderate";
    case 2: return "Weak";
    default: return "Unknown";
  }
}

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

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
}

// =====================================================
// Loop
// =====================================================
void loop() {
  float rssi = WiFi.RSSI();          // e.g. -67 dBm
  float x = normalizeRSSI(rssi);     // 0..1 normalized

  float probs[3];
  int classId = predictRSSIClass(x, probs);

  Serial.println("====================================");
  Serial.print("RSSI (dBm): ");
  Serial.println(rssi);

  Serial.print("Normalized input: ");
  Serial.println(x, 4);

  Serial.print("P(Strong): ");
  Serial.println(probs[0], 4);

  Serial.print("P(Moderate): ");
  Serial.println(probs[1], 4);

  Serial.print("P(Weak): ");
  Serial.println(probs[2], 4);

  Serial.print("Predicted Class: ");
  Serial.println(className(classId));

  // Example decision logic
  if (classId == 0) {
    Serial.println("Action: Link is strong, keep normal operation.");
  } else if (classId == 1) {
    Serial.println("Action: Monitor signal closely.");
  } else if (classId == 2) {
    Serial.println("Action: Weak signal detected, consider reconnect or warning.");
  }

  delay(2000);
}
