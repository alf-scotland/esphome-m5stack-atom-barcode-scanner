#pragma once

#include <memory>
#include <string>
#include <vector>

#include "command.h"
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
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

/// Version tag for the stored preference struct. Increment when the struct layout changes
/// to automatically invalidate stale preferences and force a full re-sync.
static const uint8_t SETTINGS_VERSION = 2;

/// Packed representation of all scanner settings stored in ESPHome preferences (NVS flash).
/// On first boot (or after a factory reset / version bump) all fields are sent to the scanner.
/// On subsequent boots only settings that differ from what was last ACK'd are re-sent.
struct ScannerPreferences {
  uint8_t version;
  uint8_t operation_mode;
  uint8_t terminator;
  uint8_t light_mode;
  uint8_t locate_light_mode;
  uint8_t sound_mode;
  uint8_t buzzer_volume;
  uint8_t decoding_success_light_mode;
  uint8_t boot_sound_mode;
  uint8_t decode_sound_mode;
  uint8_t scan_duration;
  uint8_t stable_induction_time;
  uint8_t reading_interval;
  uint8_t same_code_interval;
  uint8_t cmd_ack_sound_mode;
  uint8_t config_code_scan_mode;
} __attribute__((packed));

// Catch struct layout changes (added/removed fields, unexpected padding) at compile time.
// Increment SETTINGS_VERSION whenever the struct changes so stale NVS data is discarded.
static_assert(sizeof(ScannerPreferences) == 16, "ScannerPreferences size changed — bump SETTINGS_VERSION");

extern const char *const TAG_SCANNER;

/// Driver for the M5Stack Atom QR code scanner (UART, 9600 baud).
///
/// Settings are sent to the scanner through a non-blocking command queue: each command is
/// preceded by the wake-up byte, sent once the scanner has had WAKEUP_DELAY_MS to wake, and
/// must be ACKed before the next one is sent.  Settings take effect in memory, NVS and the
/// HA entities only once the scanner ACKs them.
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

  /// Register a callback invoked with every successfully decoded barcode.
  void add_on_barcode_callback(std::function<void(const std::string &)> &&callback) {
    this->barcode_callback_.add(std::move(callback));
  }
  /// Register a callback invoked when a HOST-mode scan times out without a result.
  void add_on_scan_timeout_callback(std::function<void()> &&callback) {
    this->scan_timeout_callback_.add(std::move(callback));
  }

  // Initial values from YAML, applied before setup().  configure_defaults_() then sends only
  // the settings that differ from what the scanner last ACKed (persisted in NVS).
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

  // Optional HA entities kept in sync with the ACKed settings.
  void set_operation_mode_select(select::Select *sel) { this->operation_mode_select_ = sel; }
  void set_terminator_select(select::Select *sel) { this->terminator_select_ = sel; }
  void set_light_mode_select(select::Select *sel) { this->light_mode_select_ = sel; }
  void set_locate_light_mode_select(select::Select *sel) { this->locate_light_mode_select_ = sel; }
  void set_buzzer_volume_select(select::Select *sel) { this->buzzer_volume_select_ = sel; }
  void set_scan_duration_select(select::Select *sel) { this->scan_duration_select_ = sel; }
  void set_stable_induction_time_select(select::Select *sel) { this->stable_induction_time_select_ = sel; }
  void set_reading_interval_select(select::Select *sel) { this->reading_interval_select_ = sel; }
  void set_same_code_interval_select(select::Select *sel) { this->same_code_interval_select_ = sel; }
  void set_sound_switch(switch_::Switch *sw) { this->sound_switch_ = sw; }
  void set_boot_sound_switch(switch_::Switch *sw) { this->boot_sound_switch_ = sw; }
  void set_decode_sound_switch(switch_::Switch *sw) { this->decode_sound_switch_ = sw; }
  void set_decoding_success_light_switch(switch_::Switch *sw) { this->decoding_success_light_switch_ = sw; }
  void set_cmd_ack_sound_switch(switch_::Switch *sw) { this->cmd_ack_sound_switch_ = sw; }
  void set_config_code_scan_switch(switch_::Switch *sw) { this->config_code_scan_switch_ = sw; }

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

  // Setting changes: queue the UART command if the value differs from the current one.
  // The new value is applied (and published to HA) only once the scanner ACKs it.
  void set_operation_mode(OperationMode mode);
  void set_terminator(Terminator term);
  void set_light_mode(LightMode mode);
  void set_locate_light_mode(LocateLightMode mode);
  void set_sound_mode(SoundMode mode);
  void set_buzzer_volume(BuzzerVolume volume);
  void set_decoding_success_light_mode(DecodingSuccessLightMode mode);
  void set_boot_sound_mode(BootSoundMode mode);
  void set_decode_sound_mode(DecodeSoundMode mode);
  void set_scan_duration(ScanDuration duration);
  void set_stable_induction_time(StableInductionTime time);
  void set_reading_interval(ReadingInterval interval);
  void set_same_code_interval(SameCodeInterval interval);
  void set_cmd_ack_sound_mode(CmdAckSoundMode mode);
  void set_config_code_scan_mode(ConfigCodeScanMode mode);

  /// True in continuous and auto-sense mode, where the scanner scans on its own.
  bool is_continuous_mode() const;
  bool is_scanning() const { return this->scan_state_ != ScanState::IDLE; }
  ScanState get_scan_state() const { return this->scan_state_; }
  void set_scan_state(ScanState state);

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
  CmdAckSoundMode get_cmd_ack_sound_mode() const { return this->cmd_ack_sound_mode_; }
  ConfigCodeScanMode get_config_code_scan_mode() const { return this->config_code_scan_mode_; }
  /// Scan duration in milliseconds (0 for unlimited).
  uint32_t get_scan_duration_ms() const { return scan_duration_to_ms(this->scan_duration_); }

 protected:
  // CommandFactory callbacks apply settings once the scanner ACKs them.
  friend class CommandFactory;

  // Apply an ACKed setting: update memory, persist to NVS and publish to the HA entity.
  void set_operation_mode_state(OperationMode mode);
  void set_terminator_state(Terminator term);
  void set_light_mode_state(LightMode mode);
  void set_locate_light_mode_state(LocateLightMode mode);
  void set_sound_mode_state(SoundMode mode);
  void set_buzzer_volume_state(BuzzerVolume volume);
  void set_decoding_success_light_mode_state(DecodingSuccessLightMode mode);
  void set_boot_sound_mode_state(BootSoundMode mode);
  void set_decode_sound_mode_state(DecodeSoundMode mode);
  void set_scan_duration_state(ScanDuration duration);
  void set_stable_induction_time_state(StableInductionTime time);
  void set_reading_interval_state(ReadingInterval interval);
  void set_same_code_interval_state(SameCodeInterval interval);
  void set_cmd_ack_sound_mode_state(CmdAckSoundMode mode);
  void set_config_code_scan_mode_state(ConfigCodeScanMode mode);

  /// Queue the commands for every setting whose value differs from the NVS-persisted state.
  void configure_defaults_();
  /// Publish the current setting values to every attached HA entity.
  void publish_initial_states_();
  /// Persist all current settings to NVS after an ACK.
  void save_settings_();
  /// Invalidate NVS preferences and reboot (called once the scanner ACKs a factory reset).
  void do_factory_reset_();

  void queue_command(std::unique_ptr<Command> command);
  /// Queue a setting command, superseding any queued command for the same setting.
  /// @param is_current Whether the value equals the current (ACKed) one
  void queue_setting_(std::unique_ptr<Command> command, bool is_current);
  void process_command_queue_();
  void wake_up_();
  void write_command_(const std::unique_ptr<Command> &command);
  /// Consume the ACK for the in-flight command, or retry / drop it once COMMAND_TIMEOUT_MS elapses.
  void handle_ack_or_timeout_();
  /// Collect the unframed GET_VERSION response once the line has gone idle.
  void handle_version_response_();
  /// Pop the in-flight command off the queue and reset the command state machine.
  void finish_command_();
  /// Called when the scanner ACKs a start command: starts the on_scan_timeout timer.
  void on_scan_started_();

  void read_buffer_();
  void clear_buffer_() { this->rx_buffer_.clear(); }
  /// Length of the configured terminator if rx_buffer_ currently ends with it, else 0.
  size_t terminator_length_in_buffer_() const;
  /// Whether rx_buffer_ holds a complete barcode frame (terminator received or line idle).
  bool has_complete_frame_() const;
  void process_barcode_();
  void process_version_();

  ESPPreferenceObject pref_;
  /// Per-setting flags (1 = the scanner already had the YAML value at boot, per NVS)
  ScannerPreferences confirmed_at_boot_{};

  CallbackManager<void(const std::string &)> barcode_callback_;
  CallbackManager<void()> scan_timeout_callback_;

  text_sensor::TextSensor *barcode_sensor_{nullptr};
  text_sensor::TextSensor *version_sensor_{nullptr};
  event::Event *scan_event_{nullptr};
  binary_sensor::BinarySensor *scanning_binary_sensor_{nullptr};
  select::Select *operation_mode_select_{nullptr};
  select::Select *terminator_select_{nullptr};
  select::Select *light_mode_select_{nullptr};
  select::Select *locate_light_mode_select_{nullptr};
  select::Select *buzzer_volume_select_{nullptr};
  select::Select *scan_duration_select_{nullptr};
  select::Select *stable_induction_time_select_{nullptr};
  select::Select *reading_interval_select_{nullptr};
  select::Select *same_code_interval_select_{nullptr};
  switch_::Switch *sound_switch_{nullptr};
  switch_::Switch *boot_sound_switch_{nullptr};
  switch_::Switch *decode_sound_switch_{nullptr};
  switch_::Switch *decoding_success_light_switch_{nullptr};
  switch_::Switch *cmd_ack_sound_switch_{nullptr};
  switch_::Switch *config_code_scan_switch_{nullptr};

  std::vector<uint8_t> rx_buffer_;
  std::vector<std::unique_ptr<Command>> command_queue_;

  ScanState scan_state_{ScanState::IDLE};
  CommandState command_state_{CommandState::IDLE};
  ResponseType expected_response_{ResponseType::NONE};  ///< Response type of the in-flight command
  bool waiting_for_ack_{false};
  bool initial_states_published_{false};  ///< Guard for one-shot publish_initial_states_() in loop()
  bool discard_frame_{false};             ///< Drop the rest of the current frame after an RX buffer overflow
  uint8_t command_attempts_{0};           ///< Send attempts for the front-of-queue command
  uint32_t last_command_time_{0};         ///< millis() of the last wake-up or command sent
  uint32_t last_rx_time_{0};              ///< millis() of the last byte received into rx_buffer_
  uint32_t scan_started_at_{0};           ///< millis() when the scanner ACKed start; 0 = no timer

  // Scanner settings (component defaults, overridden by the YAML initial values)
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

/// HA select for one scanner setting.  The select's option index is the setting's enum value
/// (the option lists in __init__.py follow enum order), so no string round-trip is needed.
/// The displayed state only changes once the scanner ACKs the new value.
template<typename E, void (BarcodeScanner::*Setter)(E)>
class SettingSelect : public select::Select, public Parented<BarcodeScanner> {
 protected:
  void control(size_t index) override { (this->parent_->*Setter)(static_cast<E>(index)); }
};

/// HA switch for a two-state scanner setting (DISABLED = 0, ENABLED = 1).
/// The displayed state only changes once the scanner ACKs the new value.
template<typename E, void (BarcodeScanner::*Setter)(E)>
class SettingSwitch : public switch_::Switch, public Parented<BarcodeScanner> {
 protected:
  void write_state(bool state) override { (this->parent_->*Setter)(static_cast<E>(state)); }
};

/// HA button that invokes a scanner operation.
template<void (BarcodeScanner::*Method)()>
class ScannerButton : public button::Button, public Parented<BarcodeScanner> {
 protected:
  void press_action() override { (this->parent_->*Method)(); }
};

using OperationModeSelect = SettingSelect<OperationMode, &BarcodeScanner::set_operation_mode>;
using TerminatorSelect = SettingSelect<Terminator, &BarcodeScanner::set_terminator>;
using LightModeSelect = SettingSelect<LightMode, &BarcodeScanner::set_light_mode>;
using LocateLightModeSelect = SettingSelect<LocateLightMode, &BarcodeScanner::set_locate_light_mode>;
using BuzzerVolumeSelect = SettingSelect<BuzzerVolume, &BarcodeScanner::set_buzzer_volume>;
using ScanDurationSelect = SettingSelect<ScanDuration, &BarcodeScanner::set_scan_duration>;
using StableInductionTimeSelect = SettingSelect<StableInductionTime, &BarcodeScanner::set_stable_induction_time>;
using ReadingIntervalSelect = SettingSelect<ReadingInterval, &BarcodeScanner::set_reading_interval>;
using SameCodeIntervalSelect = SettingSelect<SameCodeInterval, &BarcodeScanner::set_same_code_interval>;

using SoundSwitch = SettingSwitch<SoundMode, &BarcodeScanner::set_sound_mode>;
using BootSoundSwitch = SettingSwitch<BootSoundMode, &BarcodeScanner::set_boot_sound_mode>;
using DecodeSoundSwitch = SettingSwitch<DecodeSoundMode, &BarcodeScanner::set_decode_sound_mode>;
using DecodingSuccessLightSwitch =
    SettingSwitch<DecodingSuccessLightMode, &BarcodeScanner::set_decoding_success_light_mode>;
using CmdAckSoundSwitch = SettingSwitch<CmdAckSoundMode, &BarcodeScanner::set_cmd_ack_sound_mode>;
using ConfigCodeScanSwitch = SettingSwitch<ConfigCodeScanMode, &BarcodeScanner::set_config_code_scan_mode>;

using StartButton = ScannerButton<&BarcodeScanner::start_scan>;
using StopButton = ScannerButton<&BarcodeScanner::stop_scan>;
using FactoryResetButton = ScannerButton<&BarcodeScanner::factory_reset>;

}  // namespace m5stack_barcode
}  // namespace esphome
