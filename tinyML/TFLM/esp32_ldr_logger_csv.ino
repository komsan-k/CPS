#include <Arduino.h>

static const int LDR_PIN = 36;          // ADC1_CH0
static const uint32_t SAMPLE_MS = 100;  // sampling interval

// Moving average window (set 1 to disable smoothing)
static const int MA_WINDOW = 8;

int ma_buf[MA_WINDOW];
int ma_idx = 0;
long ma_sum = 0;

String current_label = "dark";  // default label

static inline float clamp01(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

int read_adc_ma() {
  int raw = analogRead(LDR_PIN);

  // moving average
  ma_sum -= ma_buf[ma_idx];
  ma_buf[ma_idx] = raw;
  ma_sum += raw;
  ma_idx = (ma_idx + 1) % MA_WINDOW;

  return (int)(ma_sum / MA_WINDOW);
}

void print_help() {
  Serial.println();
  Serial.println("=== ESP32 LDR CSV Logger ===");
  Serial.println("Commands (type then press Enter):");
  Serial.println("  dark    -> set label to dark");
  Serial.println("  normal  -> set label to normal");
  Serial.println("  bright  -> set label to bright");
  Serial.println("  label?  -> print current label");
  Serial.println("  header  -> print CSV header");
  Serial.println("  help    -> show this help");
  Serial.println();
  Serial.println("CSV format:");
  Serial.println("timestamp_ms,adc,normalized,label");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  analogReadResolution(12); // 0..4095
  // optional: analogSetPinAttenuation(LDR_PIN, ADC_11db); // if your board supports this

  // init moving average buffer
  for (int i = 0; i < MA_WINDOW; i++) {
    ma_buf[i] = 0;
  }

  print_help();
  Serial.println("timestamp_ms,adc,normalized,label"); // CSV header
}

void handle_serial_commands() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "dark" || cmd == "normal" || cmd == "bright") {
    current_label = cmd;
    Serial.printf("# label set to: %s\n", current_label.c_str());
  } else if (cmd == "label?") {
    Serial.printf("# current label: %s\n", current_label.c_str());
  } else if (cmd == "header") {
    Serial.println("timestamp_ms,adc,normalized,label");
  } else if (cmd == "help") {
    print_help();
  } else if (cmd.length() > 0) {
    Serial.printf("# unknown command: %s (type help)\n", cmd.c_str());
  }
}

void loop() {
  static uint32_t last_ms = 0;

  handle_serial_commands();

  uint32_t now = millis();
  if (now - last_ms < SAMPLE_MS) return;
  last_ms = now;

  int adc = read_adc_ma();
  float x = clamp01((float)adc / 4095.0f);

  // CSV row
  Serial.printf("%lu,%d,%.6f,%s\n",
                (unsigned long)now, adc, x, current_label.c_str());
}
