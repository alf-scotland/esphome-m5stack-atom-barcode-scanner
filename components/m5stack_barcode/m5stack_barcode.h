#pragma once

#include <array>
#include <string>
#include <vector>

#include "commands.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/event/event.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace m5stack_barcode {

/// One value per setting, indexed by SettingId.
using SettingValues = std::array<uint8_t, NUM_SETTINGS>;

/// Version tag of the stored ScannerPreferences.  Increment when the layout changes.
static const uint8_t SETTINGS_VERSION = 3;
/// Version written by firmware before the YAML baseline was stored; migrated once at boot.
static const uint8_t LEGACY_SETTINGS_VERSION = 2;

/// Settings persisted to NVS.  `applied` holds what the scanner last ACKed (including changes
/// made from Home Assistant), `yaml` the YAML values the device last booted with.  At boot a
/// setting is re-sent only if its YAML value was edited since then; otherwise the applied value
/// is kept, so runtime changes survive reboots and OTA updates.
struct ScannerPreferences {
  uint8_t version;
  SettingValues applied;
  SettingValues yaml;
} __attribute__((packed));
static_assert(sizeof(ScannerPreferences) == 1 + 2 * NUM_SETTINGS, "layout changed: bump SETTINGS_VERSION");

/// Layout of LEGACY_SETTINGS_VERSION, stored under a different key.
struct LegacyScannerPreferences {
  uint8_t version;
  SettingValues applied;
} __attribute__((packed));
static_assert(sizeof(LegacyScannerPreferences) == 16, "the legacy layout is fixed");

enum class ScanState : uint8_t {
  IDLE,                // Not scanning
  MANUAL_SCANNING,     // HOST-mode scan started by start_scan()
  CONTINUOUS_SCANNING  // Continuous or auto-sense mode
};

/// Driver for the M5Stack Atom QR code scanner (UART, 9600 baud).
///
/// Commands go through a non-blocking queue: each is preceded by the wake-up byte, sent once
/// the scanner has had time to wake, and must be answered before the next one is sent.
/// Settings take effect in memory, NVS and the HA entities only once the scanner ACKs them.
/// A setting value is an index into the setting's option keys (see SettingInfo).
class BarcodeScanner : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_barcode_sensor(text_sensor::TextSensor *sensor) { this->barcode_sensor_ = sensor; }
  void set_version_sensor(text_sensor::TextSensor *sensor) { this->version_sensor_ = sensor; }
  void set_scan_event(event::Event *ev) { this->scan_event_ = ev; }
  void set_scanning_binary_sensor(binary_sensor::BinarySensor *bs) { this->scanning_binary_sensor_ = bs; }
  /// The YAML value of a setting, set before setup().
  void set_initial_value(SettingId id, uint8_t value) { this->settings_[static_cast<size_t>(id)] = value; }
  void set_select(SettingId id, select::Select *sel) { this->selects_[static_cast<size_t>(id)] = sel; }
  void set_switch(SettingId id, switch_::Switch *sw) { this->switches_[static_cast<size_t>(id)] = sw; }

  /// Register a callback invoked with every decoded barcode.
  void add_on_barcode_callback(std::function<void(const std::string &)> &&callback) {
    this->barcode_callback_.add(std::move(callback));
  }
  /// Register a callback invoked when a HOST-mode scan times out without a result.
  void add_on_scan_timeout_callback(std::function<void()> &&callback) {
    this->scan_timeout_callback_.add(std::move(callback));
  }

  /// Start a HOST-mode scan. Ignored outside HOST mode or while a scan is in progress.
  void start_scan();
  /// Stop the current HOST-mode scan.
  void stop_scan();
  /// Reset the scanner to factory defaults, then reboot the ESP so every YAML-configured
  /// setting is re-applied (NVS preferences are invalidated once the scanner ACKs).
  void factory_reset();
  /// Publish whatever is buffered as a barcode immediately, without waiting for the frame to
  /// end.  Normally unnecessary: loop() frames barcodes by terminator or idle gap in every mode.
  void process_current_buffer();

  /// Change a setting.  The value is applied (and published to HA) once the scanner ACKs it.
  void set_setting(SettingId id, uint8_t value);
  /// Change a setting given its option key (e.g. "medium"); invalid keys are logged and ignored.
  void set_setting(SettingId id, const std::string &key);
  /// The setting's current value: the one the scanner last ACKed, or the YAML value before that.
  uint8_t get_setting(SettingId id) const { return this->settings_[static_cast<size_t>(id)]; }

  OperationMode get_operation_mode() const {
    return static_cast<OperationMode>(this->get_setting(SettingId::OPERATION_MODE));
  }
  /// Scan duration in milliseconds (0 for unlimited).
  uint32_t get_scan_duration_ms() const { return scan_duration_ms(this->get_setting(SettingId::SCAN_DURATION)); }
  /// True in continuous and auto-sense mode, where the scanner scans on its own.
  bool is_continuous_mode() const;
  bool is_scanning() const { return this->scan_state_ != ScanState::IDLE; }
  ScanState get_scan_state() const { return this->scan_state_; }

 protected:
  enum class CommandType : uint8_t { SETTING, START_SCAN, STOP_SCAN, GET_VERSION, FACTORY_RESET };
  struct Command {
    CommandType type;
    SettingId setting{SettingId::OPERATION_MODE};  ///< SETTING only
    uint8_t value{0};                              ///< SETTING only
  };
  enum class CommandState : uint8_t {
    IDLE,         // Nothing sent for the command at the front of the queue
    WAKEUP_SENT,  // Wake-up byte sent, waiting WAKEUP_DELAY_MS before sending the command
    COMMAND_SENT  // Command sent, waiting for its reply
  };

  /// Restore the persisted settings, then queue commands for settings that must be (re)sent:
  /// all of them without valid preferences, otherwise those whose YAML value was edited.
  void restore_settings_();
  /// Persist the applied settings and the YAML baseline to NVS.
  void save_settings_();
  /// Publish a setting's current value to its HA entity, if any.
  void publish_setting_(SettingId id);
  /// Apply a setting the scanner ACKed: update memory, persist to NVS and publish to HA.
  void apply_setting_(SettingId id, uint8_t value);

  void queue_command_(const Command &command);
  /// Queue a setting command, superseding any queued command for the same setting.
  void queue_setting_(SettingId id, uint8_t value);
  void process_command_queue_();
  const uint8_t *command_frame_(const Command &command) const;
  void log_command_(const char *action, const Command &command) const;
  /// Consume the ACK or NAK for the in-flight command, or retry / drop it on timeout.
  void handle_ack_or_timeout_();
  /// Collect the unframed GET_VERSION response once the line has gone idle.
  void handle_version_response_();
  /// Pop the in-flight command off the queue and run its success or failure handling.
  void finish_command_(bool success);
  void set_scan_state_(ScanState state);

  void read_buffer_();
  /// Length of the configured terminator if rx_buffer_ currently ends with it, else 0.
  size_t terminator_length_in_buffer_() const;
  /// Whether rx_buffer_ holds a complete barcode frame (terminator received or line idle).
  bool has_complete_frame_() const;
  void process_barcode_();
  /// Apply a configuration barcode (its content after CONFIG_CODE_PREFIX) the scanner passed on.
  void apply_config_code_(const std::string &code);
  void process_version_();

  ESPPreferenceObject pref_;
  /// Current values: the YAML values until restore_settings_(), then the ones the scanner has
  /// (restored from NVS or ACKed since)
  SettingValues settings_{};
  /// YAML values this firmware was built with (the baseline saved to NVS)
  SettingValues yaml_settings_{};
  /// Bit per setting: restored from NVS at boot (the scanner already has the value)
  uint16_t confirmed_at_boot_{0};
  static_assert(NUM_SETTINGS <= 16, "confirmed_at_boot_ has a bit per setting");

  CallbackManager<void(const std::string &)> barcode_callback_;
  CallbackManager<void()> scan_timeout_callback_;

  text_sensor::TextSensor *barcode_sensor_{nullptr};
  text_sensor::TextSensor *version_sensor_{nullptr};
  event::Event *scan_event_{nullptr};
  binary_sensor::BinarySensor *scanning_binary_sensor_{nullptr};
  std::array<select::Select *, NUM_SETTINGS> selects_{};
  std::array<switch_::Switch *, NUM_SETTINGS> switches_{};

  std::vector<uint8_t> rx_buffer_;
  std::vector<Command> command_queue_;

  ScanState scan_state_{ScanState::IDLE};
  CommandState command_state_{CommandState::IDLE};
  bool initial_states_published_{false};  ///< Guard for the one-shot publish in loop()
  bool discard_frame_{false};             ///< Drop the rest of the current frame after an RX buffer overflow
  bool scan_timer_active_{false};         ///< A HOST-mode scan is running and may time out
  uint8_t command_attempts_{0};           ///< Send attempts for the front-of-queue command
  uint32_t last_command_time_{0};         ///< millis() of the last wake-up or command sent
  uint32_t last_rx_time_{0};              ///< millis() of the last byte received into rx_buffer_
  uint32_t scan_started_at_{0};           ///< millis() when the scanner ACKed start
};

/// HA select for a scanner setting: the option index is the setting value.
/// The displayed state only changes once the scanner ACKs the new value.
template<SettingId S> class SettingSelect : public select::Select, public Parented<BarcodeScanner> {
 protected:
  void control(size_t index) override { this->parent_->set_setting(S, static_cast<uint8_t>(index)); }
};

/// HA switch for a two-state scanner setting (options "disabled", "enabled").
/// The displayed state only changes once the scanner ACKs the new value.
template<SettingId S> class SettingSwitch : public switch_::Switch, public Parented<BarcodeScanner> {
 protected:
  void write_state(bool state) override { this->parent_->set_setting(S, static_cast<uint8_t>(state)); }
};

/// HA button that invokes a scanner operation.
template<void (BarcodeScanner::*Method)()>
class ScannerButton : public button::Button, public Parented<BarcodeScanner> {
 protected:
  void press_action() override { (this->parent_->*Method)(); }
};

using StartButton = ScannerButton<&BarcodeScanner::start_scan>;
using StopButton = ScannerButton<&BarcodeScanner::stop_scan>;
using FactoryResetButton = ScannerButton<&BarcodeScanner::factory_reset>;

}  // namespace m5stack_barcode
}  // namespace esphome
