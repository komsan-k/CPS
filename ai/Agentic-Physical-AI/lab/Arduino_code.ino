#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "192.168.1.10";

const int LDR_PIN = 36;
const int LED_PIN = 2;
const int BUTTON_PIN = 14;

String deviceId = "esp32_lab_01";
String mode = "AUTO";
String ledState = "OFF";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err) return;

  String action = doc["action"] | "";

  if (action == "LED_ON") {
    digitalWrite(LED_PIN, HIGH);
    ledState = "ON";
  } 
  else if (action == "LED_OFF") {
    digitalWrite(LED_PIN, LOW);
    ledState = "OFF";
  }
  else if (action == "MODE_AUTO") {
    mode = "AUTO";
  }
  else if (action == "MODE_MANUAL") {
    mode = "MANUAL";
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(deviceId.c_str())) {
      client.subscribe("cps/lab/cmd");
    } else {
      delay(2000);
    }
  }
}

String classifyLight(int adc) {
  if (adc < 1200) return "dark";
  else if (adc < 2800) return "medium";
  else return "bright";
}

void publishTelemetry() {
  int adc = analogRead(LDR_PIN);
  String lightLevel = classifyLight(adc);

  StaticJsonDocument<256> doc;
  doc["device"] = deviceId;
  doc["ts"] = millis();
  doc["adc"] = adc;
  doc["light_level"] = lightLevel;
  doc["led_state"] = ledState;
  doc["mode"] = mode;

  char buffer[256];
  serializeJson(doc, buffer);
  client.publish("cps/lab/telemetry", buffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (millis() - lastPublish > 2000) {
    lastPublish = millis();
    publishTelemetry();
  }
}
