#pragma once
#include <cstdint>

// This header pairs with the generated C array file from:
// xxd -i traffic_cls_int8.tflite > traffic_model_data.cpp
//
// Ensure the symbols match what xxd produced.
// Typically xxd produces something like:
//   unsigned char traffic_cls_int8_tflite[] = {...};
//   unsigned int traffic_cls_int8_tflite_len = ...;

extern const unsigned char traffic_cls_int8_tflite[];
extern const unsigned int traffic_cls_int8_tflite_len;
