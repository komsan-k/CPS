#include <Arduino.h>

#include "sensorml_parser.h"
#include "features.h"
#include "inference.h"
#include "controller.h"

// ---------- Sensor wiring ----------
static const int PIN_LDR = 34;     // ADC1 (ESP32 DevKit)
static const int PIN_LED = 2;      // Onboard LED on many ESP32 boards

// ---------- SensorML (subset) as an embedded XML string ----------
static const char kSensorML[] PROGMEM = R"XML(
<SensorML>
  <member>
    <System>
      <identifier>LDR_ESP32_01</identifier>
      <outputs>
        <output name="light">
          <uom>adc_counts</uom>
          <calibration>
            <scale>1.0</scale>
            <offset>0.0</offset>
          </calibration>
          <samplingRateHz>20</samplingRateHz>
          <uncertainty>0.05</uncertainty>
        </output>
      </outputs>
    </System>
  </member>
</SensorML>
)XML";

// ---------- Global state ----------
SensorConfig g_cfg;
WindowBuffer g_win;
TinyML g_ml;
Controller g_ctrl;

// sampling timing
unsigned long g_lastSampleMs = 0;
unsigned long g_samplePeriodMs = 50; // default 20 Hz

static float readLdrAdc()
{
  // ESP32 ADC range depends on attenuation; keep it simple for lab
  int raw = analogRead(PIN_LDR);
  return (float)raw;
}

void setup()
{
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Parse SensorML subset
  if (!parseSensorMLFromProgmem(kSensorML, g_cfg)) {
    Serial.println("[SensorML] Parse failed, using defaults.");
    g_cfg.identifier = "LDR_ESP32_01";
    g_cfg.uom = "adc_counts";
    g_cfg.scale = 1.0f;
    g_cfg.offset = 0.0f;
    g_cfg.samplingRateHz = 20.0f;
    g_cfg.uncertainty = 0.05f;
  }

  g_samplePeriodMs = (unsigned long)(1000.0f / max(1.0f, g_cfg.samplingRateHz));

  Serial.println("==== SensorML Config ====");
  Serial.print("id: "); Serial.println(g_cfg.identifier);
  Serial.print("uom: "); Serial.println(g_cfg.uom);
  Serial.print("scale: "); Serial.println(g_cfg.scale, 6);
  Serial.print("offset: "); Serial.println(g_cfg.offset, 6);
  Serial.print("samplingRateHz: "); Serial.println(g_cfg.samplingRateHz, 2);
  Serial.print("uncertainty: "); Serial.println(g_cfg.uncertainty, 4);
  Serial.print("samplePeriodMs: "); Serial.println(g_samplePeriodMs);

  // Init window buffer
  initWindow(g_win, 40);  // 40 samples @ 20Hz ≈ 2 seconds

  // Init TinyML (TFLM)
  TinyMLStatus st = g_ml.begin();
  Serial.print("[TinyML] begin(): "); Serial.println((int)st);

  // Init controller (safety + actuator)
  g_ctrl.begin(PIN_LED);
}

void loop()
{
  const unsigned long now = millis();
  if (now - g_lastSampleMs < g_samplePeriodMs) return;
  g_lastSampleMs = now;

  // 1) read sensor
  float x = readLdrAdc();

  // 2) apply calibration from SensorML
  float xCal = (x * g_cfg.scale) + g_cfg.offset;

  // 3) push into window
  pushSample(g_win, xCal);

  // 4) once window full -> features -> inference -> safety -> actuate
  if (isWindowFull(g_win)) {
    Features f;
    computeFeatures(g_win, f);

    // inference
    InferenceResult r;
    if (g_ml.isReady()) {
      r = g_ml.infer(f);
    } else {
      // fallback if no valid model is loaded
      r = fallbackClassify(f);
    }

    // safety + actuation
    ControlAction a = g_ctrl.safetyAndActuate(r, g_cfg.uncertainty);

    // log
    Serial.print("mean="); Serial.print(f.mean, 2);
    Serial.print(" std="); Serial.print(f.std, 2);
    Serial.print(" slope="); Serial.print(f.slope, 4);
    Serial.print(" | class="); Serial.print(r.label);
    Serial.print(" conf="); Serial.print(r.confidence, 3);
    Serial.print(" | action="); Serial.println((int)a);

    // slide window (hop size)
    popOldest(g_win, 10); // hop 10 samples
  }
}
