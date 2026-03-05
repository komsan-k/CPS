#include <Arduino.h>
#include "traffic_model_data.h"  // ได้จาก xxd -i
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// ----------------- TFLM Globals -----------------
constexpr int kArenaSize = 20 * 1024;
static uint8_t tensor_arena[kArenaSize];

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// --------- Helper: clamp int8 ----------
static inline int8_t clamp_int8(int x){
  if (x > 127) return 127;
  if (x < -128) return -128;
  return (int8_t)x;
}

// Normalize -> quantize (ต้อง match กับ training/quantization)
int8_t quantize_float_to_int8(float x, float scale, int zero_point){
  int q = (int)roundf(x / scale) + zero_point;
  return clamp_int8(q);
}

void setup_tflm() {
  model = tflite::GetModel(traffic_tinyml_int8_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while(1);
  }

  static tflite::AllOpsResolver resolver; // ง่ายสุด (ไฟล์ใหญ่ขึ้นนิด)
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed!");
    while(1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TFLM ready.");
}

int argmax3(const int8_t* v){
  int best = 0;
  for(int i=1;i<3;i++) if(v[i] > v[best]) best=i;
  return best;
}

void setup() {
  Serial.begin(115200);
  setup_tflm();
}

// ตัวอย่าง: features ที่ normalize แล้ว (คุณแทนด้วยค่าจริงจากเซนเซอร์/คิว)
void loop() {
  float f[6] = {
    0.20f, // c_ns_norm
    0.10f, // c_ew_norm
    0.30f, // q_ns_norm
    0.15f, // q_ew_norm
    0.50f, // green_ns_norm
    0.50f  // green_ew_norm
  };

  // quant params มาจาก input tensor
  float scale = input->params.scale;
  int zp = input->params.zero_point;

  for(int i=0;i<6;i++){
    input->data.int8[i] = quantize_float_to_int8(f[i], scale, zp);
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed!");
    delay(500);
    return;
  }

  // output int8 3 ค่า
  const int8_t* y = output->data.int8;
  int cls = argmax3(y);

  const char* label = (cls==0)?"LOW":(cls==1)?"MEDIUM":"HIGH";
  Serial.print("Predicted congestion: ");
  Serial.println(label);

  // TODO: ส่ง MQTT ไป Node-RED เช่น topic: city/intersection1/prediction

  delay(1000);
}
