/*
  Transportation 5.0 CPS Lab: Smart Intersection (ESP32)
  - Publishes telemetry to MQTT: t50/intersection/telemetry
  - Subscribes commands from MQTT: t50/intersection/cmd
  - Executes deterministic phases: A_GREEN, A_YELLOW, B_GREEN, B_YELLOW, PED_CROSS, ALL_RED

  Libraries:
    - PubSubClient
    - ArduinoJson
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ====== USER CONFIG ======
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST = "192.168.1.10"; // <-- set to your broker IP
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_TELE = "t50/intersection/telemetry";
const char* TOPIC_CMD  = "t50/intersection/cmd";

const char* DEVICE_ID = "esp32-intersection-01";

// ====== PINS ======
const int PIN_LDR = 34;          // ADC
const int PIN_PED_BTN = 27;      // to GND, INPUT_PULLUP

// Lane A LEDs
const int A_R = 25;
const int A_Y = 26;
const int A_G = 33;

// Lane B LEDs
const int B_R = 14;
const int B_Y = 12;
const int B_G = 13;

// Ped LED (optional)
const int PED_LED = 32;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

String currentPhase = "ALL_RED";
unsigned long phaseEndsAt = 0;

static void setAllOff() {
  digitalWrite(A_R, LOW); digitalWrite(A_Y, LOW); digitalWrite(A_G, LOW);
  digitalWrite(B_R, LOW); digitalWrite(B_Y, LOW); digitalWrite(B_G, LOW);
  digitalWrite(PED_LED, LOW);
}

static void applyPhase(const String& phase) {
  setAllOff();

  if (phase == "A_GREEN") {
    digitalWrite(A_G, HIGH);
    digitalWrite(B_R, HIGH);
  } else if (phase == "A_YELLOW") {
    digitalWrite(A_Y, HIGH);
    digitalWrite(B_R, HIGH);
  } else if (phase == "B_GREEN") {
    digitalWrite(B_G, HIGH);
    digitalWrite(A_R, HIGH);
  } else if (phase == "B_YELLOW") {
    digitalWrite(B_Y, HIGH);
    digitalWrite(A_R, HIGH);
  } else if (phase == "PED_CROSS") {
    digitalWrite(A_R, HIGH);
    digitalWrite(B_R, HIGH);
    digitalWrite(PED_LED, HIGH);
  } else { // ALL_RED or unknown
    digitalWrite(A_R, HIGH);
    digitalWrite(B_R, HIGH);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.print("JSON parse failed: ");
    Serial.println(err.c_str());
    return;
  }

  const char* phase = doc["phase"] | "ALL_RED";
  unsigned long duration_ms = doc["duration_ms"] | 1000;

  currentPhase = String(phase);
  phaseEndsAt = millis() + duration_ms;

  applyPhase(currentPhase);

  Serial.print("CMD phase=");
  Serial.print(currentPhase);
  Serial.print(" duration_ms=");
  Serial.println(duration_ms);

  if (doc.containsKey("reason")) {
    Serial.print("Reason: ");
    Serial.println((const char*)doc["reason"]);
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    Serial.print("MQTT connecting...");
    String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      mqtt.subscribe(TOPIC_CMD);
    } else {
      Serial.print("failed rc=");
      Serial.print(mqtt.state());
      Serial.println(" retry in 2s");
      delay(2000);
    }
  }
}

void publishTelemetry() {
  int laneA = analogRead(PIN_LDR);

  // Demo fallback: laneB derived from laneA (replace with real second sensor if available)
  int laneB = laneA / 2 + (esp_random() % 200);

  int pedPressed = (digitalRead(PIN_PED_BTN) == LOW) ? 1 : 0;

  StaticJsonDocument<256> doc;
  doc["device"] = DEVICE_ID;
  doc["ts"] = (uint32_t)(millis() / 1000);
  doc["laneA"] = laneA;
  doc["laneB"] = laneB;
  doc["ped"] = pedPressed;

  char out[256];
  size_t n = serializeJson(doc, out, sizeof(out));
  mqtt.publish(TOPIC_TELE, out, n);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_PED_BTN, INPUT_PULLUP);

  pinMode(A_R, OUTPUT); pinMode(A_Y, OUTPUT); pinMode(A_G, OUTPUT);
  pinMode(B_R, OUTPUT); pinMode(B_Y, OUTPUT); pinMode(B_G, OUTPUT);
  pinMode(PED_LED, OUTPUT);

  setAllOff();
  applyPhase("ALL_RED");

  connectWiFi();
  connectMQTT();
}

unsigned long lastTele = 0;

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();

  if (now - lastTele >= 1000) {
    lastTele = now;
    publishTelemetry();
  }

  // Fail-safe: if phase expires and no new command arrives -> ALL_RED
  if (phaseEndsAt != 0 && now > phaseEndsAt) {
    phaseEndsAt = 0;
    currentPhase = "ALL_RED";
    applyPhase(currentPhase);
  }
}
