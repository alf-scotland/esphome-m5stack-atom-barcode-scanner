#pragma once

#include <cstdint>

namespace esphome {
namespace m5stack_barcode {

// Scanner operation modes
enum class OperationMode : uint8_t {
  HOST = 0x08,      // Host mode - requires explicit start/stop commands
  LEVEL = 0x00,     // Level trigger mode
  PULSE = 0x02,     // Pulse trigger mode
  CONTINUOUS = 0x04, // Continuous mode
  AUTO_SENSE = 0x09 // Auto-sense mode
};

// Terminator settings
enum class Terminator : uint8_t {
  NONE = 0x00,    // No terminator (prohibit)
  CRLF = 0x01,    // CR LF (carriage return, line feed)
  CR = 0x02,      // CR (Enter)
  TAB = 0x03,     // TAB
  CRCR = 0x04,    // CR CR
  CRLFCRLF = 0x05 // CR LF CR LF
};

// Response types for command handling
enum class ResponseType : uint8_t {
  NONE = 0x00,
  VERSION = 0x01,
  BARCODE = 0x02
};

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

// Sound settings
enum class SoundMode : uint8_t {
  SOUND_DISABLED = 0x00,  // Default
  ENABLED = 0x01
};

// Buzzer volume levels
enum class BuzzerVolume : uint8_t {
  VOLUME_HIGH = 0x00,  // Default
  MEDIUM = 0x01,
  VOLUME_LOW = 0x02
};

// String conversion helpers for logging
const char* operation_mode_to_string(OperationMode mode);
const char* terminator_to_string(Terminator term);
const char* light_mode_to_string(LightMode mode);
const char* locate_light_mode_to_string(LocateLightMode mode);
const char* sound_mode_to_string(SoundMode mode);
const char* buzzer_volume_to_string(BuzzerVolume volume);

}  // namespace m5stack_barcode
}  // namespace esphome 