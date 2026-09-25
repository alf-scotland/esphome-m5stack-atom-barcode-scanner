#pragma once

#include <string>

#include "esphome/core/automation.h"
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
template<SettingId S, typename... Ts> class SetSettingAction : public Action<Ts...>, public Parented<BarcodeScanner> {
 public:
  TEMPLATABLE_VALUE(std::string, value)

  void play(const Ts &...x) override { this->parent_->set_setting(S, this->value_.value(x...)); }
};

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
