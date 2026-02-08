#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "BeamTwinAP";
const char* password = "12345678";

WiFiUDP udp;
const int udpPort = 4210;

const int beamPins[4] = {16, 17, 18, 19}; // Beam control bits
int currentBeam = 0;

void setBeam(int beamID) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(beamPins[i], (beamID >> i) & 0x01);
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(beamPins[i], OUTPUT);
  }

  WiFi.softAP(ssid, password);
  udp.begin(udpPort);

  Serial.println("ESP32-TX Beam Controller Ready");
}

void loop() {
  for (int beam = 0; beam < 16; beam++) {
    setBeam(beam);
    currentBeam = beam;

    // Notify RX which beam is active
    udp.beginPacket("192.168.4.255", udpPort);
    udp.printf("BEAM:%d", beam);
    udp.endPacket();

    delay(250); // Beam hold time
  }

  // Listen for best beam feedback
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buffer[32];
    int len = udp.read(buffer, 32);
    buffer[len] = '\0';

    if (strncmp(buffer, "BEST:", 5) == 0) {
      int bestBeam = atoi(buffer + 5);
      Serial.printf("Locking best beam: %d\n", bestBeam);
      setBeam(bestBeam);
      delay(3000); // Lock time
    }
  }
}
