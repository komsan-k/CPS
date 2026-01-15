#include "inference.h"
#include "model.h"

// Try common TFLM include paths.
// If compilation fails, adjust these includes to match your installed library.
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

static tflite::AllOpsResolver g_resolver;
static const tflite::Model* g_tflm_model = nullptr;
static tflite::MicroInterpreter* g_interpreter = nullptr;
static TfLiteTensor* g_input = nullptr;
static TfLiteTensor* g_output = nullptr;

// Arena size: adjust when using a real model
static constexpr int kArenaSize = 20 * 1024;
static uint8_t g_tensor_arena[kArenaSize];

TinyMLStatus TinyML::begin()
{
  ready_ = false;

  g_tflm_model = tflite::GetModel(g_model);
  if (!g_tflm_model) return TINYML_MODEL_INVALID;

  if (g_tflm_model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model schema %d != supported %d",
                (int)g_tflm_model->version(), (int)TFLITE_SCHEMA_VERSION);
    return TINYML_MODEL_INVALID;
  }

  static tflite::MicroInterpreter static_interpreter(
      g_tflm_model, g_resolver, g_tensor_arena, kArenaSize);
  g_interpreter = &static_interpreter;

  if (g_interpreter->AllocateTensors() != kTfLiteOk) {
    return TINYML_ALLOC_FAILED;
  }

  g_input = g_interpreter->input(0);
  g_output = g_interpreter->output(0);

  // Expect input: [1, 5] float
  if (!g_input || g_input->type != kTfLiteFloat32) return TINYML_INPUT_SHAPE_MISMATCH;
  if (g_input->dims->size < 2) return TINYML_INPUT_SHAPE_MISMATCH;

  ready_ = true;
  return TINYML_OK;
}

static InferenceResult makeResult(const float p0, const float p1, const float p2)
{
  InferenceResult r;
  r.probs[0] = p0; r.probs[1] = p1; r.probs[2] = p2;

  int imax = 0;
  float vmax = r.probs[0];
  if (r.probs[1] > vmax) { vmax = r.probs[1]; imax = 1; }
  if (r.probs[2] > vmax) { vmax = r.probs[2]; imax = 2; }

  r.confidence = vmax;
  r.label = (imax == 0) ? "dark" : (imax == 1) ? "normal" : "bright";
  return r;
}

InferenceResult TinyML::infer(const Features& f)
{
  if (!ready_) return fallbackClassify(f);

  // Fill input features: mean, std, min, max, slope
  g_input->data.f[0] = f.mean;
  g_input->data.f[1] = f.std;
  g_input->data.f[2] = f.minv;
  g_input->data.f[3] = f.maxv;
  g_input->data.f[4] = f.slope;

  if (g_interpreter->Invoke() != kTfLiteOk) {
    return fallbackClassify(f);
  }

  // Assume output: [1, 3] float probs (softmax)
  float p0 = g_output->data.f[0];
  float p1 = g_output->data.f[1];
  float p2 = g_output->data.f[2];

  return makeResult(p0, p1, p2);
}

InferenceResult fallbackClassify(const Features& f)
{
  // Simple thresholds on mean (ADC counts) for demo
  // You can tune these based on your sensor setup.
  const float t_dark = 1200.0f;
  const float t_bright = 2800.0f;

  if (f.mean < t_dark)  return makeResult(0.85f, 0.10f, 0.05f);
  if (f.mean > t_bright) return makeResult(0.05f, 0.10f, 0.85f);
  return makeResult(0.10f, 0.80f, 0.10f);
}
