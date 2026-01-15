#pragma once
#include <Arduino.h>

struct Features {
  float mean;
  float std;
  float minv;
  float maxv;
  float slope;   // simple linear slope estimate
};

struct WindowBuffer {
  float* buf;
  int capacity;
  int size;
  int head; // next write index
};

void initWindow(WindowBuffer& w, int capacity);
void pushSample(WindowBuffer& w, float x);
bool isWindowFull(const WindowBuffer& w);
void popOldest(WindowBuffer& w, int n); // slide window by n samples

// Compute mean/std/min/max/slope from the current window
void computeFeatures(const WindowBuffer& w, Features& out);
