#include "controller.h"

void Controller::begin(int pinLed)
{
  pinLed_ = pinLed;
  pinMode(pinLed_, OUTPUT);
  digitalWrite(pinLed_, LOW);
  ledState_ = false;
  lastLabel_ = nullptr;
  sameCount_ = 0;
}

ControlAction Controller::safetyAndActuate(const InferenceResult& r, float uncertainty)
{
  // Fail-safe: if low confidence, go safe OFF
  const float conf_min = 0.60f; // can be tuned
  if (r.confidence < conf_min) {
    ledState_ = false;
    digitalWrite(pinLed_, LOW);
    lastLabel_ = nullptr;
    sameCount_ = 0;
    return ACTION_SAFE_OFF;
  }

  // Debounce / majority vote: require same label 3 times
  if (lastLabel_ && strcmp(lastLabel_, r.label) == 0) sameCount_++;
  else { lastLabel_ = r.label; sameCount_ = 1; }

  if (sameCount_ < kNeedSame) {
    // keep previous state (hysteresis-like)
    digitalWrite(pinLed_, ledState_ ? HIGH : LOW);
    return ledState_ ? ACTION_LED_ON : ACTION_SAFE_OFF;
  }

  // Action mapping (example):
  // - bright => LED ON
  // - dark/normal => LED OFF
  bool wantOn = (strcmp(r.label, "bright") == 0);

  // Use uncertainty as a reason to be conservative:
  // If uncertainty is high, require stronger evidence (here: higher confidence).
  if (uncertainty > 0.08f && r.confidence < 0.75f) {
    wantOn = false;
  }

  ledState_ = wantOn;
  digitalWrite(pinLed_, ledState_ ? HIGH : LOW);

  return ledState_ ? ACTION_LED_ON : ACTION_SAFE_OFF;
}
