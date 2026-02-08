/*
  ESP32-TX Beam Controller (SoftAP) + MQTT publish/subscribe
  - Acts as Wi-Fi SoftAP (BeamTwinAP)
  - Controls beam state via 4 GPIO pins (16 beams)
  - Publishes current beam + status via MQTT
  - Subscribes to "best beam" commands from Node-RED

  Libraries:
    - WiFi (built-in)
    - PubSubClient (install via Arduino Library Manager)
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ---------- SoftAP ----------
const char* AP_SSID = "BeamTwinAP";
const char* AP_PASS = "12345678";

// ---------- MQTT ----------
const char* MQTT_HOST = "192.168.1.100";   // <-- change to your broker IP
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "esp32_tx_beam";
const char* MQTT_USER = "";                // optional
const char* MQTT_PASS = "";                // optional

// Topics
const char* TOPIC_BEAM_ACTIVE = "twin/beam/active";   // publish: {"beam":n,"ts_ms":...}
const char* TOPIC_BEAM_LOCK   = "twin/beam/lock";     // subscribe: "5" or {"beam":5}
const char* TOPIC_STATUS      = "twin/status";        // publish: status strings

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- Beam GPIO ----------
const int beamPins[4] = {16, 17, 18, 19}; // 4-bit -> 16 beams
int currentBeam = 0;
unsigned long lastReconnectAttemptMs = 0;

void setBeam(int beamID) {
  beamID = constrain(beamID, 0, 15);
  for (int i = 0; i < 4; i++) {
    digitalWrite(beamPins[i], (beamID >> i) & 0x01);
  }
  currentBeam = beamID;
}

void publishActiveBeam() {
  char payload[96];
  unsigned long now = millis();
  snprintf(payload, sizeof(payload), "{\"beam\":%d,\"ts_ms\":%lu}", currentBeam, now);
  mqtt.publish(TOPIC_BEAM_ACTIVE, payload, true); // retained
}

int parseBeamFromPayload(const char* payload) {
  if (payload[0] >= '0' && payload[0] <= '9') return atoi(payload);
  const char* key = "\"beam\"";
  const char* p = strstr(payload, key);
  if (!p) return -1;
  p = strchr(p, ':');
  if (!p) return -1;
  p++;
  while (*p == ' ' || *p == '"') p++;
  return atoi(p);
}

void mqttCallback(char* topic, byte* message, unsigned int length) {
  char buf[128];
  unsigned int n = (length < sizeof(buf) - 1) ? length : (sizeof(buf) - 1);
  memcpy(buf, message, n);
  buf[n] = '\0';

  if (strcmp(topic, TOPIC_BEAM_LOCK) == 0) {
    int beam = parseBeamFromPayload(buf);
    if (beam >= 0 && beam <= 15) {
      Serial.printf("[MQTT] Lock beam: %d\n", beam);
      setBeam(beam);
      publishActiveBeam();
      mqtt.publish(TOPIC_STATUS, "TX locked to commanded beam");
    } else {
      mqtt.publish(TOPIC_STATUS, "TX invalid beam command");
    }
  }
}

bool mqttConnect() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  Serial.print("[MQTT] Connecting...");
  bool ok;
  if (strlen(MQTT_USER) > 0) ok = mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
  else ok = mqtt.connect(MQTT_CLIENT_ID);

  if (ok) {
    Serial.println("OK");
    mqtt.subscribe(TOPIC_BEAM_LOCK);
    mqtt.publish(TOPIC_STATUS, "TX online");
    publishActiveBeam();
  } else {
    Serial.printf("FAILED rc=%d\n", mqtt.state());
  }
  return ok;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  for (int i = 0; i < 4; i++) pinMode(beamPins[i], OUTPUT);
  setBeam(0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[WiFi] SoftAP IP: ");
  Serial.println(WiFi.softAPIP());

  mqttConnect();
}

void loop() {
  if (!mqtt.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttemptMs > 3000) {
      lastReconnectAttemptMs = now;
      mqttConnect();
    }
  } else {
    mqtt.loop();
  }

  // Demo sweep
  for (int beam = 0; beam < 16; beam++) {
    if (mqtt.connected()) mqtt.loop();
    setBeam(beam);
    publishActiveBeam();
    delay(250);
  }
}
