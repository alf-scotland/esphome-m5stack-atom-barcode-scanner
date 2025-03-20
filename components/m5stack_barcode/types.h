#pragma once

#include <cstdint>  // For uint8_t

namespace esphome::m5stack_barcode {

// Scanner operation modes
enum class OperationMode : uint8_t {
  HOST = 0x08,        // Host mode - requires explicit start/stop commands
  LEVEL = 0x00,       // Level trigger mode
  PULSE = 0x02,       // Pulse trigger mode
  CONTINUOUS = 0x04,  // Continuous mode
  AUTO_SENSE = 0x09   // Auto-sense mode
};

// Terminator settings
enum class Terminator : uint8_t {
  NONE = 0x00,     // No terminator (prohibit)
  CRLF = 0x01,     // CR LF (carriage return, line feed)
  CR = 0x02,       // CR (Enter)
  TAB = 0x03,      // TAB
  CRCR = 0x04,     // CR CR
  CRLFCRLF = 0x05  // CR LF CR LF
};

// Response types for command handling
enum class ResponseType : uint8_t { NONE = 0x00, VERSION = 0x01, BARCODE = 0x02 };

// Command processing states
enum class CommandState : uint8_t {
  IDLE = 0,         // No command in progress
  WAKEUP_SENT = 1,  // Wake-up command (0x00) sent, waiting 50ms before sending actual command
  COMMAND_SENT = 2  // Command sent, waiting for acknowledgment
};

// Lighting control modes
enum class LightMode : uint8_t {
  ON_WHEN_READING = 0x00,  // Default
  ALWAYS_ON = 0x01,
  ALWAYS_OFF = 0x02
};

// Locate light control modes
enum class LocateLightMode : uint8_t {
  ON_WHEN_READING = 0x00,  // Default
  ALWAYS_ON = 0x01,
  ALWAYS_OFF = 0x02
};

// Handle potential conflicts with Arduino's DISABLED macro
#ifdef DISABLED
#define DISABLED_TEMP DISABLED
#undef DISABLED
#endif

// Decoding success light control modes
enum class DecodingSuccessLightMode : uint8_t {
  LIGHT_ENABLED = 0x01,  // Default
  LIGHT_DISABLED = 0x00
};

// Sound settings
enum class SoundMode : uint8_t {
  SOUND_DISABLED = 0x00,  // Default
  SOUND_ENABLED = 0x01
};

// Boot sound settings
enum class BootSoundMode : uint8_t {
  BOOT_SOUND_ENABLED = 0x01,  // Default
  BOOT_SOUND_DISABLED = 0x00
};

// Decode sound settings
enum class DecodeSoundMode : uint8_t {
  DECODE_SOUND_ENABLED = 0x01,  // Default
  DECODE_SOUND_DISABLED = 0x00
};

// Restore Arduino's DISABLED macro if it was defined
#ifdef DISABLED_TEMP
#define DISABLED DISABLED_TEMP
#undef DISABLED_TEMP
#endif

// Buzzer volume levels
enum class BuzzerVolume : uint8_t {
  VOLUME_HIGH = 0x00,  // Default
  VOLUME_MEDIUM = 0x01,
  VOLUME_LOW = 0x02
};

// Scan duration settings
enum class ScanDuration : uint8_t {
  MS_500 = 0x05,
  MS_1000 = 0x0A,
  MS_3000 = 0x1E,  // Default
  MS_5000 = 0x32,
  MS_10000 = 0x64,
  MS_15000 = 0x96,
  MS_20000 = 0xC8,
  UNLIMITED = 0x00
};

// Stable induction time settings
enum class StableInductionTime : uint8_t {
  MS_0 = 0x00,
  MS_100 = 0x01,
  MS_300 = 0x03,
  MS_500 = 0x05,  // Default
  MS_1000 = 0x0A
};

// Reading interval settings
enum class ReadingInterval : uint8_t {
  MS_0 = 0x00,
  MS_100 = 0x01,
  MS_300 = 0x03,
  MS_500 = 0x05,  // Default
  MS_1000 = 0x0A,
  MS_1500 = 0x0F,
  MS_2000 = 0x14
};

// Same code output interval settings
enum class SameCodeInterval : uint8_t {
  MS_0 = 0x00,
  MS_100 = 0x01,
  MS_300 = 0x03,
  MS_500 = 0x05,  // Default
  MS_1000 = 0x0A,
  MS_1500 = 0x0F,
  MS_2000 = 0x14
};

// String conversion helpers for logging
auto operation_mode_to_string(OperationMode mode) -> const char *;
auto terminator_to_string(Terminator term) -> const char *;
auto light_mode_to_string(LightMode mode) -> const char *;
auto locate_light_mode_to_string(LocateLightMode mode) -> const char *;
auto decoding_success_light_mode_to_string(DecodingSuccessLightMode mode) -> const char *;
auto sound_mode_to_string(SoundMode mode) -> const char *;
auto boot_sound_mode_to_string(BootSoundMode mode) -> const char *;
auto decode_sound_mode_to_string(DecodeSoundMode mode) -> const char *;
auto buzzer_volume_to_string(BuzzerVolume volume) -> const char *;
auto scan_duration_to_string(ScanDuration duration) -> const char *;
auto stable_induction_time_to_string(StableInductionTime time) -> const char *;
auto reading_interval_to_string(ReadingInterval interval) -> const char *;
auto same_code_interval_to_string(SameCodeInterval interval) -> const char *;

}  // namespace esphome::m5stack_barcode
