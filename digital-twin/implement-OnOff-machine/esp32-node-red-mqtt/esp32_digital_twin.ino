#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

#define MAIN_LED 2
#define BACKUP_LED 5

void setup() {
  pinMode(MAIN_LED, OUTPUT);
  pinMode(BACKUP_LED, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) client.connect("esp32_dt");
  client.loop();
  digitalWrite(MAIN_LED, HIGH);
  client.publish("esp32/main_led", "ON");
  delay(3000);
}
