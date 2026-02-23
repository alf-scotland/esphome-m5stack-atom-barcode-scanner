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
  std::string val = this->mode_.value(x...);
  OperationMode mode;
  if (!parse_operation_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Unknown operation mode: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting operation mode to: %s", val.c_str());
  this->scanner_->set_operation_mode(mode);
}

template<typename... Ts> void SetTerminatorAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->terminator_.value(x...);
  Terminator term;
  if (!parse_terminator(val, term)) {
    ESP_LOGW(TAG_ACTION, "Unknown terminator: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting terminator to: %s", val.c_str());
  this->scanner_->set_terminator(term);
}

template<typename... Ts> void SetLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  if (!this->light_mode_.has_value()) {
    ESP_LOGW(TAG_ACTION, "No light mode value provided");
    return;
  }
  std::string val = this->light_mode_.value(x...);
  LightMode mode;
  if (!parse_light_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Invalid light mode: %s", val.c_str());
    return;
  }
  this->scanner_->set_light_mode(mode);
}

template<typename... Ts> void SetLocateLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  if (!this->locate_light_mode_.has_value()) {
    ESP_LOGW(TAG_ACTION, "No locate light mode value provided");
    return;
  }
  std::string val = this->locate_light_mode_.value(x...);
  LocateLightMode mode;
  if (!parse_locate_light_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Invalid locate light mode: %s", val.c_str());
    return;
  }
  this->scanner_->set_locate_light_mode(mode);
}

template<typename... Ts> void SetSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->sound_mode_.value(x...);
  SoundMode mode;
  if (!parse_sound_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Unknown sound mode: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting sound mode to: %s", val.c_str());
  this->scanner_->set_sound_mode(mode);
}

template<typename... Ts> void SetBuzzerVolumeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  if (!this->volume_.has_value()) {
    ESP_LOGW(TAG_ACTION, "No buzzer volume value provided");
    return;
  }
  std::string val = this->volume_.value(x...);
  BuzzerVolume volume;
  if (!parse_buzzer_volume(val, volume)) {
    ESP_LOGW(TAG_ACTION, "Invalid buzzer volume: %s", val.c_str());
    return;
  }
  this->scanner_->set_buzzer_volume(volume);
}

template<typename... Ts> void SetDecodingSuccessLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  if (!this->decoding_success_light_mode_.has_value()) {
    ESP_LOGW(TAG_ACTION, "No decoding success light mode value provided");
    return;
  }
  std::string val = this->decoding_success_light_mode_.value(x...);
  DecodingSuccessLightMode mode;
  if (!parse_decoding_success_light_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Invalid decoding success light mode: %s", val.c_str());
    return;
  }
  this->scanner_->set_decoding_success_light_mode(mode);
}

template<typename... Ts> void SetBootSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->boot_sound_mode_.value(x...);
  BootSoundMode mode;
  if (!parse_boot_sound_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Unknown boot sound mode: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting boot sound mode to: %s", val.c_str());
  this->scanner_->set_boot_sound_mode(mode);
}

template<typename... Ts> void SetDecodeSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->decode_sound_mode_.value(x...);
  DecodeSoundMode mode;
  if (!parse_decode_sound_mode(val, mode)) {
    ESP_LOGW(TAG_ACTION, "Unknown decode sound mode: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting decode sound mode to: %s", val.c_str());
  this->scanner_->set_decode_sound_mode(mode);
}

template<typename... Ts> void SetScanDurationAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->duration_.value(x...);
  ScanDuration duration;
  if (!parse_scan_duration(val, duration)) {
    ESP_LOGW(TAG_ACTION, "Unknown scan duration: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting scan duration to: %s", val.c_str());
  this->scanner_->set_scan_duration(duration);
}

template<typename... Ts> void SetStableInductionTimeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->time_.value(x...);
  StableInductionTime time;
  if (!parse_stable_induction_time(val, time)) {
    ESP_LOGW(TAG_ACTION, "Unknown stable induction time: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting stable induction time to: %s", val.c_str());
  this->scanner_->set_stable_induction_time(time);
}

template<typename... Ts> void SetReadingIntervalAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->interval_.value(x...);
  ReadingInterval interval;
  if (!parse_reading_interval(val, interval)) {
    ESP_LOGW(TAG_ACTION, "Unknown reading interval: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting reading interval to: %s", val.c_str());
  this->scanner_->set_reading_interval(interval);
}

template<typename... Ts> void SetSameCodeIntervalAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }
  std::string val = this->interval_.value(x...);
  SameCodeInterval interval;
  if (!parse_same_code_interval(val, interval)) {
    ESP_LOGW(TAG_ACTION, "Unknown same code interval: %s", val.c_str());
    return;
  }
  ESP_LOGD(TAG_ACTION, "Setting same code interval to: %s", val.c_str());
  this->scanner_->set_same_code_interval(interval);
}

template<typename... Ts> void ProcessCurrentBufferAction<Ts...>::play(Ts... x) {
  ESP_LOGD(TAG_ACTION, "Processing current buffer");
  if (this->scanner_ != nullptr) {
    this->scanner_->process_current_buffer();
  }
}

template<typename... Ts> bool IsManualScanningCondition<Ts...>::check(Ts... x) {
  if (this->scanner_ == nullptr) {
    return false;
  }
  return this->scanner_->get_scan_state() == ScanState::MANUAL_SCANNING;
}

template<typename... Ts> bool IsIdleCondition<Ts...>::check(Ts... x) {
  if (this->scanner_ == nullptr) {
    return false;
  }
  return this->scanner_->get_scan_state() == ScanState::IDLE;
}

template<typename... Ts> bool IsContinuousModeCondition<Ts...>::check(Ts... x) {
  if (this->scanner_ == nullptr) {
    return false;
  }
  return this->scanner_->is_continuous_mode();
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
template class IsManualScanningCondition<>;
template class IsIdleCondition<>;
template class IsContinuousModeCondition<>;

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
