#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "BeamTwinAP";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 4210;

int rssiSum[16];
int rssiCount[16];

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  udp.begin(udpPort);
  Serial.println("\nESP32-RX RSSI Node Ready");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buffer[32];
    int len = udp.read(buffer, 32);
    buffer[len] = '\0';

    if (strncmp(buffer, "BEAM:", 5) == 0) {
      int beam = atoi(buffer + 5);
      int rssi = WiFi.RSSI();

      rssiSum[beam] += rssi;
      rssiCount[beam]++;

      Serial.printf("Beam %d RSSI %d dBm\n", beam, rssi);

      // After enough samples, decide best beam
      if (rssiCount[beam] >= 20) {
        int bestBeam = 0;
        float bestAvg = -999;

        for (int i = 0; i < 16; i++) {
          if (rssiCount[i] > 0) {
            float avg = (float)rssiSum[i] / rssiCount[i];
            if (avg > bestAvg) {
              bestAvg = avg;
              bestBeam = i;
            }
          }
        }

        udp.beginPacket("192.168.4.1", udpPort);
        udp.printf("BEST:%d", bestBeam);
        udp.endPacket();

        memset(rssiSum, 0, sizeof(rssiSum));
        memset(rssiCount, 0, sizeof(rssiCount));
      }
    }
  }
}
