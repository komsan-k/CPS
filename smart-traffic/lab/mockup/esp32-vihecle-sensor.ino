#include <WiFi.h>
#include <PubSubClient.h>

// -------- WiFi / MQTT --------
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";

const char* MQTT_HOST = "broker.hivemq.com";   // เปลี่ยนได้
const int   MQTT_PORT = 1883;

const char* TOPIC_TRAFFIC = "city/intersection1/traffic";

// -------- Mock sensor buttons --------
const int BTN_NS = 14;   // 1 press = 1 vehicle (North-South)
const int BTN_EW = 16;   // 1 press = 1 vehicle (East-West)

// -------- Debounce --------
volatile uint32_t nsCount = 0;
volatile uint32_t ewCount = 0;

volatile uint32_t lastNsMs = 0;
volatile uint32_t lastEwMs = 0;
const uint32_t DEBOUNCE_MS = 120;  // กันเด้งปุ่ม

// -------- Report window --------
const uint32_t WINDOW_MS = 10000;  // สรุปทุก 10 วินาที
uint32_t lastReportMs = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// ISR: Button press -> +1 vehicle
void IRAM_ATTR isrNs() {
  uint32_t now = millis();
  if (now - lastNsMs >= DEBOUNCE_MS) {
    nsCount++;
    lastNsMs = now;
  }
}

void IRAM_ATTR isrEw() {
  uint32_t now = millis();
  if (now - lastEwMs >= DEBOUNCE_MS) {
    ewCount++;
    lastEwMs = now;
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void connectMQTT() {
  client.setServer(MQTT_HOST, MQTT_PORT);
  while (!client.connected()) {
    String cid = "esp32-traffic-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    client.connect(cid.c_str());
    delay(300);
  }
}

void setup() {
  Serial.begin(115200);

  // Buttons (active LOW with pullup)
  pinMode(BTN_NS, INPUT_PULLUP);
  pinMode(BTN_EW, INPUT_PULLUP);

  // Interrupt on FALLING edge (when pressed to GND)
  attachInterrupt(digitalPinToInterrupt(BTN_NS), isrNs, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_EW), isrEw, FALLING);

  connectWiFi();
  connectMQTT();

  lastReportMs = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) connectMQTT();
  client.loop();

  uint32_t now = millis();
  if (now - lastReportMs >= WINDOW_MS) {
    // Copy & reset counts atomically
    noInterrupts();
    uint32_t ns = nsCount; nsCount = 0;
    uint32_t ew = ewCount; ewCount = 0;
    interrupts();

    // JSON payload
    // ns_10s, ew_10s = จำนวนรถใน 10 วินาทีล่าสุด
    String payload = String("{\"ns_10s\":") + ns +
                     ",\"ew_10s\":" + ew +
                     ",\"window_ms\":" + WINDOW_MS +
                     "}";

    client.publish(TOPIC_TRAFFIC, payload.c_str());
    Serial.println(payload);

    lastReportMs = now;
  }
}
