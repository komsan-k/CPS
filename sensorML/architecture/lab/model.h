#pragma once
#include <stdint.h>

// -----------------------------------------------------------------------------
// PLACEHOLDER MODEL
// Replace with your own TFLM flatbuffer model bytes (converted .tflite).
// This placeholder lets the project COMPILE; interpreter init will fail at runtime.
// -----------------------------------------------------------------------------

// A minimal dummy array (NOT a valid .tflite model)
static const unsigned char g_model[] = { 0x00, 0x11, 0x22, 0x33 };
static const unsigned int g_model_len = sizeof(g_model);
