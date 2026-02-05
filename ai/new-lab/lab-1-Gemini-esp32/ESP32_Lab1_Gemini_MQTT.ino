/*
  Lab-1: ESP32 + MQTT + Node-RED + Gemini (CPS Loop)
  Telemetry topic: lab1/esp32/telemetry
  Command topic:   lab1/esp32/cmd/led   payload "0" or "1"

  Hardware:
    - LED on GPIO 2 (many boards have onboard LED on GPIO2)
    - LDR voltage divider to ADC pin (GPIO34 recommended)

  Libraries (Arduino IDE):
    - PubSubClient
    - WiFi (built-in)
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ======= USER SETTINGS =======
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// MQTT broker (local Mosquitto recommended)
const char* MQTT_HOST = "192.168.1.10";   // <-- change to your broker IP (or "broker.hivemq.com")
const uint16_t MQTT_PORT = 1883;

// Topics
const char* TOPIC_TELE = "lab1/esp32/telemetry";
const char* TOPIC_LED  = "lab1/esp32/cmd/led";

// Pins
const int PIN_LED = 2;
const int PIN_LDR = 34;   // ADC input-only pin on many ESP32 boards

// Publish interval
const unsigned long PUBLISH_MS = 1000;

// =============================
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastPub = 0;
int ledState = 0;

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Copy payload into a string
  String msg;
  msg.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  if (String(topic) == TOPIC_LED) {
    if (msg == "1" || msg == "ON" || msg == "on") ledState = 1;
    else ledState = 0;

    digitalWrite(PIN_LED, ledState ? HIGH : LOW);

    Serial.print("LED cmd received: ");
    Serial.println(ledState);
  }
}

static void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32-lab1-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("MQTT connecting as ");
    Serial.print(clientId);
    Serial.print(" ... ");

    // No username/password in this template
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      mqtt.subscribe(TOPIC_LED);
      Serial.print("Subscribed: ");
      Serial.println(TOPIC_LED);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retry in 2s");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // ADC setup (optional tuning)
  analogReadResolution(12); // 0..4095
  analogSetAttenuation(ADC_11db); // better range for 3.3V dividers

  connectWiFi();
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPub >= PUBLISH_MS) {
    lastPub = now;

    int ldr = analogRead(PIN_LDR); // 0..4095
    int led = ledState;

    // Simple JSON telemetry
    // Example: {"ldr":1234,"led":1,"rssi":-55}
    String payload = "{";
    payload += "\"ldr\":" + String(ldr) + ",";
    payload += "\"led\":" + String(led) + ",";
    payload += "\"rssi\":" + String(WiFi.RSSI());
    payload += "}";

    mqtt.publish(TOPIC_TELE, payload.c_str());
    Serial.print("Published: ");
    Serial.println(payload);
  }
}
