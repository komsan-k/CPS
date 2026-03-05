/*
  Smart Traffic Digital Twin — ESP32 (Single Intersection)
  Matches the Node-RED flow:
    - Publishes traffic sensor data to:  city/intersection1/traffic
    - Subscribes control commands from: city/intersection1/control
      Payload example: {"intersection":"intersection1","mode":"AUTO","phase":"NS_GREEN","green_time":45,"ts":...}

  Hardware (example):
    - Two vehicle sensors (digital): NS and EW
    - Two traffic-light sets: NS (R,Y,G) and EW (R,Y,G)

  Pins (edit as needed):
    NS: R=23, Y=19, G=18
    EW: R=5,  Y=17, G=16
    Sensors: NS=34, EW=35  (use pullups/pulldowns as required by your sensor)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ---------- WiFi + MQTT ----------
const char* WIFI_SSID     = "WiFi";
const char* WIFI_PASSWORD = "password";

const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_TRAFFIC  = "city/intersection1/traffic";
const char* TOPIC_CONTROL  = "city/intersection1/control";
const char* TOPIC_STATUS   = "city/intersection1/status";   // optional status publishing

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ---------- Pins ----------
const int NS_R = 23;
const int NS_Y = 19;
const int NS_G = 18;

const int EW_R = 5;
const int EW_Y = 17;
const int EW_G = 16;

const int SENSOR_NS = 34;   // digital input (if analog, you can change reading)
const int SENSOR_EW = 35;

// ---------- Timing (ms) ----------
const uint32_t PUBLISH_PERIOD_MS = 1000;
const uint32_t DEFAULT_GREEN_MS  = 30000;
const uint32_t MIN_GREEN_MS      = 10000;
const uint32_t MAX_GREEN_MS      = 60000;
const uint32_t YELLOW_MS         = 2000;
const uint32_t ALL_RED_MS        = 800;

// ---------- Control State ----------
enum Phase { PHASE_NS_GREEN, PHASE_EW_GREEN };

volatile Phase currentPhase = PHASE_NS_GREEN;
volatile uint32_t greenMsTarget = DEFAULT_GREEN_MS;

// traffic counts (simple pulse counter demo)
volatile uint32_t nsCount = 0;
volatile uint32_t ewCount = 0;

// ---------- Non-blocking FSM ----------
enum LightState { ST_NS_GREEN, ST_NS_YELLOW, ST_ALL_RED_1, ST_EW_GREEN, ST_EW_YELLOW, ST_ALL_RED_2 };
LightState st = ST_NS_GREEN;

uint32_t stStartMs = 0;
uint32_t lastPublishMs = 0;

// ---------- Helpers ----------
void setNS(bool r, bool y, bool g) {
  digitalWrite(NS_R, r);
  digitalWrite(NS_Y, y);
  digitalWrite(NS_G, g);
}
void setEW(bool r, bool y, bool g) {
  digitalWrite(EW_R, r);
  digitalWrite(EW_Y, y);
  digitalWrite(EW_G, g);
}

void applyStateOutputs(LightState s) {
  switch (s) {
    case ST_NS_GREEN:
      setNS(false, false, true);
      setEW(true,  false, false);
      break;
    case ST_NS_YELLOW:
      setNS(false, true,  false);
      setEW(true,  false, false);
      break;
    case ST_ALL_RED_1:
      setNS(true,  false, false);
      setEW(true,  false, false);
      break;
    case ST_EW_GREEN:
      setNS(true,  false, false);
      setEW(false, false, true);
      break;
    case ST_EW_YELLOW:
      setNS(true,  false, false);
      setEW(false, true,  false);
      break;
    case ST_ALL_RED_2:
      setNS(true,  false, false);
      setEW(true,  false, false);
      break;
  }
}

// clamp helper
uint32_t clampU32(uint32_t v, uint32_t lo, uint32_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// ---------- MQTT Callback ----------
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  if (String(topic) != TOPIC_CONTROL) return;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) return;

  const char* phaseStr = doc["phase"] | "NS_GREEN";
  int greenSec = doc["green_time"] | 30;

  uint32_t newGreenMs = clampU32((uint32_t)greenSec * 1000UL, MIN_GREEN_MS, MAX_GREEN_MS);

  Phase newPhase = PHASE_NS_GREEN;
  if (String(phaseStr) == "EW_GREEN") newPhase = PHASE_EW_GREEN;

  // Update targets atomically-ish (single-core operations OK here)
  greenMsTarget = newGreenMs;
  currentPhase = newPhase;

  // Optional: if requested phase differs from current FSM direction, we don't hard-jump immediately.
  // The FSM will naturally switch at the next decision point.
}

// ---------- WiFi / MQTT Connect ----------
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32-i1-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(TOPIC_CONTROL);
    } else {
      delay(800);
    }
  }
}

// ---------- Sensor Reading (simple) ----------
void readSensorsAndAccumulate() {
  // If your sensors output pulses, you should use interrupts.
  // Here we just read digital presence and accumulate counts as a simple demo.
  int ns = digitalRead(SENSOR_NS);
  int ew = digitalRead(SENSOR_EW);

  // Treat HIGH as "vehicle detected" for this example:
  if (ns == HIGH) nsCount++;
  if (ew == HIGH) ewCount++;
}

// ---------- Publish Traffic ----------
void publishTraffic() {
  StaticJsonDocument<128> doc;
  doc["NS"] = (uint32_t)nsCount; // counts per publish window (if you reset)
  doc["EW"] = (uint32_t)ewCount;

  char out[128];
  size_t n = serializeJson(doc, out, sizeof(out));
  mqtt.publish(TOPIC_TRAFFIC, out, n);

  // reset window counts to represent vehicles per second window
  nsCount = 0;
  ewCount = 0;

  // Optional status publishing
  StaticJsonDocument<160> stDoc;
  stDoc["phase_fsm"] = (st == ST_NS_GREEN || st == ST_NS_YELLOW || st == ST_ALL_RED_1) ? "NS" : "EW";
  stDoc["green_ms_target"] = greenMsTarget;
  char stOut[180];
  size_t m = serializeJson(stDoc, stOut, sizeof(stOut));
  mqtt.publish(TOPIC_STATUS, stOut, m);
}

// ---------- Control Logic (phase selection at decision points) ----------
Phase chooseNextGreenPhase() {
  // Minimal policy:
  // - Use requested phase from Node-RED (currentPhase variable) as the target
  // In a more advanced version, you could combine target with local sensor dominance.
  return currentPhase;
}

// ---------- FSM Update ----------
void fsmTick() {
  uint32_t now = millis();
  uint32_t elapsed = now - stStartMs;

  switch (st) {
    case ST_NS_GREEN:
      // Stay green for greenMsTarget, then go yellow
      if (elapsed >= greenMsTarget) {
        st = ST_NS_YELLOW;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;

    case ST_NS_YELLOW:
      if (elapsed >= YELLOW_MS) {
        st = ST_ALL_RED_1;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;

    case ST_ALL_RED_1:
      if (elapsed >= ALL_RED_MS) {
        // Decide next green phase
        Phase next = chooseNextGreenPhase();
        st = (next == PHASE_EW_GREEN) ? ST_EW_GREEN : ST_NS_GREEN;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;

    case ST_EW_GREEN:
      if (elapsed >= greenMsTarget) {
        st = ST_EW_YELLOW;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;

    case ST_EW_YELLOW:
      if (elapsed >= YELLOW_MS) {
        st = ST_ALL_RED_2;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;

    case ST_ALL_RED_2:
      if (elapsed >= ALL_RED_MS) {
        Phase next = chooseNextGreenPhase();
        st = (next == PHASE_NS_GREEN) ? ST_NS_GREEN : ST_EW_GREEN;
        stStartMs = now;
        applyStateOutputs(st);
      }
      break;
  }
}

void setup() {
  // IO
  pinMode(NS_R, OUTPUT); pinMode(NS_Y, OUTPUT); pinMode(NS_G, OUTPUT);
  pinMode(EW_R, OUTPUT); pinMode(EW_Y, OUTPUT); pinMode(EW_G, OUTPUT);

  // Sensors (set pullups/pulldowns as needed)
  pinMode(SENSOR_NS, INPUT);
  pinMode(SENSOR_EW, INPUT);

  // Default state
  st = ST_NS_GREEN;
  stStartMs = millis();
  applyStateOutputs(st);

  Serial.begin(115200);
  connectWiFi();
  connectMQTT();

  lastPublishMs = millis();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  // Read sensors & update counts
  readSensorsAndAccumulate();

  // Update light FSM
  fsmTick();

  // Periodic publish
  uint32_t now = millis();
  if (now - lastPublishMs >= PUBLISH_PERIOD_MS) {
    lastPublishMs = now;
    publishTraffic();
  }

  delay(5); // small yield
}
