#pragma once

#include <string>

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "m5stack_barcode.h"

namespace esphome {
namespace m5stack_barcode {

// ── Triggers ──────────────────────────────────────────────────────────────────

/// on_barcode: fired for every decoded barcode, with the barcode as `x`.
class BarcodeTrigger : public Trigger<std::string> {
 public:
  explicit BarcodeTrigger(BarcodeScanner *parent) {
    parent->add_on_barcode_callback([this](const std::string &barcode) { this->trigger(barcode); });
  }
};

/// on_scan_timeout: fired when a HOST-mode scan ends after scan_duration without a result.
class ScanTimeoutTrigger : public Trigger<> {
 public:
  explicit ScanTimeoutTrigger(BarcodeScanner *parent) {
    parent->add_on_scan_timeout_callback([this]() { this->trigger(); });
  }
};

// ── Actions ───────────────────────────────────────────────────────────────────

/// Action that calls a parameterless scanner operation (start, stop, factory reset, ...).
template<void (BarcodeScanner::*Method)(), typename... Ts>
class ScannerMethodAction : public Action<Ts...>, public Parented<BarcodeScanner> {
 public:
  void play(const Ts &...x) override { (this->parent_->*Method)(); }
};

template<typename... Ts> using StartAction = ScannerMethodAction<&BarcodeScanner::start_scan, Ts...>;
template<typename... Ts> using StopAction = ScannerMethodAction<&BarcodeScanner::stop_scan, Ts...>;
template<typename... Ts> using FactoryResetAction = ScannerMethodAction<&BarcodeScanner::factory_reset, Ts...>;
template<typename... Ts>
using ProcessCurrentBufferAction = ScannerMethodAction<&BarcodeScanner::process_current_buffer, Ts...>;

/// Action that changes one scanner setting.  The value is the setting's option key (as used in
/// YAML and by the HA select), so it can come from a lambda, e.g. another select's state.
template<typename E, bool (*Parse)(const std::string &, E &), void (BarcodeScanner::*Setter)(E), typename... Ts>
class SetSettingAction : public Action<Ts...>, public Parented<BarcodeScanner> {
 public:
  TEMPLATABLE_VALUE(std::string, value)

  void play(const Ts &...x) override {
    const std::string value = this->value_.value(x...);
    E setting;
    if (!Parse(value, setting)) {
      ESP_LOGW(TAG_SCANNER, "Ignoring invalid setting value '%s'", value.c_str());
      return;
    }
    (this->parent_->*Setter)(setting);
  }
};

template<typename... Ts>
using SetModeAction = SetSettingAction<OperationMode, parse_operation_mode, &BarcodeScanner::set_operation_mode, Ts...>;
template<typename... Ts>
using SetTerminatorAction = SetSettingAction<Terminator, parse_terminator, &BarcodeScanner::set_terminator, Ts...>;
template<typename... Ts>
using SetLightModeAction = SetSettingAction<LightMode, parse_light_mode, &BarcodeScanner::set_light_mode, Ts...>;
template<typename... Ts>
using SetLocateLightModeAction =
    SetSettingAction<LocateLightMode, parse_locate_light_mode, &BarcodeScanner::set_locate_light_mode, Ts...>;
template<typename... Ts>
using SetBuzzerVolumeAction =
    SetSettingAction<BuzzerVolume, parse_buzzer_volume, &BarcodeScanner::set_buzzer_volume, Ts...>;
template<typename... Ts>
using SetScanDurationAction =
    SetSettingAction<ScanDuration, parse_scan_duration, &BarcodeScanner::set_scan_duration, Ts...>;
template<typename... Ts>
using SetStableInductionTimeAction = SetSettingAction<StableInductionTime, parse_stable_induction_time,
                                                      &BarcodeScanner::set_stable_induction_time, Ts...>;
template<typename... Ts>
using SetReadingIntervalAction =
    SetSettingAction<ReadingInterval, parse_reading_interval, &BarcodeScanner::set_reading_interval, Ts...>;
template<typename... Ts>
using SetSameCodeIntervalAction =
    SetSettingAction<SameCodeInterval, parse_same_code_interval, &BarcodeScanner::set_same_code_interval, Ts...>;
template<typename... Ts>
using SetSoundModeAction =
    SetSettingAction<SoundMode, parse_enabled<SoundMode>, &BarcodeScanner::set_sound_mode, Ts...>;
template<typename... Ts>
using SetBootSoundModeAction =
    SetSettingAction<BootSoundMode, parse_enabled<BootSoundMode>, &BarcodeScanner::set_boot_sound_mode, Ts...>;
template<typename... Ts>
using SetDecodeSoundModeAction =
    SetSettingAction<DecodeSoundMode, parse_enabled<DecodeSoundMode>, &BarcodeScanner::set_decode_sound_mode, Ts...>;
template<typename... Ts>
using SetDecodingSuccessLightModeAction =
    SetSettingAction<DecodingSuccessLightMode, parse_enabled<DecodingSuccessLightMode>,
                     &BarcodeScanner::set_decoding_success_light_mode, Ts...>;
template<typename... Ts>
using SetCmdAckSoundModeAction =
    SetSettingAction<CmdAckSoundMode, parse_enabled<CmdAckSoundMode>, &BarcodeScanner::set_cmd_ack_sound_mode, Ts...>;
template<typename... Ts>
using SetConfigCodeScanModeAction = SetSettingAction<ConfigCodeScanMode, parse_enabled<ConfigCodeScanMode>,
                                                     &BarcodeScanner::set_config_code_scan_mode, Ts...>;

// ── Conditions ────────────────────────────────────────────────────────────────

template<typename... Ts> class IsIdleCondition : public Condition<Ts...>, public Parented<BarcodeScanner> {
 public:
  bool check(const Ts &...x) override { return this->parent_->get_scan_state() == ScanState::IDLE; }
};

template<typename... Ts> class IsManualScanningCondition : public Condition<Ts...>, public Parented<BarcodeScanner> {
 public:
  bool check(const Ts &...x) override { return this->parent_->get_scan_state() == ScanState::MANUAL_SCANNING; }
};

template<typename... Ts> class IsContinuousModeCondition : public Condition<Ts...>, public Parented<BarcodeScanner> {
 public:
  bool check(const Ts &...x) override { return this->parent_->is_continuous_mode(); }
};

}  // namespace m5stack_barcode
}  // namespace esphome
