#include "types.h"
namespace esphome::m5stack_barcode {

// String conversion helpers for logging
auto operation_mode_to_string(OperationMode mode) -> const char * {
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

auto terminator_to_string(Terminator term) -> const char * {
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

auto light_mode_to_string(LightMode mode) -> const char * {
  switch (mode) {
    case LightMode::LIGHT_ON_WHEN_READING:
      return "On when reading";
    case LightMode::LIGHT_ALWAYS_ON:
      return "Always on";
    case LightMode::LIGHT_ALWAYS_OFF:
      return "Always off";
    default:
      return "Unknown Light Mode";
  }
}

auto locate_light_mode_to_string(LocateLightMode mode) -> const char * {
  switch (mode) {
    case LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING:
      return "On when reading";
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_ON:
      return "Always on";
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF:
      return "Always off";
    default:
      return "Unknown Locate Light Mode";
  }
}

auto decoding_success_light_mode_to_string(DecodingSuccessLightMode mode) -> const char * {
  switch (mode) {
    case DecodingSuccessLightMode::DECODING_LIGHT_ENABLED:
      return "Enabled";
    case DecodingSuccessLightMode::DECODING_LIGHT_DISABLED:
      return "Disabled";
    default:
      return "Unknown Decoding Success Light Mode";
  }
}

auto sound_mode_to_string(SoundMode mode) -> const char * {
  switch (mode) {
    case SoundMode::SOUND_DISABLED:
      return "Disabled";
    case SoundMode::SOUND_ENABLED:
      return "Enabled";
    default:
      return "Unknown Sound Mode";
  }
}

auto boot_sound_mode_to_string(BootSoundMode mode) -> const char * {
  switch (mode) {
    case BootSoundMode::BOOT_SOUND_ENABLED:
      return "Enabled";
    case BootSoundMode::BOOT_SOUND_DISABLED:
      return "Disabled";
    default:
      return "Unknown Boot Sound Mode";
  }
}

auto decode_sound_mode_to_string(DecodeSoundMode mode) -> const char * {
  switch (mode) {
    case DecodeSoundMode::DECODE_SOUND_ENABLED:
      return "Enabled";
    case DecodeSoundMode::DECODE_SOUND_DISABLED:
      return "Disabled";
    default:
      return "Unknown Decode Sound Mode";
  }
}

auto buzzer_volume_to_string(BuzzerVolume volume) -> const char * {
  switch (volume) {
    case BuzzerVolume::BUZZER_VOLUME_HIGH:
      return "High";
    case BuzzerVolume::BUZZER_VOLUME_MEDIUM:
      return "Medium";
    case BuzzerVolume::BUZZER_VOLUME_LOW:
      return "Low";
    default:
      return "Unknown Buzzer Volume";
  }
}

auto scan_duration_to_string(ScanDuration duration) -> const char * {
  switch (duration) {
    case ScanDuration::MS_500:
      return "500 ms";
    case ScanDuration::MS_1000:
      return "1 second";
    case ScanDuration::MS_3000:
      return "3 seconds";
    case ScanDuration::MS_5000:
      return "5 seconds";
    case ScanDuration::MS_10000:
      return "10 seconds";
    case ScanDuration::MS_15000:
      return "15 seconds";
    case ScanDuration::MS_20000:
      return "20 seconds";
    case ScanDuration::UNLIMITED:
      return "Unlimited";
    default:
      return "Unknown Duration";
  }
}

auto stable_induction_time_to_string(StableInductionTime time) -> const char * {
  switch (time) {
    case StableInductionTime::MS_0:
      return "0 ms";
    case StableInductionTime::MS_100:
      return "100 ms";
    case StableInductionTime::MS_300:
      return "300 ms";
    case StableInductionTime::MS_500:
      return "500 ms";
    case StableInductionTime::MS_1000:
      return "1 second";
    default:
      return "Unknown Induction Time";
  }
}

auto reading_interval_to_string(ReadingInterval interval) -> const char * {
  switch (interval) {
    case ReadingInterval::MS_0:
      return "0 ms";
    case ReadingInterval::MS_100:
      return "100 ms";
    case ReadingInterval::MS_300:
      return "300 ms";
    case ReadingInterval::MS_500:
      return "500 ms";
    case ReadingInterval::MS_1000:
      return "1 second";
    case ReadingInterval::MS_1500:
      return "1.5 seconds";
    case ReadingInterval::MS_2000:
      return "2 seconds";
    default:
      return "Unknown Reading Interval";
  }
}

auto same_code_interval_to_string(SameCodeInterval interval) -> const char * {
  switch (interval) {
    case SameCodeInterval::MS_0:
      return "0 ms";
    case SameCodeInterval::MS_100:
      return "100 ms";
    case SameCodeInterval::MS_300:
      return "300 ms";
    case SameCodeInterval::MS_500:
      return "500 ms";
    case SameCodeInterval::MS_1000:
      return "1 second";
    case SameCodeInterval::MS_1500:
      return "1.5 seconds";
    case SameCodeInterval::MS_2000:
      return "2 seconds";
    default:
      return "Unknown Same Code Interval";
  }
}

auto scan_state_to_string(ScanState state) -> const char * {
  switch (state) {
    case ScanState::IDLE:
      return "Idle";
    case ScanState::MANUAL_SCANNING:
      return "Manual Scanning";
    case ScanState::CONTINUOUS_SCANNING:
      return "Continuous Scanning";
    default:
      return "Unknown Scan State";
  }
}

auto scan_duration_to_ms(ScanDuration duration) -> uint32_t {
  switch (duration) {
    case ScanDuration::MS_500:
      return 500;
    case ScanDuration::MS_1000:
      return 1000;
    case ScanDuration::MS_3000:
      return 3000;
    case ScanDuration::MS_5000:
      return 5000;
    case ScanDuration::MS_10000:
      return 10000;
    case ScanDuration::MS_15000:
      return 15000;
    case ScanDuration::MS_20000:
      return 20000;
    case ScanDuration::UNLIMITED:
      return 0;
    default:
      return 3000;  // Default to 3 seconds
  }
}

}  // namespace esphome::m5stack_barcode
