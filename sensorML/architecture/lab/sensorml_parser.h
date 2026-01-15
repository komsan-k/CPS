#pragma once
#include <Arduino.h>

// Minimal SensorML subset for the lab
struct SensorConfig {
  String identifier;
  String uom;
  float samplingRateHz;
  float scale;
  float offset;
  float uncertainty;
};

// Parse from PROGMEM XML string (subset tags).
// Returns true if key fields were found.
bool parseSensorMLFromProgmem(const char* xmlProgmem, SensorConfig& out);
