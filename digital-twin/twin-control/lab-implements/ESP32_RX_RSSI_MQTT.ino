/*
  ESP32-RX RSSI Sensor + MQTT publisher
  - Connects to TX SoftAP (BeamTwinAP)
  - Subscribes to active beam (retained) from TX via MQTT
  - Reads WiFi.RSSI() and publishes RSSI stream
  - Optionally computes best beam locally and publishes suggestion

  Libraries:
    - WiFi (built-in)
    - PubSubClient (install via Arduino Library Manager)
*/

#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "BeamTwinAP";
const char* WIFI_PASS = "12345678";

const char* MQTT_HOST = "192.168.1.100";  // <-- change to broker IP
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "esp32_rx_rssi";
const char* MQTT_USER = "";
const char* MQTT_PASS = "";

const char* TOPIC_BEAM_ACTIVE = "twin/beam/active";
const char* TOPIC_RSSI        = "twin/rssi";
const char* TOPIC_BEST_SUGG   = "twin/beam/best";
const char* TOPIC_STATUS      = "twin/status";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

int activeBeam = -1;

long rssiSum[16] = {0};
int  rssiCount[16] = {0};
const int SAMPLES_PER_BEAM = 10;
const int MIN_TOTAL_SAMPLES = 40;

unsigned long lastPublishMs = 0;
const unsigned long PUBLISH_PERIOD_MS = 200;

int parseBeamFromJson(const char* payload) {
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

  if (strcmp(topic, TOPIC_BEAM_ACTIVE) == 0) {
    int b = parseBeamFromJson(buf);
    if (b >= 0 && b <= 15) activeBeam = b;
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
    mqtt.subscribe(TOPIC_BEAM_ACTIVE);
    mqtt.publish(TOPIC_STATUS, "RX online");
  } else {
    Serial.printf("FAILED rc=%d\n", mqtt.state());
  }
  return ok;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WiFi] Connecting");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    if (millis() - t0 > 15000) {
      Serial.println("\n[WiFi] Timeout");
      return;
    }
  }
  Serial.println("\n[WiFi] Connected");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

int totalSamples() {
  int sum = 0;
  for (int i = 0; i < 16; i++) sum += rssiCount[i];
  return sum;
}

void publishRssi(int beam, int rssi) {
  char payload[96];
  unsigned long now = millis();
  snprintf(payload, sizeof(payload), "{\"beam\":%d,\"rssi\":%d,\"ts_ms\":%lu}", beam, rssi, now);
  mqtt.publish(TOPIC_RSSI, payload);
}

void publishBestBeam(int bestBeam, float bestAvg) {
  char payload[96];
  snprintf(payload, sizeof(payload), "{\"beam\":%d,\"avg_rssi\":%.2f}", bestBeam, bestAvg);
  mqtt.publish(TOPIC_BEST_SUGG, payload);
}

void resetStats() {
  memset(rssiSum, 0, sizeof(rssiSum));
  memset(rssiCount, 0, sizeof(rssiCount));
}

void setup() {
  Serial.begin(115200);
  delay(200);

  connectWiFi();
  mqttConnect();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    delay(1000);
    return;
  }

  if (!mqtt.connected()) {
    mqttConnect();
    delay(500);
    return;
  }
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPublishMs < PUBLISH_PERIOD_MS) return;
  lastPublishMs = now;

  if (activeBeam < 0) return;

  int rssi = WiFi.RSSI();
  publishRssi(activeBeam, rssi);

  rssiSum[activeBeam] += rssi;
  rssiCount[activeBeam]++;

  if (rssiCount[activeBeam] >= SAMPLES_PER_BEAM && totalSamples() >= MIN_TOTAL_SAMPLES) {
    int bestBeam = 0;
    float bestAvg = -999.0f;

    for (int i = 0; i < 16; i++) {
      if (rssiCount[i] > 0) {
        float avg = (float)rssiSum[i] / (float)rssiCount[i];
        if (avg > bestAvg) {
          bestAvg = avg;
          bestBeam = i;
        }
      }
    }

    Serial.printf("[BEST] Beam %d avg=%.2f dBm\n", bestBeam, bestAvg);
    publishBestBeam(bestBeam, bestAvg);
    resetStats();
  }
}
