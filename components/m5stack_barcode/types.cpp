#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// String conversion helpers for logging
const char* operation_mode_to_string(OperationMode mode) {
  switch (mode) {
    case OperationMode::HOST:
      return "Host Mode";
    case OperationMode::LEVEL:
      return "Level Trigger Mode";
    case OperationMode::PULSE:
      return "Pulse Trigger Mode";
    case OperationMode::CONTINUOUS:
      return "Continuous Mode";
    case OperationMode::AUTO_SENSE:
      return "Auto-Sense Mode";
    default:
      return "Unknown Mode";
  }
}

const char* terminator_to_string(Terminator term) {
  switch (term) {
    case Terminator::NONE:
      return "None";
    case Terminator::CRLF:
      return "CR LF";
    case Terminator::CR:
      return "CR";
    case Terminator::TAB:
      return "TAB";
    case Terminator::CRCR:
      return "CR CR";
    case Terminator::CRLFCRLF:
      return "CR LF CR LF";
    default:
      return "Unknown Terminator";
  }
}

const char* light_mode_to_string(LightMode mode) {
  switch (mode) {
    case LightMode::ON_WHEN_READING:
      return "On When Reading";
    case LightMode::ALWAYS_ON:
      return "Always On";
    case LightMode::ALWAYS_OFF:
      return "Always Off";
    default:
      return "Unknown Light Mode";
  }
}

const char* locate_light_mode_to_string(LocateLightMode mode) {
  switch (mode) {
    case LocateLightMode::ON_WHEN_READING:
      return "On When Reading";
    case LocateLightMode::ALWAYS_ON:
      return "Always On";
    case LocateLightMode::ALWAYS_OFF:
      return "Always Off";
    default:
      return "Unknown Locate Light Mode";
  }
}

const char* sound_mode_to_string(SoundMode mode) {
  switch (mode) {
    case SoundMode::SOUND_DISABLED:
      return "Disabled";
    case SoundMode::ENABLED:
      return "Enabled";
    default:
      return "Unknown Sound Mode";
  }
}

const char* buzzer_volume_to_string(BuzzerVolume volume) {
  switch (volume) {
    case BuzzerVolume::VOLUME_HIGH:
      return "High";
    case BuzzerVolume::MEDIUM:
      return "Medium";
    case BuzzerVolume::VOLUME_LOW:
      return "Low";
    default:
      return "Unknown Volume";
  }
}

}  // namespace m5stack_barcode
}  // namespace esphome 