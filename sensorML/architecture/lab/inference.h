#pragma once
#include <Arduino.h>
#include "features.h"

enum TinyMLStatus : int {
  TINYML_OK = 0,
  TINYML_MODEL_INVALID = 1,
  TINYML_ALLOC_FAILED = 2,
  TINYML_INPUT_SHAPE_MISMATCH = 3
};

struct InferenceResult {
  const char* label;    // "dark" | "normal" | "bright"
  float confidence;     // max probability
  float probs[3];       // probs per class
};

class TinyML {
public:
  TinyMLStatus begin();
  bool isReady() const { return ready_; }
  InferenceResult infer(const Features& f);

private:
  bool ready_ = false;
};

// Fallback classifier if no valid model is available
InferenceResult fallbackClassify(const Features& f);
