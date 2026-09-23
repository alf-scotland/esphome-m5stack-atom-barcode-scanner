#pragma once

#include <cstdint>
#include <string>

namespace esphome {
namespace m5stack_barcode {

// Scanner setting enums.
//
// The enumerator order is load-bearing:
//  - it is the option order of the matching HA select (option index == enum value), and the
//    option lists in __init__.py must list the keys in the same order (checked by
//    tests/test_enum_consistency.py);
//  - values are persisted to NVS in ScannerPreferences, so reordering requires a
//    SETTINGS_VERSION bump;
//  - two-state settings map an HA switch's bool directly onto the enum, so DISABLED must be 0.

enum class OperationMode : uint8_t {
  HOST,        // Scanner waits for explicit start/stop decoding commands
  LEVEL,       // Level trigger mode (scan while the trigger is held)
  PULSE,       // Pulse trigger mode (single scan per trigger)
  CONTINUOUS,  // Continuous scanning
  AUTO_SENSE,  // Scan when an object is detected
};

enum class Terminator : uint8_t {
  NONE,
  CRLF,
  CR,
  TAB,
  CRCR,
  CRLFCRLF,
};

enum class LightMode : uint8_t {
  LIGHT_ON_WHEN_READING,
  LIGHT_ALWAYS_ON,
  LIGHT_ALWAYS_OFF,
};

enum class LocateLightMode : uint8_t {
  LOCATE_LIGHT_ON_WHEN_READING,
  LOCATE_LIGHT_ALWAYS_ON,
  LOCATE_LIGHT_ALWAYS_OFF,
};

enum class DecodingSuccessLightMode : uint8_t {
  DECODING_LIGHT_DISABLED = 0,
  DECODING_LIGHT_ENABLED = 1,
};

// Global beeper mute (PDF item 11)
enum class SoundMode : uint8_t {
  SOUND_DISABLED = 0,
  SOUND_ENABLED = 1,
};

enum class BootSoundMode : uint8_t {
  BOOT_SOUND_DISABLED = 0,
  BOOT_SOUND_ENABLED = 1,
};

enum class DecodeSoundMode : uint8_t {
  DECODE_SOUND_DISABLED = 0,
  DECODE_SOUND_ENABLED = 1,
};

// Beep when a configuration command is ACKed (PDF item 14)
enum class CmdAckSoundMode : uint8_t {
  CMD_ACK_SOUND_DISABLED = 0,
  CMD_ACK_SOUND_ENABLED = 1,
};

// Whether scanning a configuration barcode may reconfigure the scanner (PDF item 21)
enum class ConfigCodeScanMode : uint8_t {
  CONFIG_CODE_SCAN_DISABLED = 0,
  CONFIG_CODE_SCAN_ENABLED = 1,
};

enum class BuzzerVolume : uint8_t {
  BUZZER_VOLUME_HIGH,
  BUZZER_VOLUME_MEDIUM,
  BUZZER_VOLUME_LOW,
};

enum class ScanDuration : uint8_t {
  MS_500,
  MS_1000,
  MS_3000,
  MS_5000,
  MS_10000,
  MS_15000,
  MS_20000,
  UNLIMITED,
};

enum class StableInductionTime : uint8_t {
  MS_0,
  MS_100,
  MS_300,
  MS_500,
  MS_1000,
};

enum class ReadingInterval : uint8_t {
  MS_0,
  MS_100,
  MS_300,
  MS_500,
  MS_1000,
  MS_1500,
  MS_2000,
};

enum class SameCodeInterval : uint8_t {
  MS_0,
  MS_100,
  MS_300,
  MS_500,
  MS_1000,
  MS_1500,
  MS_2000,
};

// Response the scanner sends for a command other than the standard ACK
enum class ResponseType : uint8_t { NONE, VERSION };

// Command processing states
enum class CommandState : uint8_t {
  IDLE,         // No command in progress
  WAKEUP_SENT,  // Wake-up byte (0x00) sent, waiting WAKEUP_DELAY_MS before sending the command
  COMMAND_SENT  // Command sent, waiting for its ACK or response
};

enum class ScanState : uint8_t {
  IDLE,                // Not scanning
  MANUAL_SCANNING,     // HOST-mode scan started by start_scan()
  CONTINUOUS_SCANNING  // Continuous or auto-sense mode
};

// Human-readable names for logging
const char *operation_mode_to_string(OperationMode mode);
const char *terminator_to_string(Terminator term);
const char *light_mode_to_string(LightMode mode);
const char *buzzer_volume_to_string(BuzzerVolume volume);
const char *scan_duration_to_string(ScanDuration duration);
const char *interval_to_string(uint8_t index);  // Shared by the 0 ms … 2 s timing settings
const char *enabled_to_string(bool enabled);
const char *scan_state_to_string(ScanState state);

// Bytes the scanner appends to barcode output for a terminator setting ("" for NONE)
const char *terminator_to_bytes(Terminator term);

// Scan duration in milliseconds; 0 for UNLIMITED
uint32_t scan_duration_to_ms(ScanDuration duration);

// String-to-enum parsers for action values; the strings are the YAML/select option keys.
// Return false (leaving out untouched) if the string is not a valid option.
bool parse_operation_mode(const std::string &str, OperationMode &out);
bool parse_terminator(const std::string &str, Terminator &out);
bool parse_light_mode(const std::string &str, LightMode &out);
bool parse_locate_light_mode(const std::string &str, LocateLightMode &out);
bool parse_buzzer_volume(const std::string &str, BuzzerVolume &out);
bool parse_scan_duration(const std::string &str, ScanDuration &out);
bool parse_stable_induction_time(const std::string &str, StableInductionTime &out);
bool parse_reading_interval(const std::string &str, ReadingInterval &out);
bool parse_same_code_interval(const std::string &str, SameCodeInterval &out);
template<typename E> bool parse_enabled(const std::string &str, E &out) {
  if (str != "enabled" && str != "disabled")
    return false;
  out = static_cast<E>(str == "enabled");
  return true;
}

}  // namespace m5stack_barcode
}  // namespace esphome
