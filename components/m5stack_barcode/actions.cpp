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

  // Use the operation_mode_to_string helper
  const char* mode_str = operation_mode_to_string(this->mode_);
  ESP_LOGD(TAG_ACTION, "Setting operation mode to: %s", mode_str);

  this->scanner_->set_operation_mode(this->mode_);
}

template <typename... Ts>
void SetTerminatorAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  // Use the terminator_to_string helper
  const char* term_str = terminator_to_string(this->terminator_);
  ESP_LOGD(TAG_ACTION, "Setting terminator to: %s", term_str);

  this->scanner_->set_terminator(this->terminator_);
}

template <typename... Ts>
void SetLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  // Use the light_mode_to_string helper
  const char* mode_str = light_mode_to_string(this->mode_);
  ESP_LOGD(TAG_ACTION, "Setting light mode to: %s", mode_str);

  this->scanner_->set_light_mode(this->mode_);
}

template <typename... Ts>
void SetLocateLightModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  // Use the locate_light_mode_to_string helper
  const char* mode_str = locate_light_mode_to_string(this->mode_);
  ESP_LOGD(TAG_ACTION, "Setting locate light mode to: %s", mode_str);

  this->scanner_->set_locate_light_mode(this->mode_);
}

template <typename... Ts>
void SetSoundModeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  // Use the sound_mode_to_string helper
  const char* mode_str = sound_mode_to_string(this->mode_);
  ESP_LOGD(TAG_ACTION, "Setting sound mode to: %s", mode_str);

  this->scanner_->set_sound_mode(this->mode_);
}

template <typename... Ts>
void SetBuzzerVolumeAction<Ts...>::play(Ts... x) {
  if (this->scanner_ == nullptr) {
    ESP_LOGW(TAG_ACTION, "No scanner available");
    return;
  }

  // Use the buzzer_volume_to_string helper
  const char* volume_str = buzzer_volume_to_string(this->volume_);
  ESP_LOGD(TAG_ACTION, "Setting buzzer volume to: %s", volume_str);

  this->scanner_->set_buzzer_volume(this->volume_);
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
