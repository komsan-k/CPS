/*
  === Traffic Twin: ESP32 Intersection Node (i1) ===
  - IR pulse counting via interrupt -> vehicles/min
  - Publishes flow + pulse events + status to MQTT
  - Subscribes to /cmd for signal timing + mode (AUTO/MANUAL)
  - Drives 3 LEDs (R/Y/G) as a mini traffic light

  Hardware:
    IR OUT -> GPIO27
    LED G  -> GPIO12
    LED Y  -> GPIO13
    LED R  -> GPIO14
*/

#include <WiFi.h>
#include <PubSubClient.h>

// -------------------- WiFi / MQTT --------------------
#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASSWORD"

#define MQTT_HOST   "192.168.1.10"   // change to your broker IP
#define MQTT_PORT   1883

// Topics
const char* TOPIC_FLOW   = "traffic/i1/flow_vpm";
const char* TOPIC_PULSE  = "traffic/i1/pulse";
const char* TOPIC_STATUS = "traffic/i1/status";
const char* TOPIC_CMD    = "traffic/i1/cmd";

// -------------------- Pins --------------------
const int IR_PIN = 27;
const int LED_G  = 12;
const int LED_Y  = 13;
const int LED_R  = 14;

// -------------------- Pulse Counting --------------------
volatile uint32_t pulseCount = 0;
volatile uint32_t lastIrMicros = 0;
const uint32_t DEBOUNCE_US = 5000; // 5ms debounce for IR pulses

void IRAM_ATTR onIrPulse() {
  uint32_t now = micros();
  if (now - lastIrMicros > DEBOUNCE_US) {
    pulseCount++;
    lastIrMicros = now;
  }
}

// -------------------- Traffic Light Timing --------------------
enum Phase { GREEN, YELLOW, RED };
Phase phase = RED;

unsigned long phaseStartMs = 0;

// Defaults (can be overridden by MQTT cmd)
uint32_t green_ms  = 10000;
uint32_t yellow_ms = 2000;
uint32_t red_ms    = 10000;

String mode = "AUTO"; // AUTO or MANUAL

// -------------------- Networking --------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastMinuteMs = 0;
unsigned long lastStatusMs = 0;
uint32_t lastPublishedFlow = 0;

void setLights(bool r, bool y, bool g) {
  digitalWrite(LED_R, r ? HIGH : LOW);
  digitalWrite(LED_Y, y ? HIGH : LOW);
  digitalWrite(LED_G, g ? HIGH : LOW);
}

void enterPhase(Phase p) {
  phase = p;
  phaseStartMs = millis();
  if (phase == GREEN)  setLights(false, false, true);
  if (phase == YELLOW) setLights(false, true,  false);
  if (phase == RED)    setLights(true,  false, false);
}

void applyLightFSM() {
  if (mode != "AUTO") return; // In MANUAL, you can implement direct light cmd later if needed

  unsigned long now = millis();
  unsigned long elapsed = now - phaseStartMs;

  switch (phase) {
    case GREEN:
      if (elapsed >= green_ms) enterPhase(YELLOW);
      break;
    case YELLOW:
      if (elapsed >= yellow_ms) enterPhase(RED);
      break;
    case RED:
      if (elapsed >= red_ms) enterPhase(GREEN);
      break;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  // Expect JSON like:
  // {"mode":"AUTO","green_ms":12000,"yellow_ms":2000,"red_ms":12000}
  // Lightweight parse (no ArduinoJson needed)
  if (String(topic) == TOPIC_CMD) {
    if (msg.indexOf("\"mode\"") >= 0) {
      if (msg.indexOf("AUTO") >= 0) mode = "AUTO";
      else if (msg.indexOf("MANUAL") >= 0) mode = "MANUAL";
    }

    auto readNumber = [&](const char* key, uint32_t& out) {
      String k = String("\"") + key + "\"";
      int idx = msg.indexOf(k);
      if (idx < 0) return;
      int colon = msg.indexOf(':', idx);
      if (colon < 0) return;
      int end1 = msg.indexOf(',', colon);
      int end2 = msg.indexOf('}', colon);
      int end = (end1 < 0) ? end2 : ((end2 < 0) ? end1 : min(end1, end2));
      if (end < 0) return;
      String num = msg.substring(colon + 1, end);
      num.trim();
      uint32_t val = (uint32_t)num.toInt();
      if (val > 0) out = val;
    };

    readNumber("green_ms",  green_ms);
    readNumber("yellow_ms", yellow_ms);
    readNumber("red_ms",    red_ms);
  }
}

void ensureMqtt() {
  while (!mqtt.connected()) {
    String cid = "esp32-traffic-i1-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(cid.c_str())) {
      mqtt.subscribe(TOPIC_CMD);
      // publish boot status
      mqtt.publish(TOPIC_STATUS, "{\"boot\":1,\"node\":\"i1\"}", true);
    } else {
      delay(1000);
    }
  }
}

void setup() {
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(IR_PIN), onIrPulse, FALLING);

  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  enterPhase(RED);
  lastMinuteMs = millis();
  lastStatusMs = millis();
}

void loop() {
  if (!mqtt.connected()) ensureMqtt();
  mqtt.loop();

  applyLightFSM();

  unsigned long now = millis();

  // Publish pulses quickly (optional)
  static uint32_t lastPulsePublished = 0;
  uint32_t pc = pulseCount;
  if (pc != lastPulsePublished) {
    lastPulsePublished = pc;
    mqtt.publish(TOPIC_PULSE, "1");
  }

  // Every 60s: publish vehicles/min (flow_vpm)
  if (now - lastMinuteMs >= 60000UL) {
    noInterrupts();
    uint32_t count = pulseCount;
    pulseCount = 0;
    interrupts();

    lastPublishedFlow = count; // vehicles per minute
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)lastPublishedFlow);
    mqtt.publish(TOPIC_FLOW, buf, true);

    lastMinuteMs = now;
  }

  // Heartbeat status every 10s
  if (now - lastStatusMs >= 10000UL) {
    String st = String("{\"node\":\"i1\",\"mode\":\"") + mode +
                String("\",\"phase\":") + (int)phase +
                String(",\"g\":") + green_ms +
                String(",\"y\":") + yellow_ms +
                String(",\"r\":") + red_ms +
                String(",\"rssi\":") + WiFi.RSSI() +
                String("}");
    mqtt.publish(TOPIC_STATUS, st.c_str(), false);
    lastStatusMs = now;
  }
}
