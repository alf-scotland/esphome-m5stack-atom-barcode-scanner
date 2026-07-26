#pragma once

#include <functional>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/event/event.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Forward declarations — concrete sub-component classes defined below
class OperationModeSelect;
class BuzzerVolumeSelect;
class LightModeSelect;
class LocateLightModeSelect;
class ScanDurationSelect;
class TerminatorSelect;
class StableInductionTimeSelect;
class ReadingIntervalSelect;
class SameCodeIntervalSelect;
class SoundSwitch;
class BootSoundSwitch;
class DecodeSoundSwitch;
class DecodingSuccessLightSwitch;
class CmdAckSoundSwitch;
class ConfigCodeScanSwitch;
class FactoryResetButton;

extern const char *const TAG_SCANNER;

/// Abstract base class shared by all scanner model implementations.
///
/// Owns all HA sub-component pointers, callbacks, and settings storage so that
/// concrete classes (BarcodeScanner for V1, BarcodeScannerV2UART for V2) only
/// need to implement the wire protocol.  Every public setter/getter and all
/// HA entity wiring lives here — concrete classes implement the pure-virtual
/// `set_*()` and control methods to translate settings into UART commands.
class BarcodeScannerBase : public Component, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  // ── Sensor / event attachment ─────────────────────────────────────────────
  void set_barcode_sensor(text_sensor::TextSensor *sensor) { this->barcode_sensor_ = sensor; }
  void set_version_sensor(text_sensor::TextSensor *sensor) { this->version_sensor_ = sensor; }
  void set_scan_event(event::Event *ev) { this->scan_event_ = ev; }

  // ── Callback registration ─────────────────────────────────────────────────
  void add_on_barcode_callback(std::function<void(std::string)> &&callback) {
    this->barcode_callback_.add(std::move(callback));
  }
  void add_on_scan_timeout_callback(std::function<void()> &&callback) {
    this->scan_timeout_callback_.add(std::move(callback));
  }

  // ── Initial-value setters (codegen calls before setup()) ─────────────────
  void set_operation_mode_initial(OperationMode mode) { this->operation_mode_ = mode; }
  void set_terminator_initial(Terminator term) { this->terminator_ = term; }
  void set_light_mode_initial(LightMode mode) { this->light_mode_ = mode; }
  void set_locate_light_mode_initial(LocateLightMode mode) { this->locate_light_mode_ = mode; }
  void set_sound_mode_initial(SoundMode mode) { this->sound_mode_ = mode; }
  void set_buzzer_volume_initial(BuzzerVolume volume) { this->buzzer_volume_ = volume; }
  void set_decoding_success_light_mode_initial(DecodingSuccessLightMode mode) {
    this->decoding_success_light_mode_ = mode;
  }
  void set_boot_sound_mode_initial(BootSoundMode mode) { this->boot_sound_mode_ = mode; }
  void set_decode_sound_mode_initial(DecodeSoundMode mode) { this->decode_sound_mode_ = mode; }
  void set_scan_duration_initial(ScanDuration duration) { this->scan_duration_ = duration; }
  void set_stable_induction_time_initial(StableInductionTime time) { this->stable_induction_time_ = time; }
  void set_reading_interval_initial(ReadingInterval interval) { this->reading_interval_ = interval; }
  void set_same_code_interval_initial(SameCodeInterval interval) { this->same_code_interval_ = interval; }
  void set_cmd_ack_sound_mode_initial(CmdAckSoundMode mode) { this->cmd_ack_sound_mode_ = mode; }
  void set_config_code_scan_mode_initial(ConfigCodeScanMode mode) { this->config_code_scan_mode_ = mode; }

  // ── Sub-component attachment (codegen wires these up) ────────────────────
  void set_scanning_binary_sensor(binary_sensor::BinarySensor *bs) { this->scanning_binary_sensor_ = bs; }
  void set_operation_mode_select(OperationModeSelect *select) { this->operation_mode_select_ = select; }
  void set_buzzer_volume_select(BuzzerVolumeSelect *sel) { this->buzzer_volume_select_ = sel; }
  void set_light_mode_select(LightModeSelect *sel) { this->light_mode_select_ = sel; }
  void set_locate_light_mode_select(LocateLightModeSelect *sel) { this->locate_light_mode_select_ = sel; }
  void set_scan_duration_select(ScanDurationSelect *sel) { this->scan_duration_select_ = sel; }
  void set_terminator_select(TerminatorSelect *sel) { this->terminator_select_ = sel; }
  void set_stable_induction_time_select(StableInductionTimeSelect *sel) {
    this->stable_induction_time_select_ = sel;
  }
  void set_reading_interval_select(ReadingIntervalSelect *sel) { this->reading_interval_select_ = sel; }
  void set_same_code_interval_select(SameCodeIntervalSelect *sel) { this->same_code_interval_select_ = sel; }
  void set_sound_switch(SoundSwitch *sw) { this->sound_switch_ = sw; }
  void set_boot_sound_switch(BootSoundSwitch *sw) { this->boot_sound_switch_ = sw; }
  void set_decode_sound_switch(DecodeSoundSwitch *sw) { this->decode_sound_switch_ = sw; }
  void set_decoding_success_light_switch(DecodingSuccessLightSwitch *sw) {
    this->decoding_success_light_switch_ = sw;
  }
  void set_cmd_ack_sound_switch(CmdAckSoundSwitch *sw) { this->cmd_ack_sound_switch_ = sw; }
  void set_config_code_scan_switch(ConfigCodeScanSwitch *sw) { this->config_code_scan_switch_ = sw; }

  // ── Getters ───────────────────────────────────────────────────────────────
  OperationMode get_operation_mode() const { return this->operation_mode_; }
  Terminator get_terminator() const { return this->terminator_; }
  LightMode get_light_mode() const { return this->light_mode_; }
  LocateLightMode get_locate_light_mode() const { return this->locate_light_mode_; }
  SoundMode get_sound_mode() const { return this->sound_mode_; }
  BuzzerVolume get_buzzer_volume() const { return this->buzzer_volume_; }
  DecodingSuccessLightMode get_decoding_success_light_mode() const { return this->decoding_success_light_mode_; }
  BootSoundMode get_boot_sound_mode() const { return this->boot_sound_mode_; }
  DecodeSoundMode get_decode_sound_mode() const { return this->decode_sound_mode_; }
  ScanDuration get_scan_duration() const { return this->scan_duration_; }
  StableInductionTime get_stable_induction_time() const { return this->stable_induction_time_; }
  ReadingInterval get_reading_interval() const { return this->reading_interval_; }
  SameCodeInterval get_same_code_interval() const { return this->same_code_interval_; }

  // ── Concrete state accessors (non-virtual, shared implementation) ─────────
  bool is_scanning() const { return this->scan_state_ != ScanState::IDLE; }
  bool is_continuous_mode() const {
    return this->operation_mode_ == OperationMode::CONTINUOUS ||
           this->operation_mode_ == OperationMode::AUTO_SENSE;
  }
  ScanState get_scan_state() const { return this->scan_state_; }
  void set_scan_state(ScanState state);
  uint32_t get_scan_duration_ms() const;

  // Default no-op — V1 overrides; V2 processes barcodes via packet protocol
  virtual void process_current_buffer() {}

  // ── Pure-virtual: scanner control ────────────────────────────────────────
  virtual void start_scan() = 0;
  virtual void stop_scan() = 0;
  virtual void factory_reset() = 0;

  // ── Pure-virtual: settings (each concrete class sends via its own protocol)
  virtual void set_operation_mode(OperationMode mode) = 0;
  virtual void set_terminator(Terminator term) = 0;
  virtual void set_light_mode(LightMode mode) = 0;
  virtual void set_locate_light_mode(LocateLightMode mode) = 0;
  virtual void set_sound_mode(SoundMode mode) = 0;
  virtual void set_buzzer_volume(BuzzerVolume volume) = 0;
  virtual void set_decoding_success_light_mode(DecodingSuccessLightMode mode) = 0;
  virtual void set_boot_sound_mode(BootSoundMode mode) = 0;
  virtual void set_decode_sound_mode(DecodeSoundMode mode) = 0;
  virtual void set_scan_duration(ScanDuration duration) = 0;
  virtual void set_stable_induction_time(StableInductionTime time) = 0;
  virtual void set_reading_interval(ReadingInterval interval) = 0;
  virtual void set_same_code_interval(SameCodeInterval interval) = 0;
  virtual void set_cmd_ack_sound_mode(CmdAckSoundMode mode) = 0;
  virtual void set_config_code_scan_mode(ConfigCodeScanMode mode) = 0;

 protected:
  // Publish a decoded barcode string to all registered HA entities/callbacks.
  void fire_barcode_(const std::string &data);

  // Push all in-memory settings to HA entity state (called once on first loop()).
  void publish_initial_states_();

  // ── Sub-component pointers ────────────────────────────────────────────────
  text_sensor::TextSensor *barcode_sensor_{nullptr};
  text_sensor::TextSensor *version_sensor_{nullptr};
  event::Event *scan_event_{nullptr};
  binary_sensor::BinarySensor *scanning_binary_sensor_{nullptr};
  OperationModeSelect *operation_mode_select_{nullptr};
  BuzzerVolumeSelect *buzzer_volume_select_{nullptr};
  LightModeSelect *light_mode_select_{nullptr};
  LocateLightModeSelect *locate_light_mode_select_{nullptr};
  ScanDurationSelect *scan_duration_select_{nullptr};
  TerminatorSelect *terminator_select_{nullptr};
  StableInductionTimeSelect *stable_induction_time_select_{nullptr};
  ReadingIntervalSelect *reading_interval_select_{nullptr};
  SameCodeIntervalSelect *same_code_interval_select_{nullptr};
  SoundSwitch *sound_switch_{nullptr};
  BootSoundSwitch *boot_sound_switch_{nullptr};
  DecodeSoundSwitch *decode_sound_switch_{nullptr};
  DecodingSuccessLightSwitch *decoding_success_light_switch_{nullptr};
  CmdAckSoundSwitch *cmd_ack_sound_switch_{nullptr};
  ConfigCodeScanSwitch *config_code_scan_switch_{nullptr};

  // ── Callbacks ─────────────────────────────────────────────────────────────
  CallbackManager<void(std::string)> barcode_callback_;
  CallbackManager<void()> scan_timeout_callback_;

  // ── Scan state ────────────────────────────────────────────────────────────
  ScanState scan_state_{ScanState::IDLE};

  // ── Settings storage (set by initial setters; updated on ACK) ────────────
  OperationMode operation_mode_{OperationMode::HOST};
  Terminator terminator_{Terminator::NONE};
  LightMode light_mode_{LightMode::LIGHT_ON_WHEN_READING};
  LocateLightMode locate_light_mode_{LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING};
  SoundMode sound_mode_{SoundMode::SOUND_DISABLED};
  BuzzerVolume buzzer_volume_{BuzzerVolume::BUZZER_VOLUME_LOW};
  DecodingSuccessLightMode decoding_success_light_mode_{DecodingSuccessLightMode::DECODING_LIGHT_ENABLED};
  BootSoundMode boot_sound_mode_{BootSoundMode::BOOT_SOUND_DISABLED};
  DecodeSoundMode decode_sound_mode_{DecodeSoundMode::DECODE_SOUND_ENABLED};
  ScanDuration scan_duration_{ScanDuration::MS_3000};
  StableInductionTime stable_induction_time_{StableInductionTime::MS_500};
  ReadingInterval reading_interval_{ReadingInterval::MS_500};
  SameCodeInterval same_code_interval_{SameCodeInterval::MS_500};
  CmdAckSoundMode cmd_ack_sound_mode_{CmdAckSoundMode::CMD_ACK_SOUND_ENABLED};
  ConfigCodeScanMode config_code_scan_mode_{ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED};
};

// ═══════════════════════════════════════════════════════════════════════════
// Trigger classes
// ═══════════════════════════════════════════════════════════════════════════

class BarcodeTrigger : public Trigger<std::string> {
 public:
  explicit BarcodeTrigger(BarcodeScannerBase *parent) {
    parent->add_on_barcode_callback([this](std::string barcode) { this->trigger(std::move(barcode)); });
  }
};

class ScanTimeoutTrigger : public Trigger<> {
 public:
  explicit ScanTimeoutTrigger(BarcodeScannerBase *parent) {
    parent->add_on_scan_timeout_callback([this]() { this->trigger(); });
  }
};

// ═══════════════════════════════════════════════════════════════════════════
// Select sub-components
// ═══════════════════════════════════════════════════════════════════════════

class OperationModeSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(OperationMode mode) {
    switch (mode) {
      case OperationMode::LEVEL:
        return "level";
      case OperationMode::PULSE:
        return "pulse";
      case OperationMode::CONTINUOUS:
        return "continuous";
      case OperationMode::AUTO_SENSE:
        return "auto_sense";
      default:
        return "host";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class BuzzerVolumeSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(BuzzerVolume volume) {
    switch (volume) {
      case BuzzerVolume::BUZZER_VOLUME_HIGH:
        return "high";
      case BuzzerVolume::BUZZER_VOLUME_MEDIUM:
        return "medium";
      default:
        return "low";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class LightModeSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(LightMode mode) {
    switch (mode) {
      case LightMode::LIGHT_ALWAYS_ON:
        return "always_on";
      case LightMode::LIGHT_ALWAYS_OFF:
        return "always_off";
      default:
        return "on_when_reading";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class LocateLightModeSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(LocateLightMode mode) {
    switch (mode) {
      case LocateLightMode::LOCATE_LIGHT_ALWAYS_ON:
        return "always_on";
      case LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF:
        return "always_off";
      default:
        return "on_when_reading";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class ScanDurationSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(ScanDuration duration) {
    switch (duration) {
      case ScanDuration::MS_500:
        return "500ms";
      case ScanDuration::MS_1000:
        return "1s";
      case ScanDuration::MS_3000:
        return "3s";
      case ScanDuration::MS_5000:
        return "5s";
      case ScanDuration::MS_10000:
        return "10s";
      case ScanDuration::MS_15000:
        return "15s";
      case ScanDuration::MS_20000:
        return "20s";
      case ScanDuration::UNLIMITED:
        return "unlimited";
    }
    return nullptr;
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class TerminatorSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(Terminator term) {
    switch (term) {
      case Terminator::CRLF:
        return "crlf";
      case Terminator::CR:
        return "cr";
      case Terminator::TAB:
        return "tab";
      case Terminator::CRCR:
        return "crcr";
      case Terminator::CRLFCRLF:
        return "crlfcrlf";
      default:
        return "none";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class StableInductionTimeSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(StableInductionTime time) {
    switch (time) {
      case StableInductionTime::MS_0:
        return "0ms";
      case StableInductionTime::MS_100:
        return "100ms";
      case StableInductionTime::MS_300:
        return "300ms";
      case StableInductionTime::MS_1000:
        return "1s";
      default:
        return "500ms";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class ReadingIntervalSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(ReadingInterval interval) {
    switch (interval) {
      case ReadingInterval::MS_0:
        return "0ms";
      case ReadingInterval::MS_100:
        return "100ms";
      case ReadingInterval::MS_300:
        return "300ms";
      case ReadingInterval::MS_1000:
        return "1s";
      case ReadingInterval::MS_1500:
        return "1.5s";
      case ReadingInterval::MS_2000:
        return "2s";
      default:
        return "500ms";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class SameCodeIntervalSelect : public select::Select, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

  static const char *to_key(SameCodeInterval interval) {
    switch (interval) {
      case SameCodeInterval::MS_0:
        return "0ms";
      case SameCodeInterval::MS_100:
        return "100ms";
      case SameCodeInterval::MS_300:
        return "300ms";
      case SameCodeInterval::MS_1000:
        return "1s";
      case SameCodeInterval::MS_1500:
        return "1.5s";
      case SameCodeInterval::MS_2000:
        return "2s";
      default:
        return "500ms";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

// ═══════════════════════════════════════════════════════════════════════════
// Switch sub-components
// ═══════════════════════════════════════════════════════════════════════════

class SoundSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class BootSoundSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class DecodeSoundSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class DecodingSuccessLightSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class CmdAckSoundSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class ConfigCodeScanSwitch : public switch_::Switch, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

// ═══════════════════════════════════════════════════════════════════════════
// Button sub-components
// ═══════════════════════════════════════════════════════════════════════════

class StartButton : public button::Button, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void press_action() override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class StopButton : public button::Button, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void press_action() override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

class FactoryResetButton : public button::Button, public Component {
 public:
  void set_scanner(BarcodeScannerBase *scanner) { scanner_ = scanner; }

 protected:
  void press_action() override;

 private:
  BarcodeScannerBase *scanner_{nullptr};
};

}  // namespace m5stack_barcode
}  // namespace esphome

// Action/condition template bodies — included here so BarcodeScannerBase is
// complete when the template methods are compiled.  Do NOT include elsewhere.
#include "actions_impl.h"
