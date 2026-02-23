#include <math.h>

const int LDR_PIN = 36;

// --- ส่วนของ Weights และ Biases (วางค่าที่ได้จาก Python ตรงนี้) ---
float w1[8][1] = { { -1.520431 }, { 0.582103 }, { 1.391024 }, { -0.421901 }, { 0.772101 }, { -1.821042 }, { 0.992103 }, { 1.121045 } };
float b1[8] = {0.12, -0.05, 0.22, 0.01, 0.15, -0.18, 0.09, 0.11};
float w2[3][8] = { { 0.51, -0.22, 0.81, -0.11, 0.42, -0.61, 0.22, 0.12 }, { -0.11, 0.62, -0.21, 0.91, -0.32, 0.21, 0.52, -0.41 }, { -0.41, -0.52, -0.11, -0.22, 0.11, 0.92, -0.71, 0.82 } };
float b2[3] = {0.01, -0.02, 0.01};

void setup() {
  Serial.begin(115200);
}

void loop() {
  // อ่านค่าและปรับช่วงข้อมูล (Normalization)
  float input = analogRead(LDR_PIN) / 4095.0; 

  // เลเยอร์ที่ 1 (Hidden Layer) พร้อม ReLU
  float hidden[8];
  for (int i = 0; i < 8; i++) {
    float sum = (input * w1[i][0]) + b1[i];
    hidden[i] = (sum > 0) ? sum : 0; 
  }

  // เลเยอร์ที่ 2 (Output Layer) และ Softmax แบบย่อ
  float scores[3];
  float exp_sum = 0;
  for (int i = 0; i < 3; i++) {
    scores[i] = b2[i];
    for (int j = 0; j < 8; j++) {
      scores[i] += hidden[j] * w2[i][j];
    }
    exp_sum += exp(scores[i]);
  }

  // หาค่าความมั่นใจ (%) และคำตอบที่น่าจะเป็นที่สุด
  float max_conf = 0;
  int pred = 0;
  for (int i = 0; i < 3; i++) {
    float confidence = exp(scores[i]) / exp_sum;
    if (confidence > max_conf) {
      max_conf = confidence;
      pred = i;
    }
  }

  const char* labels[] = {"SUNNY", "OFFICE", "NIGHT"};
  Serial.printf("Status: %s (Confidence: %.2f%%)\n", labels[pred], max_conf * 100);
  
  delay(1000);
}
