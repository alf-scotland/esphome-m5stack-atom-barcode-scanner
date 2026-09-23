#include "types.h"

#include <cstddef>

namespace esphome {
namespace m5stack_barcode {

// Option keys, indexed by enum value.  Must match the option dicts in __init__.py
// (checked by tests/test_enum_consistency.py).
static const char *const OPERATION_MODE_OPTIONS[] = {"host", "level", "pulse", "continuous", "auto_sense"};
static const char *const TERMINATOR_OPTIONS[] = {"none", "crlf", "cr", "tab", "crcr", "crlfcrlf"};
static const char *const LIGHT_MODE_OPTIONS[] = {"on_when_reading", "always_on", "always_off"};
static const char *const BUZZER_VOLUME_OPTIONS[] = {"high", "medium", "low"};
static const char *const SCAN_DURATION_OPTIONS[] = {"500ms", "1s", "3s", "5s", "10s", "15s", "20s", "unlimited"};
static const char *const STABLE_INDUCTION_TIME_OPTIONS[] = {"0ms", "100ms", "300ms", "500ms", "1s"};
static const char *const INTERVAL_OPTIONS[] = {"0ms", "100ms", "300ms", "500ms", "1s", "1.5s", "2s"};

template<typename E, size_t N>
static bool parse_option(const std::string &str, const char *const (&options)[N], E &out) {
  for (size_t i = 0; i < N; i++) {
    if (str == options[i]) {
      out = static_cast<E>(i);
      return true;
    }
  }
  return false;
}

template<typename T, size_t N> static T lookup(const T (&table)[N], size_t index, T fallback) {
  return index < N ? table[index] : fallback;
}

const char *operation_mode_to_string(OperationMode mode) {
  static const char *const NAMES[] = {"Host", "Level Trigger", "Pulse Trigger", "Continuous", "Auto-Sense"};
  return lookup(NAMES, static_cast<size_t>(mode), "Unknown");
}

const char *terminator_to_string(Terminator term) {
  static const char *const NAMES[] = {"None", "CR LF", "CR", "TAB", "CR CR", "CR LF CR LF"};
  return lookup(NAMES, static_cast<size_t>(term), "Unknown");
}

const char *light_mode_to_string(LightMode mode) {
  static const char *const NAMES[] = {"On when reading", "Always on", "Always off"};
  return lookup(NAMES, static_cast<size_t>(mode), "Unknown");
}

const char *buzzer_volume_to_string(BuzzerVolume volume) {
  static const char *const NAMES[] = {"High", "Medium", "Low"};
  return lookup(NAMES, static_cast<size_t>(volume), "Unknown");
}

const char *scan_duration_to_string(ScanDuration duration) {
  static const char *const NAMES[] = {"500 ms", "1 s", "3 s", "5 s", "10 s", "15 s", "20 s", "Unlimited"};
  return lookup(NAMES, static_cast<size_t>(duration), "Unknown");
}

const char *interval_to_string(uint8_t index) {
  static const char *const NAMES[] = {"0 ms", "100 ms", "300 ms", "500 ms", "1 s", "1.5 s", "2 s"};
  return lookup(NAMES, index, "Unknown");
}

const char *enabled_to_string(bool enabled) { return enabled ? "Enabled" : "Disabled"; }

const char *scan_state_to_string(ScanState state) {
  static const char *const NAMES[] = {"Idle", "Manual Scanning", "Continuous Scanning"};
  return lookup(NAMES, static_cast<size_t>(state), "Unknown");
}

const char *terminator_to_bytes(Terminator term) {
  static const char *const BYTES[] = {"", "\r\n", "\r", "\t", "\r\r", "\r\n\r\n"};
  return lookup(BYTES, static_cast<size_t>(term), "");
}

uint32_t scan_duration_to_ms(ScanDuration duration) {
  static const uint32_t MS[] = {500, 1000, 3000, 5000, 10000, 15000, 20000, 0};
  return lookup(MS, static_cast<size_t>(duration), static_cast<uint32_t>(0));
}

bool parse_operation_mode(const std::string &str, OperationMode &out) {
  return parse_option(str, OPERATION_MODE_OPTIONS, out);
}
bool parse_terminator(const std::string &str, Terminator &out) { return parse_option(str, TERMINATOR_OPTIONS, out); }
bool parse_light_mode(const std::string &str, LightMode &out) { return parse_option(str, LIGHT_MODE_OPTIONS, out); }
bool parse_locate_light_mode(const std::string &str, LocateLightMode &out) {
  return parse_option(str, LIGHT_MODE_OPTIONS, out);
}
bool parse_buzzer_volume(const std::string &str, BuzzerVolume &out) {
  return parse_option(str, BUZZER_VOLUME_OPTIONS, out);
}
bool parse_scan_duration(const std::string &str, ScanDuration &out) {
  return parse_option(str, SCAN_DURATION_OPTIONS, out);
}
bool parse_stable_induction_time(const std::string &str, StableInductionTime &out) {
  return parse_option(str, STABLE_INDUCTION_TIME_OPTIONS, out);
}
bool parse_reading_interval(const std::string &str, ReadingInterval &out) {
  return parse_option(str, INTERVAL_OPTIONS, out);
}
bool parse_same_code_interval(const std::string &str, SameCodeInterval &out) {
  return parse_option(str, INTERVAL_OPTIONS, out);
}

}  // namespace m5stack_barcode
}  // namespace esphome
