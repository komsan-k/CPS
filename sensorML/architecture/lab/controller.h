#pragma once
#include <Arduino.h>
#include "inference.h"

enum ControlAction : int {
  ACTION_SAFE_OFF = 0,
  ACTION_LED_ON   = 1
};

class Controller {
public:
  void begin(int pinLed);
  ControlAction safetyAndActuate(const InferenceResult& r, float uncertainty);

private:
  int pinLed_ = -1;

  // Majority vote / debounce
  static constexpr int kNeedSame = 3;
  const char* lastLabel_ = nullptr;
  int sameCount_ = 0;

  // Hysteresis-like behavior using uncertainty as margin idea
  bool ledState_ = false;
};
