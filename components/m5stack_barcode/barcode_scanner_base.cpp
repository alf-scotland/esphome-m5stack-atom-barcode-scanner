#include "barcode_scanner_base.h"

#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

const char *const TAG_SCANNER = "m5stack_barcode";

// ─── BarcodeScannerBase concrete methods ─────────────────────────────────────

void BarcodeScannerBase::set_scan_state(ScanState state) {
  if (this->scan_state_ != state) {
    ESP_LOGD(TAG_SCANNER, "Scan state changed from %d to %d", (int) this->scan_state_, (int) state);
    this->scan_state_ = state;
    if (this->scanning_binary_sensor_ != nullptr)
      this->scanning_binary_sensor_->publish_state(state != ScanState::IDLE);
  }
}

uint32_t BarcodeScannerBase::get_scan_duration_ms() const {
  return scan_duration_to_ms(this->scan_duration_);
}

void BarcodeScannerBase::fire_barcode_(const std::string &data) {
  ESP_LOGD(TAG_SCANNER, "Barcode received: %s", data.c_str());

  if (this->barcode_sensor_ != nullptr)
    this->barcode_sensor_->publish_state(data);

  this->barcode_callback_(data);

  if (this->scan_event_ != nullptr)
    this->scan_event_->trigger("scan_successful");
}

void BarcodeScannerBase::publish_initial_states_() {
  if (this->operation_mode_select_ != nullptr)
    this->operation_mode_select_->publish_state(OperationModeSelect::to_key(this->operation_mode_));
  if (this->terminator_select_ != nullptr)
    this->terminator_select_->publish_state(TerminatorSelect::to_key(this->terminator_));
  if (this->light_mode_select_ != nullptr)
    this->light_mode_select_->publish_state(LightModeSelect::to_key(this->light_mode_));
  if (this->locate_light_mode_select_ != nullptr)
    this->locate_light_mode_select_->publish_state(LocateLightModeSelect::to_key(this->locate_light_mode_));
  if (this->buzzer_volume_select_ != nullptr)
    this->buzzer_volume_select_->publish_state(BuzzerVolumeSelect::to_key(this->buzzer_volume_));
  if (this->scan_duration_select_ != nullptr)
    this->scan_duration_select_->publish_state(ScanDurationSelect::to_key(this->scan_duration_));
  if (this->stable_induction_time_select_ != nullptr)
    this->stable_induction_time_select_->publish_state(
        StableInductionTimeSelect::to_key(this->stable_induction_time_));
  if (this->reading_interval_select_ != nullptr)
    this->reading_interval_select_->publish_state(ReadingIntervalSelect::to_key(this->reading_interval_));
  if (this->same_code_interval_select_ != nullptr)
    this->same_code_interval_select_->publish_state(SameCodeIntervalSelect::to_key(this->same_code_interval_));
  if (this->sound_switch_ != nullptr)
    this->sound_switch_->publish_state(this->sound_mode_ == SoundMode::SOUND_ENABLED);
  if (this->boot_sound_switch_ != nullptr)
    this->boot_sound_switch_->publish_state(this->boot_sound_mode_ == BootSoundMode::BOOT_SOUND_ENABLED);
  if (this->decode_sound_switch_ != nullptr)
    this->decode_sound_switch_->publish_state(this->decode_sound_mode_ == DecodeSoundMode::DECODE_SOUND_ENABLED);
  if (this->decoding_success_light_switch_ != nullptr)
    this->decoding_success_light_switch_->publish_state(this->decoding_success_light_mode_ ==
                                                        DecodingSuccessLightMode::DECODING_LIGHT_ENABLED);
  if (this->cmd_ack_sound_switch_ != nullptr)
    this->cmd_ack_sound_switch_->publish_state(this->cmd_ack_sound_mode_ == CmdAckSoundMode::CMD_ACK_SOUND_ENABLED);
  if (this->config_code_scan_switch_ != nullptr)
    this->config_code_scan_switch_->publish_state(this->config_code_scan_mode_ ==
                                                  ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED);
}

// ─── Select sub-component implementations ────────────────────────────────────

void OperationModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "OperationModeSelect: no scanner attached");
    return;
  }
  OperationMode mode;
  if (!parse_operation_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "OperationModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_operation_mode(mode);
}

void BuzzerVolumeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "BuzzerVolumeSelect: no scanner attached");
    return;
  }
  BuzzerVolume vol;
  if (!parse_buzzer_volume(value, vol)) {
    ESP_LOGW(TAG_SCANNER, "BuzzerVolumeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_buzzer_volume(vol);
}

void LightModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "LightModeSelect: no scanner attached");
    return;
  }
  LightMode mode;
  if (!parse_light_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "LightModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_light_mode(mode);
}

void LocateLightModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "LocateLightModeSelect: no scanner attached");
    return;
  }
  LocateLightMode mode;
  if (!parse_locate_light_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "LocateLightModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_locate_light_mode(mode);
}

void ScanDurationSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "ScanDurationSelect: no scanner attached");
    return;
  }
  ScanDuration dur;
  if (!parse_scan_duration(value, dur)) {
    ESP_LOGW(TAG_SCANNER, "ScanDurationSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_scan_duration(dur);
}

void TerminatorSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "TerminatorSelect: no scanner attached");
    return;
  }
  Terminator term;
  if (!parse_terminator(value, term)) {
    ESP_LOGW(TAG_SCANNER, "TerminatorSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_terminator(term);
}

void StableInductionTimeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "StableInductionTimeSelect: no scanner attached");
    return;
  }
  StableInductionTime time;
  if (!parse_stable_induction_time(value, time)) {
    ESP_LOGW(TAG_SCANNER, "StableInductionTimeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_stable_induction_time(time);
}

void ReadingIntervalSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "ReadingIntervalSelect: no scanner attached");
    return;
  }
  ReadingInterval interval;
  if (!parse_reading_interval(value, interval)) {
    ESP_LOGW(TAG_SCANNER, "ReadingIntervalSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_reading_interval(interval);
}

void SameCodeIntervalSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "SameCodeIntervalSelect: no scanner attached");
    return;
  }
  SameCodeInterval interval;
  if (!parse_same_code_interval(value, interval)) {
    ESP_LOGW(TAG_SCANNER, "SameCodeIntervalSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_same_code_interval(interval);
}

// ─── Switch sub-component implementations ────────────────────────────────────

void SoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "SoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_sound_mode(state ? SoundMode::SOUND_ENABLED : SoundMode::SOUND_DISABLED);
}

void BootSoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "BootSoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_boot_sound_mode(state ? BootSoundMode::BOOT_SOUND_ENABLED : BootSoundMode::BOOT_SOUND_DISABLED);
}

void DecodeSoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "DecodeSoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_decode_sound_mode(state ? DecodeSoundMode::DECODE_SOUND_ENABLED
                                        : DecodeSoundMode::DECODE_SOUND_DISABLED);
}

void DecodingSuccessLightSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "DecodingSuccessLightSwitch: no scanner attached");
    return;
  }
  scanner_->set_decoding_success_light_mode(state ? DecodingSuccessLightMode::DECODING_LIGHT_ENABLED
                                                  : DecodingSuccessLightMode::DECODING_LIGHT_DISABLED);
}

void CmdAckSoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "CmdAckSoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_cmd_ack_sound_mode(state ? CmdAckSoundMode::CMD_ACK_SOUND_ENABLED
                                         : CmdAckSoundMode::CMD_ACK_SOUND_DISABLED);
}

void ConfigCodeScanSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "ConfigCodeScanSwitch: no scanner attached");
    return;
  }
  scanner_->set_config_code_scan_mode(state ? ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED
                                            : ConfigCodeScanMode::CONFIG_CODE_SCAN_DISABLED);
}

// ─── Button sub-component implementations ────────────────────────────────────

void StartButton::press_action() {
  if (scanner_ != nullptr)
    scanner_->start_scan();
}

void StopButton::press_action() {
  if (scanner_ != nullptr)
    scanner_->stop_scan();
}

void FactoryResetButton::press_action() {
  if (scanner_ != nullptr)
    scanner_->factory_reset();
}

}  // namespace m5stack_barcode
}  // namespace esphome
