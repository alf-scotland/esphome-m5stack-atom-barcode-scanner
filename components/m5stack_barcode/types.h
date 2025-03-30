#pragma once

#include <cstdint>  // For uint8_t

namespace esphome::m5stack_barcode {

// Scanner operation modes
enum class OperationMode {
  HOST,        // Host mode - requires explicit start/stop commands
  LEVEL,       // Level trigger mode
  PULSE,       // Pulse trigger mode
  CONTINUOUS,  // Continuous mode
  AUTO_SENSE   // Auto-sense mode
};

// Terminator settings
enum class Terminator {
  NONE,     // No terminator
  CRLF,     // CR LF (carriage return, line feed)
  CR,       // CR (Enter)
  TAB,      // TAB
  CRCR,     // CR CR
  CRLFCRLF  // CR LF CR LF
};

// Response types for command handling
enum class ResponseType : uint8_t { NONE = 0x00, VERSION = 0x01, BARCODE = 0x02 };

// Command processing states
enum class CommandState : uint8_t {
  IDLE = 0,         // No command in progress
  WAKEUP_SENT = 1,  // Wake-up command (0x00) sent, waiting 50ms before sending actual command
  COMMAND_SENT = 2  // Command sent, waiting for acknowledgment
};

// Light modes
enum class LightMode {
  LIGHT_ON_WHEN_READING,  // Light on only when reading
  LIGHT_ALWAYS_ON,        // Light always on
  LIGHT_ALWAYS_OFF        // Light always off
};

// Locate light control modes
enum class LocateLightMode {
  LOCATE_LIGHT_ON_WHEN_READING,  // Default
  LOCATE_LIGHT_ALWAYS_ON,
  LOCATE_LIGHT_ALWAYS_OFF
};

// Decoding success light modes
enum class DecodingSuccessLightMode {
  DECODING_LIGHT_DISABLED,  // Light disabled on successful decode
  DECODING_LIGHT_ENABLED    // Light enabled on successful decode
};

// Sound modes
enum class SoundMode {
  SOUND_DISABLED,  // Sound disabled
  SOUND_ENABLED    // Sound enabled
};

// Boot sound modes
enum class BootSoundMode {
  BOOT_SOUND_DISABLED,  // Boot sound disabled
  BOOT_SOUND_ENABLED    // Boot sound enabled
};

// Decode sound modes
enum class DecodeSoundMode {
  DECODE_SOUND_DISABLED,  // Decode sound disabled
  DECODE_SOUND_ENABLED    // Decode sound enabled
};

// Buzzer volume settings
enum class BuzzerVolume {
  BUZZER_VOLUME_HIGH,    // High volume
  BUZZER_VOLUME_MEDIUM,  // Medium volume
  BUZZER_VOLUME_LOW      // Low volume
};

// Scan duration settings
enum class ScanDuration {
  MS_500,    // 500ms
  MS_1000,   // 1s
  MS_3000,   // 3s
  MS_5000,   // 5s
  MS_10000,  // 10s
  MS_15000,  // 15s
  MS_20000,  // 20s
  UNLIMITED  // Unlimited duration
};

// Stable induction time settings
enum class StableInductionTime {
  MS_0,    // 0ms
  MS_100,  // 100ms
  MS_300,  // 300ms
  MS_500,  // 500ms
  MS_1000  // 1s
};

// Reading interval settings
enum class ReadingInterval {
  MS_0,     // 0ms
  MS_100,   // 100ms
  MS_300,   // 300ms
  MS_500,   // 500ms
  MS_1000,  // 1s
  MS_1500,  // 1.5s
  MS_2000   // 2s
};

// Same code interval settings
enum class SameCodeInterval {
  MS_0,     // 0ms
  MS_100,   // 100ms
  MS_300,   // 300ms
  MS_500,   // 500ms
  MS_1000,  // 1s
  MS_1500,  // 1.5s
  MS_2000   // 2s
};

// Scanning state to track different states of the scanner
enum class ScanState {
  IDLE,                // Not scanning
  MANUAL_SCANNING,     // Manual scanning (host mode or hardware trigger)
  CONTINUOUS_SCANNING  // Continuous scanning (continuous or auto-sense mode)
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
auto scan_state_to_string(ScanState state) -> const char *;

}  // namespace esphome::m5stack_barcode
