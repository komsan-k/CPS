// =====================================================
// ESP32 LDR Unsupervised Learning Deployment
// Without MQTT
// =====================================================

// ===============================
// Pins
// ===============================
const int LDR_PIN = 34;
const int LED_PIN = 2;

// ===============================
// Learned thresholds from Python
// Copy these from K-means / clustering result
// ===============================
float THRESHOLD1 = 0.275f;   // DARK / NORMAL boundary
float THRESHOLD2 = 0.655f;   // NORMAL / BRIGHT boundary

// ===============================
// ADC normalization
// ===============================
float ADC_MIN = 0.0f;
float ADC_MAX = 4095.0f;

// ===============================
// Moving average smoothing
// ===============================
const int WINDOW_SIZE = 10;
int samples[WINDOW_SIZE];
int sampleIndex = 0;
bool bufferFilled = false;

// ===============================
// State
// ===============================
String currentState = "UNKNOWN";
bool ledState = false;

// ===============================
// Helper functions
// ===============================
float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float getMovingAverage(int newValue) {
  samples[sampleIndex] = newValue;
  sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;

  if (sampleIndex == 0) {
    bufferFilled = true;
  }

  int count = bufferFilled ? WINDOW_SIZE : sampleIndex;
  if (count == 0) count = 1;

  long sum = 0;
  for (int i = 0; i < count; i++) {
    sum += samples[i];
  }

  return (float)sum / count;
}

String classifyLight(float normValue) {
  if (normValue < THRESHOLD1) {
    return "DARK";
  } 
  else if (normValue < THRESHOLD2) {
    return "NORMAL";
  } 
  else {
    return "BRIGHT";
  }
}

void setLED(bool state) {
  ledState = state;
  digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void applyControl(String state) {
  if (state == "DARK") {
    setLED(true);
  } 
  else {
    setLED(false);
  }
}

void printTelemetry(int adcRaw, float adcSmooth, float normValue, String state) {
  Serial.print("ADC Raw: ");
  Serial.print(adcRaw);

  Serial.print(" | Smooth: ");
  Serial.print(adcSmooth, 2);

  Serial.print(" | Norm: ");
  Serial.print(normValue, 4);

  Serial.print(" | State: ");
  Serial.print(state);

  Serial.print(" | LED: ");
  Serial.println(ledState ? "ON" : "OFF");
}

// ===============================
// Setup
// ===============================
void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  setLED(false);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    samples[i] = 0;
  }

  Serial.println("ESP32 LDR Unsupervised Learning Started");
  Serial.println("No Wi-Fi / No MQTT version");
}

// ===============================
// Main loop
// ===============================
void loop() {
  int adcRaw = analogRead(LDR_PIN);

  float adcSmooth = getMovingAverage(adcRaw);

  float normValue = clamp01((adcSmooth - ADC_MIN) / (ADC_MAX - ADC_MIN));

  String state = classifyLight(normValue);
  currentState = state;

  applyControl(state);

  printTelemetry(adcRaw, adcSmooth, normValue, state);

  delay(1000);
}
