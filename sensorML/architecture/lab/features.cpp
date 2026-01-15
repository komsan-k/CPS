#include "features.h"
#include <math.h>

void initWindow(WindowBuffer& w, int capacity)
{
  w.capacity = capacity;
  w.size = 0;
  w.head = 0;
  w.buf = (float*)malloc(sizeof(float) * capacity);
  for (int i = 0; i < capacity; i++) w.buf[i] = 0.0f;
}

void pushSample(WindowBuffer& w, float x)
{
  if (!w.buf) return;
  w.buf[w.head] = x;
  w.head = (w.head + 1) % w.capacity;
  if (w.size < w.capacity) w.size++;
}

bool isWindowFull(const WindowBuffer& w)
{
  return w.size >= w.capacity;
}

static float at(const WindowBuffer& w, int idxOldestToNewest)
{
  // idx=0 is oldest, idx=capacity-1 is newest (when full)
  int oldest = (w.head - w.size);
  while (oldest < 0) oldest += w.capacity;
  int i = (oldest + idxOldestToNewest) % w.capacity;
  return w.buf[i];
}

void popOldest(WindowBuffer& w, int n)
{
  if (n <= 0) return;
  if (n >= w.size) {
    w.size = 0;
    return;
  }
  w.size -= n;
  // head remains; oldest advances implicitly because size reduced
}

void computeFeatures(const WindowBuffer& w, Features& out)
{
  const int N = w.size;
  if (N <= 1) {
    out = {0,0,0,0,0};
    return;
  }

  float sum = 0.0f;
  float minv = at(w, 0);
  float maxv = minv;

  for (int i = 0; i < N; i++) {
    float x = at(w, i);
    sum += x;
    if (x < minv) minv = x;
    if (x > maxv) maxv = x;
  }

  float mean = sum / (float)N;

  float var = 0.0f;
  for (int i = 0; i < N; i++) {
    float d = at(w, i) - mean;
    var += d * d;
  }
  var /= (float)(N - 1);
  float std = sqrtf(var);

  // simple slope via endpoints (newest - oldest) / (N-1)
  float slope = (at(w, N - 1) - at(w, 0)) / (float)(N - 1);

  out.mean = mean;
  out.std = std;
  out.minv = minv;
  out.maxv = maxv;
  out.slope = slope;
}
