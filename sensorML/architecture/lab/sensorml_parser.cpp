#include "sensorml_parser.h"

// Helper: extract text between <tag> and </tag>
static bool extractTag(const String& s, const char* tag, String& out)
{
  String open = String("<") + tag + ">";
  String close = String("</") + tag + ">";
  int i = s.indexOf(open);
  if (i < 0) return false;
  i += open.length();
  int j = s.indexOf(close, i);
  if (j < 0) return false;
  out = s.substring(i, j);
  out.trim();
  return true;
}

bool parseSensorMLFromProgmem(const char* xmlProgmem, SensorConfig& out)
{
  // Read PROGMEM into String
  String xml;
  xml.reserve(1024);
  for (size_t k = 0;; k++) {
    char c = pgm_read_byte(xmlProgmem + k);
    if (c == 0) break;
    xml += c;
  }

  bool ok = true;

  String v;

  if (extractTag(xml, "identifier", v)) out.identifier = v; else ok = false;
  if (extractTag(xml, "uom", v)) out.uom = v; else out.uom = "adc_counts";

  if (extractTag(xml, "samplingRateHz", v)) out.samplingRateHz = v.toFloat();
  else { out.samplingRateHz = 20.0f; ok = false; }

  if (extractTag(xml, "scale", v)) out.scale = v.toFloat();
  else { out.scale = 1.0f; ok = false; }

  if (extractTag(xml, "offset", v)) out.offset = v.toFloat();
  else { out.offset = 0.0f; ok = false; }

  if (extractTag(xml, "uncertainty", v)) out.uncertainty = v.toFloat();
  else out.uncertainty = 0.05f;

  return ok;
}
