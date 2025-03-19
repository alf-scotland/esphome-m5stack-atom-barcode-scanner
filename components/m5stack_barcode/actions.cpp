#include "actions.h"

#include "esphome/core/log.h"
#include "m5stack_barcode.h"

namespace esphome {
namespace m5stack_barcode {

const char* const TAG_ACTION = "m5stack_barcode.action";

// Implement the action classes

template <typename... Ts>
void StartAction<Ts...>::play(Ts... x) {
  if (this->scanner_ != nullptr) {
    ESP_LOGD(TAG_ACTION, "Executing start scan action");
    this->scanner_->start_scan();
  }
}

template <typename... Ts>
void StopAction<Ts...>::play(Ts... x) {
  if (this->scanner_ != nullptr) {
    ESP_LOGD(TAG_ACTION, "Executing stop scan action");
    this->scanner_->stop_scan();
  }
}

template <typename... Ts>
void SetModeAction<Ts...>::play(Ts... x) {
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

template <typename... Ts>
void SetTerminatorAction<Ts...>::play(Ts... x) {
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

template <typename... Ts>
void SetLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  LightMode mode;

  if (mode_str == "on_when_reading")
    mode = LightMode::ON_WHEN_READING;
  else if (mode_str == "always_on")
    mode = LightMode::ALWAYS_ON;
  else if (mode_str == "always_off")
    mode = LightMode::ALWAYS_OFF;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown light mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting light mode to: %s", mode_str.c_str());
  this->scanner_->set_light_mode(mode);
}

template <typename... Ts>
void SetLocateLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  LocateLightMode mode;

  if (mode_str == "on_when_reading")
    mode = LocateLightMode::ON_WHEN_READING;
  else if (mode_str == "always_on")
    mode = LocateLightMode::ALWAYS_ON;
  else if (mode_str == "always_off")
    mode = LocateLightMode::ALWAYS_OFF;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown locate light mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting locate light mode to: %s", mode_str.c_str());
  this->scanner_->set_locate_light_mode(mode);
}

template <typename... Ts>
void SetSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string mode_str = this->mode_.value(x...);
  SoundMode mode;

  if (mode_str == "disabled")
    mode = SoundMode::SOUND_DISABLED;
  else if (mode_str == "enabled")
    mode = SoundMode::ENABLED;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown sound mode: %s", mode_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting sound mode to: %s", mode_str.c_str());
  this->scanner_->set_sound_mode(mode);
}

template <typename... Ts>
void SetBuzzerVolumeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  std::string volume_str = this->volume_.value(x...);
  BuzzerVolume volume;

  if (volume_str == "high")
    volume = BuzzerVolume::VOLUME_HIGH;
  else if (volume_str == "medium")
    volume = BuzzerVolume::MEDIUM;
  else if (volume_str == "low")
    volume = BuzzerVolume::VOLUME_LOW;
  else {
    ESP_LOGW(TAG_ACTION, "Unknown buzzer volume: %s", volume_str.c_str());
    return;
  }

  ESP_LOGD(TAG_ACTION, "Setting buzzer volume to: %s", volume_str.c_str());
  this->scanner_->set_buzzer_volume(volume);
}

// Explicit template instantiations
template class StartAction<>;
template class StopAction<>;
template class SetModeAction<>;
template class SetTerminatorAction<>;
template class SetLightModeAction<>;
template class SetLocateLightModeAction<>;
template class SetSoundModeAction<>;
template class SetBuzzerVolumeAction<>;

// Explicit template instantiations for the types used in YAML
template class StartAction<std::string, unsigned int>;
template class StopAction<std::string, unsigned int>;
template class SetModeAction<std::string, unsigned int>;
template class SetTerminatorAction<std::string, unsigned int>;
template class SetLightModeAction<std::string, unsigned int>;
template class SetLocateLightModeAction<std::string, unsigned int>;
template class SetSoundModeAction<std::string, unsigned int>;
template class SetBuzzerVolumeAction<std::string, unsigned int>;

}  // namespace m5stack_barcode
}  // namespace esphome
