# 🧪 CPS / IoT Lab (Solution)
## Implementing a Digital Twin in Node-RED for On–Off LED Control Using ESP32
### ✅ With Backup LED + Failure Detection (LDR-based)

This instructor pack includes:
- Full ESP32 firmware (MQTT control + feedback + LDR failure detection + backup activation)
- Node-RED Dashboard flow (control + monitoring)
- Calibration notes and marking guidance

---

## 1) Reference Concepts (for discussion)
- A Digital Twin provides **real-time synchronization** between a physical system and its virtual representation.
- This lab supports a staged CPS learning path:
  - Phase 1: manual control
  - Phase 2: perception (LDR)
  - Phase 3: IoT (MQTT + Node-RED)
  - Phase 4: visualization (Dashboard)
  - Phase 5: cognition (fault/prediction + backup activation)

---

## 2) MQTT Topics (Recommended)
Commands:
- `dt/main/cmd` : ON/OFF
- `dt/reset` : RESET

Telemetry:
- `dt/main/state`
- `dt/backup/state`
- `dt/fault`
- `dt/ldr`

*(If you want a simplified version, you can map dt/main/cmd to led/control.)*

---

## 3) ESP32 Full Solution Code (Arduino)
> Configure Wi‑Fi + MQTT broker and upload to ESP32.

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST = "broker.hivemq.com"; // or local Mosquitto IP
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_MAIN_CMD     = "dt/main/cmd";
const char* TOPIC_RESET        = "dt/reset";
const char* TOPIC_MAIN_STATE   = "dt/main/state";
const char* TOPIC_BACKUP_STATE = "dt/backup/state";
const char* TOPIC_FAULT        = "dt/fault";
const char* TOPIC_LDR          = "dt/ldr";

WiFiClient espClient;
PubSubClient client(espClient);

#define MAIN_LED    2
#define BACKUP_LED  5
#define SWITCH_PIN  4
#define LDR_PIN     34

const int LDR_OK_THRESHOLD = 1200;      // calibrate
const int FAIL_PERSIST_COUNT = 6;
const unsigned long HEALTH_INTERVAL_MS = 500;

bool main_cmd_on = false;
bool backup_on = false;
bool fault = false;
int fail_counter = 0;
unsigned long last_health_ms = 0;

void publish_state() {
  client.publish(TOPIC_MAIN_STATE, main_cmd_on ? "ON" : "OFF", true);
  client.publish(TOPIC_BACKUP_STATE, backup_on ? "ON" : "OFF", true);
  client.publish(TOPIC_FAULT, fault ? "FAIL" : "OK", true);
}

void set_main(bool on) {
  main_cmd_on = on;
  digitalWrite(MAIN_LED, on ? HIGH : LOW);
  if (!on) fail_counter = 0;
  publish_state();
}

void set_backup(bool on) {
  backup_on = on;
  digitalWrite(BACKUP_LED, on ? HIGH : LOW);
  publish_state();
}

void reset_fault() {
  fault = false;
  fail_counter = 0;
  set_backup(false);
  publish_state();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  if (String(topic) == TOPIC_MAIN_CMD) {
    if (msg == "ON") set_main(true);
    else if (msg == "OFF") set_main(false);
  } else if (String(topic) == TOPIC_RESET) {
    if (msg == "RESET") reset_fault();
  }
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(400);
}

void ensure_mqtt_connected() {
  while (!client.connected()) {
    String clientId = "esp32_dt_" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(TOPIC_MAIN_CMD);
      client.subscribe(TOPIC_RESET);
      publish_state();
    } else {
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(MAIN_LED, OUTPUT);
  pinMode(BACKUP_LED, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  digitalWrite(MAIN_LED, LOW);
  digitalWrite(BACKUP_LED, LOW);

  setup_wifi();
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(mqtt_callback);
  ensure_mqtt_connected();
  publish_state();
}

void loop() {
  if (!client.connected()) ensure_mqtt_connected();
  client.loop();

  // manual toggle
  static int last_button = HIGH;
  static unsigned long last_debounce = 0;
  int b = digitalRead(SWITCH_PIN);
  if (b != last_button) { last_debounce = millis(); last_button = b; }

  if ((millis() - last_debounce) > 80) {
    static int stable_state = HIGH;
    if (b != stable_state) {
      stable_state = b;
      if (stable_state == LOW) set_main(!main_cmd_on);
    }
  }

  // health check
  unsigned long now = millis();
  if (now - last_health_ms >= HEALTH_INTERVAL_MS) {
    last_health_ms = now;

    int ldr = analogRead(LDR_PIN);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", ldr);
    client.publish(TOPIC_LDR, buf, false);

    if (main_cmd_on && !fault) {
      if (ldr < LDR_OK_THRESHOLD) fail_counter++;
      else if (fail_counter > 0) fail_counter--;

      if (fail_counter >= FAIL_PERSIST_COUNT) {
        fault = true;
        set_backup(true);
        publish_state();
      }
    }

    if (!main_cmd_on && !fault && backup_on) set_backup(false);
  }
}
```

---

## 4) Calibration Notes (LDR)
1. Read `dt/ldr` with main LED OFF → record typical value
2. Read `dt/ldr` with main LED ON → record typical value
3. Choose `LDR_OK_THRESHOLD` safely between them.
4. Use `FAIL_PERSIST_COUNT` to reduce false alarms.

---

## 5) Node-RED Instructor Flow
Use the provided **Node-RED JSON** (control + monitoring):
- UI Switch → publish `dt/main/cmd`
- Reset button → publish `dt/reset`
- MQTT IN → display main/backup/fault and plot LDR

> Import JSON flow (Menu → Import) and configure broker.

---

## 6) Marking Guidance (100)
- If students complete only basic LED digital twin (no LDR/backup), award marks based on the student rubric.
- Award extension marks for:
  - correct fault detection logic
  - stable MQTT design
  - clean dashboard UX
  - clear explanation of physical ↔ virtual synchronization

---

## 7) Quick Demo Checklist
- UI ON/OFF toggles main LED
- `dt/main/state` updates correctly
- Cover LDR / reduce brightness → fault triggers backup
- Reset clears fault and turns backup OFF
