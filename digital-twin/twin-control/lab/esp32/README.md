# ESP32 RSSI-Based Twin-Control Beam Steering Lab

This README describes a **complete, lab-ready Twin-Control beam-steering system** using two ESP32 boards. The setup demonstrates how **RSSI-based sensing**, **beam reconfiguration**, and **feedback-driven decision making** together realize a practical **Twin-Control loop** for smart antennas or reconfigurable metasurfaces.

---

## 1. System Overview (ESP32 Twin-Control)

```
[ ESP32-TX ]  ---> Wi-Fi packets --->  [ ESP32-RX ]
     |                                      |
     |-- GPIO beam states                   |-- RSSI measurement
     |                                      |
     <----------- Best Beam ID --------------
```

### Roles

* **ESP32-TX (Transmitter / Beam Controller)**
  Controls antenna or metasurface configuration using GPIO pins (e.g., PIN diodes or phase bits).

* **ESP32-RX (Receiver / RSSI Sensor)**
  Measures Wi-Fi RSSI for each beam and decides which beam performs best.

### Twin-Control Logic

RSSI represents **real-world physical feedback**.
Beam ID is the **control variable**.
Scanning, selection, and reconfiguration form a **closed-loop Twin-Control system**.

---

## 2. ESP32-TX: Beam Controller (SoftAP)

### Hardware Assumptions

* 4 GPIO pins → **16 beam states** (4-bit control)
* Replace GPIO mapping with:

  * PIN diode drivers
  * Phase shifters
  * Coding metasurface controllers

### ESP32_TX_BeamController.ino

```cpp
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
```

---

## 3. ESP32-RX: RSSI Sensor & Decision Node

### Function

* Connects to TX SoftAP
* Measures RSSI for each beam
* Computes average RSSI
* Sends **best beam ID** back to TX

### ESP32_RX_RSSI.ino

```cpp
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

      // Decide best beam after enough samples
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
```

---

## 4. Why This Is **Twin-Control** (Not Just Beam Search)

| Element          | Role in Twin-Control     |
| ---------------- | ------------------------ |
| RSSI             | Physical sensing         |
| Beam code        | Control variable         |
| Scan + selection | Cognitive decision       |
| Lock + rescan    | Closed-loop control      |
| Digital Twin     | Implicit beam–RSSI model |

### Digital Twin Interpretation

The **digital twin** can be:

* A lookup table: `beam → expected RSSI`
* A machine-learning model predicting best beam
* EM-simulation-derived patterns (CST / HFSS)

RSSI measurements continuously **correct the twin**, and the updated twin **reconfigures the physical system** — completing a true **Twin-Control loop**.

---

## 5. Learning Outcomes

By completing this lab, students will:

* Understand RSSI as a **virtual sensor**
* Implement real-time **beam control** with ESP32
* Build a **closed-loop Twin-Control system**
* Connect Digital Twin concepts to real RF hardware

---

## 6. Extensions (Optional)

* Replace RSSI averaging with **ML-based prediction**
* Add **Node-RED dashboard** for beam/RSSI visualization
* Integrate **SensorML metadata** for self-describing beams
* Extend to **multi-user beam selection**

---

**This lab directly supports teaching Digital Twin, CPS, and Intelligent RF Systems for 6G and Smart Antennas.**
