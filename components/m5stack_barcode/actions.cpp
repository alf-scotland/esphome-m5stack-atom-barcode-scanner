#include "actions.h"

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "m5stack_barcode.h"

namespace esphome {
namespace m5stack_barcode {

// Tag for logging from actions
const char *const TAG_ACTION = "m5stack_barcode.action";

// Implement the action classes

template<typename... Ts> void StartAction<Ts...>::play(Ts... x) {
  if (this->scanner_ != nullptr) {
    ESP_LOGD(TAG_ACTION, "Executing start scan action");
    this->scanner_->start_scan();
  }
}

template<typename... Ts> void StopAction<Ts...>::play(Ts... x) {
  if (this->scanner_ != nullptr) {
    ESP_LOGD(TAG_ACTION, "Executing stop scan action");
    this->scanner_->stop_scan();
  }
}

template<typename... Ts> void SetModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  OperationMode mode;

  if (mode_str == "host")
    mode = OperationMode::HOST;
  else if (mode_str == "level")
    mode = OperationMode::LEVEL;
  else if (mode_str == "pulse")
    mode = OperationMode::PULSE;
  else if (mode_str == "continuous")
    mode = OperationMode::CONTINUOUS;
  else if (mode_str == "auto_sense")
    mode = OperationMode::AUTO_SENSE;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown operation mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting operation mode to: %s", mode_str.c_str());
  this->scanner_->set_operation_mode(mode);
}

template<typename... Ts> void SetTerminatorAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string term_str = this->terminator_.value(x...);
  Terminator term;

  if (term_str == "none")
    term = Terminator::NONE;
  else if (term_str == "crlf")
    term = Terminator::CRLF;
  else if (term_str == "cr")
    term = Terminator::CR;
  else if (term_str == "tab")
    term = Terminator::TAB;
  else if (term_str == "crcr")
    term = Terminator::CRCR;
  else if (term_str == "crlfcrlf")
    term = Terminator::CRLFCRLF;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown terminator: %s", term_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting terminator to: %s", term_str.c_str());
  this->scanner_->set_terminator(term);
}

template<typename... Ts> void SetLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  LightMode mode;
  if (this->mode_.has_value()) {
    auto val = this->mode_.value(x...);
    if (val == "on_when_reading") {
      mode = LightMode::LIGHT_ON_WHEN_READING;
    } else if (val == "always_on") {
      mode = LightMode::LIGHT_ALWAYS_ON;
    } else if (val == "always_off") {
      mode = LightMode::LIGHT_ALWAYS_OFF;
    } else {
      ESP_LOGW(TAG_ACTION, "Invalid light mode: %s", val.c_str());
      return;
    }
  }
  this->scanner_->set_light_mode(mode);
}

template<typename... Ts> void SetLocateLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  LocateLightMode mode;
  if (this->mode_.has_value()) {
    auto val = this->mode_.value(x...);
    if (val == "on_when_reading") {
      mode = LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING;
    } else if (val == "always_on") {
      mode = LocateLightMode::LOCATE_LIGHT_ALWAYS_ON;
    } else if (val == "always_off") {
      mode = LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF;
    } else {
      ESP_LOGW(TAG_ACTION, "Invalid locate light mode: %s", val.c_str());
      return;
    }
  }
  this->scanner_->set_locate_light_mode(mode);
}

template<typename... Ts> void SetSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  SoundMode mode;

  if (mode_str == "disabled")
    mode = SoundMode::SOUND_DISABLED;
  else if (mode_str == "enabled")
    mode = SoundMode::SOUND_ENABLED;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown sound mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting sound mode to: %s", mode_str.c_str());
  this->scanner_->set_sound_mode(mode);
}

template<typename... Ts> void SetBuzzerVolumeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  BuzzerVolume volume;
  if (this->volume_.has_value()) {
    auto val = this->volume_.value(x...);
    if (val == "high") {
      volume = BuzzerVolume::BUZZER_VOLUME_HIGH;
    } else if (val == "medium") {
      volume = BuzzerVolume::BUZZER_VOLUME_MEDIUM;
    } else if (val == "low") {
      volume = BuzzerVolume::BUZZER_VOLUME_LOW;
    } else {
      ESP_LOGW(TAG_ACTION, "Invalid buzzer volume: %s", val.c_str());
      return;
    }
  }
  this->scanner_->set_buzzer_volume(volume);
}

// Implement new action classes

template<typename... Ts> void SetDecodingSuccessLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  DecodingSuccessLightMode mode;
  if (this->mode_.has_value()) {
    auto val = this->mode_.value(x...);
    if (val == "enabled") {
      mode = DecodingSuccessLightMode::DECODING_LIGHT_ENABLED;
    } else if (val == "disabled") {
      mode = DecodingSuccessLightMode::DECODING_LIGHT_DISABLED;
    } else {
      ESP_LOGW(TAG_ACTION, "Invalid decoding success light mode: %s", val.c_str());
      return;
    }
  }
  this->scanner_->set_decoding_success_light_mode(mode);
}

template<typename... Ts> void SetBootSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  BootSoundMode mode;

  if (mode_str == "enabled")
    mode = BootSoundMode::BOOT_SOUND_ENABLED;
  else if (mode_str == "disabled")
    mode = BootSoundMode::BOOT_SOUND_DISABLED;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown boot sound mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting boot sound mode to: %s", mode_str.c_str());
  this->scanner_->set_boot_sound_mode(mode);
}

template<typename... Ts> void SetDecodeSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  DecodeSoundMode mode;

  if (mode_str == "enabled")
    mode = DecodeSoundMode::DECODE_SOUND_ENABLED;
  else if (mode_str == "disabled")
    mode = DecodeSoundMode::DECODE_SOUND_DISABLED;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown decode sound mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting decode sound mode to: %s", mode_str.c_str());
  this->scanner_->set_decode_sound_mode(mode);
}

template<typename... Ts> void SetScanDurationAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string duration_str = this->duration_.value(x...);
  ScanDuration duration;

  if (duration_str == "500ms")
    duration = ScanDuration::MS_500;
  else if (duration_str == "1s")
    duration = ScanDuration::MS_1000;
  else if (duration_str == "3s")
    duration = ScanDuration::MS_3000;
  else if (duration_str == "5s")
    duration = ScanDuration::MS_5000;
  else if (duration_str == "10s")
    duration = ScanDuration::MS_10000;
  else if (duration_str == "15s")
    duration = ScanDuration::MS_15000;
  else if (duration_str == "20s")
    duration = ScanDuration::MS_20000;
  else if (duration_str == "unlimited")
    duration = ScanDuration::UNLIMITED;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown scan duration: %s", duration_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting scan duration to: %s", duration_str.c_str());
  this->scanner_->set_scan_duration(duration);
}

template<typename... Ts> void SetStableInductionTimeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string time_str = this->time_.value(x...);
  StableInductionTime time;

  if (time_str == "0ms")
    time = StableInductionTime::MS_0;
  else if (time_str == "100ms")
    time = StableInductionTime::MS_100;
  else if (time_str == "300ms")
    time = StableInductionTime::MS_300;
  else if (time_str == "500ms")
    time = StableInductionTime::MS_500;
  else if (time_str == "1s")
    time = StableInductionTime::MS_1000;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown stable induction time: %s", time_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting stable induction time to: %s", time_str.c_str());
  this->scanner_->set_stable_induction_time(time);
}

template<typename... Ts> void SetReadingIntervalAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string interval_str = this->interval_.value(x...);
  ReadingInterval interval;

  if (interval_str == "0ms")
    interval = ReadingInterval::MS_0;
  else if (interval_str == "100ms")
    interval = ReadingInterval::MS_100;
  else if (interval_str == "300ms")
    interval = ReadingInterval::MS_300;
  else if (interval_str == "500ms")
    interval = ReadingInterval::MS_500;
  else if (interval_str == "1s")
    interval = ReadingInterval::MS_1000;
  else if (interval_str == "1.5s")
    interval = ReadingInterval::MS_1500;
  else if (interval_str == "2s")
    interval = ReadingInterval::MS_2000;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown reading interval: %s", interval_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting reading interval to: %s", interval_str.c_str());
  this->scanner_->set_reading_interval(interval);
}

template<typename... Ts> void SetSameCodeIntervalAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string interval_str = this->interval_.value(x...);
  SameCodeInterval interval;

  if (interval_str == "0ms")
    interval = SameCodeInterval::MS_0;
  else if (interval_str == "100ms")
    interval = SameCodeInterval::MS_100;
  else if (interval_str == "300ms")
    interval = SameCodeInterval::MS_300;
  else if (interval_str == "500ms")
    interval = SameCodeInterval::MS_500;
  else if (interval_str == "1s")
    interval = SameCodeInterval::MS_1000;
  else if (interval_str == "1.5s")
    interval = SameCodeInterval::MS_1500;
  else if (interval_str == "2s")
    interval = SameCodeInterval::MS_2000;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown same code interval: %s", interval_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting same code interval to: %s", interval_str.c_str());
  this->scanner_->set_same_code_interval(interval);
}

template<typename... Ts> void ProcessCurrentBufferAction<Ts...>::play(Ts... x) {
  if (this->scanner_ != nullptr) {
    ESP_LOGD(TAG_ACTION, "Processing current buffer");
    this->scanner_->process_current_buffer();
  }
}

template<typename... Ts> bool IsContinuousModeCondition<Ts...>::check(Ts... x) {
  // This method is already defined in the header file
  // Using this empty implementation to avoid linker errors
  return false;
}

// Explicit template instantiations
template class StartAction<>;
template class StartAction<bool>;
template class StartAction<int>;
template class StartAction<std::string>;
template class StartAction<std::string, int>;
template class StopAction<>;
template class SetModeAction<>;
template class SetTerminatorAction<>;
template class SetLightModeAction<>;
template class SetLocateLightModeAction<>;
template class SetSoundModeAction<>;
template class SetBuzzerVolumeAction<>;
template class SetDecodingSuccessLightModeAction<>;
template class SetBootSoundModeAction<>;
template class SetDecodeSoundModeAction<>;
template class SetScanDurationAction<>;
template class SetStableInductionTimeAction<>;
template class SetReadingIntervalAction<>;
template class SetSameCodeIntervalAction<>;
template class ProcessCurrentBufferAction<>;
template class ProcessCurrentBufferAction<bool>;
template class ProcessCurrentBufferAction<int>;
template class ProcessCurrentBufferAction<std::string>;
template class ProcessCurrentBufferAction<std::string, int>;
template class IsContinuousModeCondition<>;
template class IsContinuousModeCondition<bool>;
template class IsContinuousModeCondition<int>;
template class IsContinuousModeCondition<std::string>;
template class IsContinuousModeCondition<std::string, int>;

// Explicit template instantiations for the types used in YAML
template class StartAction<std::string, unsigned int>;
template class StopAction<std::string, unsigned int>;
template class SetModeAction<std::string, unsigned int>;
template class SetTerminatorAction<std::string, unsigned int>;
template class SetLightModeAction<std::string, unsigned int>;
template class SetLocateLightModeAction<std::string, unsigned int>;
template class SetSoundModeAction<std::string, unsigned int>;
template class SetBuzzerVolumeAction<std::string, unsigned int>;
template class SetDecodingSuccessLightModeAction<std::string, unsigned int>;
template class SetBootSoundModeAction<std::string, unsigned int>;
template class SetDecodeSoundModeAction<std::string, unsigned int>;
template class SetScanDurationAction<std::string, unsigned int>;
template class SetStableInductionTimeAction<std::string, unsigned int>;
template class SetReadingIntervalAction<std::string, unsigned int>;
template class SetSameCodeIntervalAction<std::string, unsigned int>;
template class ProcessCurrentBufferAction<std::string, unsigned int>;
template class IsContinuousModeCondition<std::string, unsigned int>;

}  // namespace m5stack_barcode
}  // namespace esphome
