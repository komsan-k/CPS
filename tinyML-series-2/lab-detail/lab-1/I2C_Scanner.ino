/*
  I2C Scanner (ESP32)
  Upload and open Serial Monitor (115200) to find I2C addresses.
*/
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin(); // SDA=21, SCL=22 default
  Serial.println("\nI2C Scanner starting...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found.");
  else Serial.println("Done.");
  delay(3000);
}
