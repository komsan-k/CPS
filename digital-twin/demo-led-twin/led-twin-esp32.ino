//
// === ESP32 LED Digital Twin: Interrupt + Debounce + MQTT ===
// - Toggles LED on GPIO12 with a button on GPIO14 using an interrupt
// - Publishes LED state and button events via MQTT
// - Subscribes to LED command topic to control LED from Node-RED dashboard
//
// Replace Wi-Fi and MQTT settings below before uploading.
//

#include <WiFi.h>
#include <PubSubClient.h>

// -------------------- User Config --------------------
#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASSWORD"

#define MQTT_HOST   "192.168.1.10"   // Broker IP or hostname
#define MQTT_PORT   1883

// MQTT Topics
const char* TOPIC_STATE = "iot/lab/led/state";     // payload: "ON"/"OFF"
const char* TOPIC_CMD   = "iot/lab/led/cmd";       // payload: "ON"/"OFF"/"1"/"0"/"TOGGLE"
const char* TOPIC_BTN   = "iot/lab/button/event";  // payload: "press"

// Pins
const int LED_PIN    = 12;   // LED output (active HIGH)
const int BUTTON_PIN = 14;   // Button input with internal pull-up (active LOW)

// Debounce (ms)
const unsigned long debounceDelay = 200;

// -------------------- Globals --------------------
WiFiClient espClient;
PubSubClient mqtt(espClient);

volatile bool ledState = false;                 // LED ON/OFF
volatile unsigned long lastInterruptTime = 0;   // last ISR time (ms)
volatile bool buttonEventPending = false;       // defer publish/print to loop()

// -------------------- Helpers --------------------
void publishState() {
  const char* s = ledState ? "ON" : "OFF";
  mqtt.publish(TOPIC_STATE, s);
  Serial.print("[STATE] Published: "); Serial.println(s);
}

void publishButtonEvent() {
  mqtt.publish(TOPIC_BTN, "press");
  Serial.println("[BUTTON] Published: press");
}

void setLed(bool on) {
  ledState = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

// -------------------- ISR --------------------
void IRAM_ATTR handleButtonPress() {
  unsigned long now = millis();
  if (now - lastInterruptTime > debounceDelay) {
    // Toggle LED state
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    // Defer network/serial work to loop
    buttonEventPending = true;
    lastInterruptTime = now;
  }
}

// -------------------- MQTT --------------------
void onMqttMsg(char* topic, byte* payload, unsigned int len) {
  String t = String(topic);
  String msg;
  msg.reserve(len);
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim(); msg.toUpperCase();

  Serial.print("[MQTT] Topic: "); Serial.print(t);
  Serial.print("  Payload: "); Serial.println(msg);

  if (t == TOPIC_CMD) {
    if (msg == "ON" || msg == "1" || msg == "TRUE") {
      setLed(true);
      publishState();
    } else if (msg == "OFF" || msg == "0" || msg == "FALSE") {
      setLed(false);
      publishState();
    } else if (msg == "TOGGLE") {
      setLed(!ledState);
      publishState();
    } else {
      Serial.println("[MQTT] Unknown command");
    }
  }
}

void ensureMqtt() {
  while (!mqtt.connected()) {
    String cid = "esp32-led-twin-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("[MQTT] Connecting as "); Serial.println(cid);
    if (mqtt.connect(cid.c_str())) {
      Serial.println("[MQTT] Connected");
      mqtt.subscribe(TOPIC_CMD);
      publishState(); // announce state on (re)connect
    } else {
      Serial.print("[MQTT] Connect failed, rc="); Serial.println(mqtt.state());
      delay(1500);
    }
  }
}

// -------------------- Wi-Fi --------------------
void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("[WiFi] Connecting to "); Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
    if (++dots % 20 == 0) Serial.println();
  }
  Serial.println();
  Serial.print("[WiFi] Connected. IP: "); Serial.println(WiFi.localIP());
}

// -------------------- Arduino --------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // initial OFF

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  ensureWifi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMsg);
  ensureMqtt();

  Serial.println("System ready. Press the button or use Node-RED to control the LED.");
}

void loop() {
  ensureWifi();
  if (!mqtt.connected()) ensureMqtt();
  mqtt.loop();

  // Handle deferred button event (from ISR)
  if (buttonEventPending) {
    // Clear flag first to avoid duplicate handling
    buttonEventPending = false;
    publishButtonEvent();
    publishState();
  }
}
