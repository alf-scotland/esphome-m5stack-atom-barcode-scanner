#include "types.h"

namespace esphome {
namespace m5stack_barcode {

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

auto cmd_ack_sound_mode_to_string(CmdAckSoundMode mode) -> const char * {
  switch (mode) {
    case CmdAckSoundMode::CMD_ACK_SOUND_ENABLED:
      return "Enabled";
    case CmdAckSoundMode::CMD_ACK_SOUND_DISABLED:
      return "Disabled";
    default:
      return "Unknown Command ACK Sound Mode";
  }
}

auto config_code_scan_mode_to_string(ConfigCodeScanMode mode) -> const char * {
  switch (mode) {
    case ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED:
      return "Enabled";
    case ConfigCodeScanMode::CONFIG_CODE_SCAN_DISABLED:
      return "Disabled";
    default:
      return "Unknown Config Code Scan Mode";
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

// String-to-enum parse helpers (used by actions for runtime string values from selects)
auto parse_operation_mode(const std::string &str, OperationMode &out) -> bool {
  if (str == "host") {
    out = OperationMode::HOST;
  } else if (str == "level") {
    out = OperationMode::LEVEL;
  } else if (str == "pulse") {
    out = OperationMode::PULSE;
  } else if (str == "continuous") {
    out = OperationMode::CONTINUOUS;
  } else if (str == "auto_sense") {
    out = OperationMode::AUTO_SENSE;
  } else {
    return false;
  }
  return true;
}

auto parse_terminator(const std::string &str, Terminator &out) -> bool {
  if (str == "none") {
    out = Terminator::NONE;
  } else if (str == "crlf") {
    out = Terminator::CRLF;
  } else if (str == "cr") {
    out = Terminator::CR;
  } else if (str == "tab") {
    out = Terminator::TAB;
  } else if (str == "crcr") {
    out = Terminator::CRCR;
  } else if (str == "crlfcrlf") {
    out = Terminator::CRLFCRLF;
  } else {
    return false;
  }
  return true;
}

auto parse_light_mode(const std::string &str, LightMode &out) -> bool {
  if (str == "on_when_reading") {
    out = LightMode::LIGHT_ON_WHEN_READING;
  } else if (str == "always_on") {
    out = LightMode::LIGHT_ALWAYS_ON;
  } else if (str == "always_off") {
    out = LightMode::LIGHT_ALWAYS_OFF;
  } else {
    return false;
  }
  return true;
}

auto parse_locate_light_mode(const std::string &str, LocateLightMode &out) -> bool {
  if (str == "on_when_reading") {
    out = LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING;
  } else if (str == "always_on") {
    out = LocateLightMode::LOCATE_LIGHT_ALWAYS_ON;
  } else if (str == "always_off") {
    out = LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF;
  } else {
    return false;
  }
  return true;
}

auto parse_sound_mode(const std::string &str, SoundMode &out) -> bool {
  if (str == "enabled") {
    out = SoundMode::SOUND_ENABLED;
  } else if (str == "disabled") {
    out = SoundMode::SOUND_DISABLED;
  } else {
    return false;
  }
  return true;
}

auto parse_buzzer_volume(const std::string &str, BuzzerVolume &out) -> bool {
  if (str == "high") {
    out = BuzzerVolume::BUZZER_VOLUME_HIGH;
  } else if (str == "medium") {
    out = BuzzerVolume::BUZZER_VOLUME_MEDIUM;
  } else if (str == "low") {
    out = BuzzerVolume::BUZZER_VOLUME_LOW;
  } else {
    return false;
  }
  return true;
}

auto parse_decoding_success_light_mode(const std::string &str, DecodingSuccessLightMode &out) -> bool {
  if (str == "enabled") {
    out = DecodingSuccessLightMode::DECODING_LIGHT_ENABLED;
  } else if (str == "disabled") {
    out = DecodingSuccessLightMode::DECODING_LIGHT_DISABLED;
  } else {
    return false;
  }
  return true;
}

auto parse_boot_sound_mode(const std::string &str, BootSoundMode &out) -> bool {
  if (str == "enabled") {
    out = BootSoundMode::BOOT_SOUND_ENABLED;
  } else if (str == "disabled") {
    out = BootSoundMode::BOOT_SOUND_DISABLED;
  } else {
    return false;
  }
  return true;
}

auto parse_decode_sound_mode(const std::string &str, DecodeSoundMode &out) -> bool {
  if (str == "enabled") {
    out = DecodeSoundMode::DECODE_SOUND_ENABLED;
  } else if (str == "disabled") {
    out = DecodeSoundMode::DECODE_SOUND_DISABLED;
  } else {
    return false;
  }
  return true;
}

auto parse_scan_duration(const std::string &str, ScanDuration &out) -> bool {
  if (str == "500ms") {
    out = ScanDuration::MS_500;
  } else if (str == "1s") {
    out = ScanDuration::MS_1000;
  } else if (str == "3s") {
    out = ScanDuration::MS_3000;
  } else if (str == "5s") {
    out = ScanDuration::MS_5000;
  } else if (str == "10s") {
    out = ScanDuration::MS_10000;
  } else if (str == "15s") {
    out = ScanDuration::MS_15000;
  } else if (str == "20s") {
    out = ScanDuration::MS_20000;
  } else if (str == "unlimited") {
    out = ScanDuration::UNLIMITED;
  } else {
    return false;
  }
  return true;
}

auto parse_stable_induction_time(const std::string &str, StableInductionTime &out) -> bool {
  if (str == "0ms") {
    out = StableInductionTime::MS_0;
  } else if (str == "100ms") {
    out = StableInductionTime::MS_100;
  } else if (str == "300ms") {
    out = StableInductionTime::MS_300;
  } else if (str == "500ms") {
    out = StableInductionTime::MS_500;
  } else if (str == "1s") {
    out = StableInductionTime::MS_1000;
  } else {
    return false;
  }
  return true;
}

auto parse_reading_interval(const std::string &str, ReadingInterval &out) -> bool {
  if (str == "0ms") {
    out = ReadingInterval::MS_0;
  } else if (str == "100ms") {
    out = ReadingInterval::MS_100;
  } else if (str == "300ms") {
    out = ReadingInterval::MS_300;
  } else if (str == "500ms") {
    out = ReadingInterval::MS_500;
  } else if (str == "1s") {
    out = ReadingInterval::MS_1000;
  } else if (str == "1.5s") {
    out = ReadingInterval::MS_1500;
  } else if (str == "2s") {
    out = ReadingInterval::MS_2000;
  } else {
    return false;
  }
  return true;
}

auto parse_same_code_interval(const std::string &str, SameCodeInterval &out) -> bool {
  if (str == "0ms") {
    out = SameCodeInterval::MS_0;
  } else if (str == "100ms") {
    out = SameCodeInterval::MS_100;
  } else if (str == "300ms") {
    out = SameCodeInterval::MS_300;
  } else if (str == "500ms") {
    out = SameCodeInterval::MS_500;
  } else if (str == "1s") {
    out = SameCodeInterval::MS_1000;
  } else if (str == "1.5s") {
    out = SameCodeInterval::MS_1500;
  } else if (str == "2s") {
    out = SameCodeInterval::MS_2000;
  } else {
    return false;
  }
  return true;
}

}  // namespace m5stack_barcode
}  // namespace esphome
