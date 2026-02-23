#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "actions.h"
#include "command_handlers.h"
#include "commands.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/event/event.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

/// Version tag for the stored preference struct. Increment when the struct layout changes
/// to automatically invalidate stale preferences and force a full re-sync.
static const uint8_t SETTINGS_VERSION = 1;

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
} __attribute__((packed));

// Forward declarations
template<typename T> class StateCommand;
class OperationModeSelect;
class BuzzerVolumeSelect;
class LightModeSelect;
class LocateLightModeSelect;
class ScanDurationSelect;
class SoundSwitch;
class BootSoundSwitch;
class DecodeSoundSwitch;
class DecodingSuccessLightSwitch;

// Logging tag for this component
extern const char *const TAG_SCANNER;

/**
 * @brief M5Stack Barcode Scanner component for ESPHome.
 *
 * This component provides an interface to control the M5Stack 2D/QR Barcode Scanner Module.
 * It supports various operation modes, light control, sound settings, and firmware version
 * reporting.
 *
 * Features:
 * - Multiple operation modes (HOST, LEVEL, PULSE, CONTINUOUS, AUTO_SENSE)
 * - Configurable terminator settings
 * - Light control (main and locate lights)
 * - Sound and buzzer volume control
 * - Firmware version reporting
 * - Automatic wake-up handling
 * - Command acknowledgment verification
 * - Barcode scan events
 *
 * Requirements:
 * - UART communication configured in YAML
 * - Optional text sensor for barcode output
 * - Optional text sensor for firmware version
 * - Optional event component for barcode scan events
 *
 * Usage:
 * ```yaml
 * uart:
 *   tx_pin: GPIO1
 *   rx_pin: GPIO2
 *   baud_rate: 115200
 *
 * m5stack_barcode:
 *   text_sensor: barcode_sensor
 *   version_sensor: version_sensor
 *   operation_mode: HOST
 *   terminator: CRLF
 * ```
 */
class BarcodeScanner : public Component, public uart::UARTDevice {
 public:
  /**
   * @brief Initialize the barcode scanner component.
   *
   * Sets up UART communication and configures default settings.
   * Optionally requests firmware version if version sensor is configured.
   */
  void setup() override;

  /**
   * @brief Main loop function for processing commands and responses.
   *
   * Handles:
   * - Command queue processing
   * - Response acknowledgment
   * - Barcode data processing
   * - Version information processing
   */
  void loop() override;

  /**
   * @brief Dump the current configuration to the log.
   *
   * Logs all current settings and sensor states.
   */
  void dump_config() override;

  /**
   * @brief Get the setup priority for this component.
   * @return float Priority value (DATA)
   */
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Sensor Configuration
  /**
   * @brief Set the text sensor for barcode output.
   * @param text_sensor Pointer to the text sensor component
   */
  void set_text_sensor(text_sensor::TextSensor *text_sensor) { this->text_sensor_ = text_sensor; }

  /**
   * @brief Set the text sensor for firmware version.
   * @param version_sensor Pointer to the text sensor component
   */
  void set_version_sensor(text_sensor::TextSensor *version_sensor) { this->version_sensor_ = version_sensor; }

  /**
   * @brief Set the event component for barcode scan events.
   * @param event Pointer to the event component
   */
  void set_barcode_event(event::Event *event) { this->barcode_event_ = event; }

  /**
   * @brief Register a callback invoked whenever a barcode is successfully decoded.
   * @param callback Function receiving the decoded barcode string
   */
  void add_on_barcode_callback(std::function<void(std::string)> &&callback) {
    this->barcode_callback_.add(std::move(callback));
  }

  /**
   * @brief Register a callback invoked when a HOST-mode scan times out without a result.
   * @param callback Function called on scan timeout
   */
  void add_on_scan_timeout_callback(std::function<void()> &&callback) {
    this->scan_timeout_callback_.add(std::move(callback));
  }

  // Initial-value setters — called by ESPHome code generation to set desired values
  // before setup() runs. They update the in-memory state so configure_defaults_() sends
  // only the settings that differ from NVS, without double-queuing.
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

  /// Attach the optional operation-mode select sub-component.
  void set_operation_mode_select(OperationModeSelect *select) { this->operation_mode_select_ = select; }

  // Optional HA sub-component attachment — call from codegen to wire up entities.
  void set_scanning_binary_sensor(binary_sensor::BinarySensor *bs) { this->scanning_binary_sensor_ = bs; }
  void set_sound_switch(SoundSwitch *sw) { this->sound_switch_ = sw; }
  void set_boot_sound_switch(BootSoundSwitch *sw) { this->boot_sound_switch_ = sw; }
  void set_decode_sound_switch(DecodeSoundSwitch *sw) { this->decode_sound_switch_ = sw; }
  void set_decoding_success_light_switch(DecodingSuccessLightSwitch *sw) {
    this->decoding_success_light_switch_ = sw;
  }
  void set_buzzer_volume_select(BuzzerVolumeSelect *sel) { this->buzzer_volume_select_ = sel; }
  void set_light_mode_select(LightModeSelect *sel) { this->light_mode_select_ = sel; }
  void set_locate_light_mode_select(LocateLightModeSelect *sel) { this->locate_light_mode_select_ = sel; }
  void set_scan_duration_select(ScanDurationSelect *sel) { this->scan_duration_select_ = sel; }

  // Scanner Control Methods
  /**
   * @brief Start the barcode scanning process.
   *
   * Only works in HOST mode. Sets scanning state and queues start command.
   */
  void start_scan();

  /**
   * @brief Stop the barcode scanning process.
   *
   * Only works in HOST mode. Clears scanning state and queues stop command.
   */
  void stop_scan();

  // Scanner Settings Methods
  /**
   * @brief Set the operation mode of the scanner.
   * @param mode The desired operation mode
   * @return bool True if command was queued successfully
   */
  void set_operation_mode(OperationMode mode);

  /**
   * @brief Set the terminator sequence for barcode output.
   * @param term The desired terminator type
   * @return bool True if command was queued successfully
   */
  void set_terminator(Terminator term);

  /**
   * @brief Set the main light mode of the scanner.
   * @param mode The desired light mode
   * @return bool True if command was queued successfully
   */
  void set_light_mode(LightMode mode);

  /**
   * @brief Set the locate light mode of the scanner.
   * @param mode The desired locate light mode
   * @return bool True if command was queued successfully
   */
  void set_locate_light_mode(LocateLightMode mode);

  /**
   * @brief Set the sound mode of the scanner.
   * @param mode The desired sound mode
   * @return bool True if command was queued successfully
   */
  void set_sound_mode(SoundMode mode);

  /**
   * @brief Set the buzzer volume level.
   * @param volume The desired volume level
   * @return bool True if command was queued successfully
   */
  void set_buzzer_volume(BuzzerVolume volume);

  /**
   * @brief Set the decoding success light mode of the scanner.
   * @param mode The desired decoding success light mode
   * @return bool True if command was queued successfully
   */
  void set_decoding_success_light_mode(DecodingSuccessLightMode mode);

  /**
   * @brief Set the boot sound mode of the scanner.
   * @param mode The desired boot sound mode
   * @return bool True if command was queued successfully
   */
  void set_boot_sound_mode(BootSoundMode mode);

  /**
   * @brief Set the decode sound mode of the scanner.
   * @param mode The desired decode sound mode
   * @return bool True if command was queued successfully
   */
  void set_decode_sound_mode(DecodeSoundMode mode);

  /**
   * @brief Set the scan duration of the scanner.
   * @param duration The desired scan duration
   * @return bool True if command was queued successfully
   */
  void set_scan_duration(ScanDuration duration);

  /**
   * @brief Set the stable induction time of the scanner.
   * @param time The desired stable induction time
   * @return bool True if command was queued successfully
   */
  void set_stable_induction_time(StableInductionTime time);

  /**
   * @brief Set the reading interval of the scanner.
   * @param interval The desired reading interval
   * @return bool True if command was queued successfully
   */
  void set_reading_interval(ReadingInterval interval);

  /**
   * @brief Set the same code interval of the scanner.
   * @param interval The desired same code interval
   * @return bool True if command was queued successfully
   */
  void set_same_code_interval(SameCodeInterval interval);

  /**
   * @brief Process the current buffer as a barcode.
   * This is useful for continuous mode where we want to process intermediate results.
   */
  void process_current_buffer();

  /**
   * @brief Check if the scanner is in continuous mode.
   * @return bool True if in continuous mode or auto sense mode
   */
  bool is_continuous_mode() const;

  // State Accessors
  /**
   * @brief Check if the scanner is currently scanning.
   * @return bool True if scanning is active
   */
  bool is_scanning() const;

  /**
   * @brief Get the current scan state
   * @return ScanState enum value
   */
  ScanState get_scan_state() const;

  /**
   * @brief Set the scan state
   * @param state The new scan state
   */
  void set_scan_state(ScanState state);

  /**
   * @brief Get the current operation mode.
   * @return OperationMode Current operation mode
   */
  OperationMode get_operation_mode() const { return this->operation_mode_; }

  /**
   * @brief Get the current terminator setting.
   * @return Terminator Current terminator type
   */
  Terminator get_terminator() const { return this->terminator_; }

  /**
   * @brief Get the current light mode.
   * @return LightMode Current light mode
   */
  LightMode get_light_mode() const { return this->light_mode_; }

  /**
   * @brief Get the current locate light mode.
   * @return LocateLightMode Current locate light mode
   */
  LocateLightMode get_locate_light_mode() const { return this->locate_light_mode_; }

  /**
   * @brief Get the current sound mode.
   * @return SoundMode Current sound mode
   */
  SoundMode get_sound_mode() const { return this->sound_mode_; }

  /**
   * @brief Get the current buzzer volume.
   * @return BuzzerVolume Current volume level
   */
  BuzzerVolume get_buzzer_volume() const { return this->buzzer_volume_; }

  /**
   * @brief Get the current decoding success light mode.
   * @return DecodingSuccessLightMode Current decoding success light mode
   */
  DecodingSuccessLightMode get_decoding_success_light_mode() const { return this->decoding_success_light_mode_; }

  /**
   * @brief Get the current boot sound mode.
   * @return BootSoundMode Current boot sound mode
   */
  BootSoundMode get_boot_sound_mode() const { return this->boot_sound_mode_; }

  /**
   * @brief Get the current decode sound mode.
   * @return DecodeSoundMode Current decode sound mode
   */
  DecodeSoundMode get_decode_sound_mode() const { return this->decode_sound_mode_; }

  /**
   * @brief Get the current scan duration.
   * @return ScanDuration Current scan duration
   */
  ScanDuration get_scan_duration() const { return this->scan_duration_; }

  /**
   * @brief Get the current stable induction time.
   * @return StableInductionTime Current stable induction time
   */
  StableInductionTime get_stable_induction_time() const { return this->stable_induction_time_; }

  /**
   * @brief Get the current reading interval.
   * @return ReadingInterval Current reading interval
   */
  ReadingInterval get_reading_interval() const { return this->reading_interval_; }

  /**
   * @brief Get the current same code interval.
   * @return SameCodeInterval Current same code interval
   */
  SameCodeInterval get_same_code_interval() const { return this->same_code_interval_; }

  /**
   * @brief Get the current scan duration in milliseconds
   * @return uint32_t Duration in milliseconds (0 for unlimited)
   */
  uint32_t get_scan_duration_ms() const;

 protected:
  friend class CommandBase;
  template<typename T> friend class Command;
  template<typename T> friend class StateCommand;
  friend class SimpleCommand;
  template<typename... Ts> friend class IsContinuousModeCondition;
  template<typename... Ts> friend class ProcessCurrentBufferAction;
  template<typename... Ts> friend class StartAction;
  template<typename... Ts> friend class StopAction;
  template<typename... Ts> friend class SetModeAction;
  template<typename... Ts> friend class SetTerminatorAction;
  template<typename... Ts> friend class SetLightModeAction;
  template<typename... Ts> friend class SetLocateLightModeAction;
  template<typename... Ts> friend class SetSoundModeAction;
  template<typename... Ts> friend class SetBuzzerVolumeAction;
  template<typename... Ts> friend class SetDecodingSuccessLightModeAction;
  template<typename... Ts> friend class SetBootSoundModeAction;
  template<typename... Ts> friend class SetDecodeSoundModeAction;
  template<typename... Ts> friend class SetScanDurationAction;
  template<typename... Ts> friend class SetStableInductionTimeAction;
  template<typename... Ts> friend class SetReadingIntervalAction;
  template<typename... Ts> friend class SetSameCodeIntervalAction;

  // Protected state setter methods for use by commands
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
  void set_operation_mode_state(OperationMode mode);

  // Command Processing Methods
  /**
   * @brief Process the command queue and send commands to the scanner.
   */
  void process_command_queue_();

  /**
   * @brief Add a command to the processing queue.
   * @param command Unique pointer to the command to queue
   */
  void queue_command(std::unique_ptr<CommandBase> command);

  /**
   * @brief Wake up the scanner before sending a command.
   */
  void wake_up_();

  /**
   * @brief Check if the given data contains an acknowledgment sequence.
   * @param data Pointer to the data to check
   * @param len Length of the data
   * @param offset Offset into the data to start checking
   * @return bool True if an ACK sequence is found
   */
  bool is_ack_sequence_(const uint8_t *data, size_t len, size_t offset = 0) const;

  // Response Processing Methods
  /**
   * @brief Process received barcode data.
   */
  void process_barcode_();

  /**
   * @brief Request firmware version information.
   */
  void request_version_();

  /**
   * @brief Process received version information.
   */
  void process_version_();

  /**
   * @brief Configure default scanner settings.
   *
   * Compares each setting against the last-saved preferences and only queues
   * commands for values that have changed. On first boot (or after a version
   * bump) all settings are sent so the scanner state is fully initialised.
   */
  void configure_defaults_();

  /**
   * @brief Persist all current scanner settings to ESPHome preferences (NVS flash).
   *
   * Called after each successful command ACK so that subsequent boots can skip
   * re-sending settings that are already applied.
   */
  void save_settings_();

  // Buffer Management
  /**
   * @brief Clear the receive buffer.
   */
  void clear_buffer_();

  /**
   * @brief Reads available data from UART and adds it to the rx_buffer_
   */
  void read_buffer_();

  /**
   * @brief Set the expected response type for the current command.
   * @param type The expected response type
   */
  void set_expected_response_(ResponseType type);

  /**
   * @brief Write a command to the scanner and update state.
   * @param command Reference to the command to write
   */
  void write_command_(const std::unique_ptr<CommandBase> &command);

  // Preferences
  ESPPreferenceObject pref_;  ///< NVS storage for persisting scanner settings across reboots

  // Callbacks
  CallbackManager<void(std::string)> barcode_callback_;  ///< on_barcode automation triggers
  CallbackManager<void()> scan_timeout_callback_;        ///< on_scan_timeout automation triggers

  // Component State
  text_sensor::TextSensor *text_sensor_{nullptr};     ///< Sensor for barcode output
  text_sensor::TextSensor *version_sensor_{nullptr};  ///< Sensor for firmware version
  event::Event *barcode_event_{nullptr};              ///< Event for barcode scans
  OperationModeSelect *operation_mode_select_{nullptr};        ///< Optional HA select for operation mode
  BuzzerVolumeSelect *buzzer_volume_select_{nullptr};          ///< Optional HA select for buzzer volume
  LightModeSelect *light_mode_select_{nullptr};                ///< Optional HA select for light mode
  LocateLightModeSelect *locate_light_mode_select_{nullptr};   ///< Optional HA select for locate light mode
  ScanDurationSelect *scan_duration_select_{nullptr};          ///< Optional HA select for scan duration
  SoundSwitch *sound_switch_{nullptr};                         ///< Optional HA switch for sound mode
  BootSoundSwitch *boot_sound_switch_{nullptr};                ///< Optional HA switch for boot sound mode
  DecodeSoundSwitch *decode_sound_switch_{nullptr};            ///< Optional HA switch for decode sound mode
  DecodingSuccessLightSwitch *decoding_success_light_switch_{nullptr};  ///< Optional HA switch for success light
  binary_sensor::BinarySensor *scanning_binary_sensor_{nullptr};        ///< Optional HA binary sensor for scan state

  std::vector<uint8_t> rx_buffer_;                           ///< Buffer for received data
  std::vector<std::unique_ptr<CommandBase>> command_queue_;  ///< Queue of pending commands

  ScanState scan_state_{ScanState::IDLE};               ///< Current detailed scan state
  bool waiting_for_ack_{false};                         ///< Whether waiting for command acknowledgment
  uint32_t last_command_time_{0};                       ///< Timestamp of last command sent
  uint32_t scan_started_at_{0};                         ///< millis() when start_scan() was called; 0 = idle
  CommandState command_state_{CommandState::IDLE};      ///< Current command processing state
  ResponseType expected_response_{ResponseType::NONE};  ///< Expected response type

  // Scanner Settings
  OperationMode operation_mode_{OperationMode::HOST};                                 ///< Current operation mode
  Terminator terminator_{Terminator::NONE};                                           ///< Current terminator setting
  LightMode light_mode_{LightMode::LIGHT_ON_WHEN_READING};                            ///< Current light mode
  LocateLightMode locate_light_mode_{LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING};  ///< Current locate light mode
  SoundMode sound_mode_{SoundMode::SOUND_DISABLED};                                   ///< Current sound mode
  BuzzerVolume buzzer_volume_{BuzzerVolume::BUZZER_VOLUME_LOW};                       ///< Current buzzer volume
  DecodingSuccessLightMode decoding_success_light_mode_{
      DecodingSuccessLightMode::DECODING_LIGHT_ENABLED};                      ///< Current decoding success light mode
  BootSoundMode boot_sound_mode_{BootSoundMode::BOOT_SOUND_ENABLED};          ///< Current boot sound mode
  DecodeSoundMode decode_sound_mode_{DecodeSoundMode::DECODE_SOUND_ENABLED};  ///< Current decode sound mode
  ScanDuration scan_duration_{ScanDuration::MS_3000};                         ///< Current scan duration
  StableInductionTime stable_induction_time_{StableInductionTime::MS_500};    ///< Current stable induction time
  ReadingInterval reading_interval_{ReadingInterval::MS_500};                 ///< Current reading interval
  SameCodeInterval same_code_interval_{SameCodeInterval::MS_500};             ///< Current same code interval
};

/// Trigger fired whenever a barcode is successfully decoded.
/// Use via `on_barcode:` in YAML to run automations with the scanned code as `x`.
class BarcodeTrigger : public Trigger<std::string> {
 public:
  explicit BarcodeTrigger(BarcodeScanner *parent) {
    parent->add_on_barcode_callback([this](std::string barcode) { this->trigger(std::move(barcode)); });
  }
};

/// Trigger fired when a HOST-mode scan times out (scan_duration elapsed without a result).
/// Use via `on_scan_timeout:` in YAML for error handling and user feedback automations.
class ScanTimeoutTrigger : public Trigger<> {
 public:
  explicit ScanTimeoutTrigger(BarcodeScanner *parent) {
    parent->add_on_scan_timeout_callback([this]() { this->trigger(); });
  }
};

/// Select sub-component that exposes the scanner's operation mode as a Home Assistant
/// select entity. Changing the select in HA sends the appropriate UART command and waits
/// for the scanner ACK before publishing the new confirmed state.
class OperationModeSelect : public select::Select {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

  /// Returns the YAML key string for an OperationMode enum value (used when publishing state).
  static const char *mode_to_key(OperationMode mode) {
    switch (mode) {
      case OperationMode::LEVEL: return "level";
      case OperationMode::PULSE: return "pulse";
      case OperationMode::CONTINUOUS: return "continuous";
      case OperationMode::AUTO_SENSE: return "auto_sense";
      default: return "host";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Select sub-component for buzzer_volume. Exposes the three volume levels as a HA select.
class BuzzerVolumeSelect : public select::Select {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

  static const char *volume_to_key(BuzzerVolume volume) {
    switch (volume) {
      case BuzzerVolume::BUZZER_VOLUME_HIGH: return "high";
      case BuzzerVolume::BUZZER_VOLUME_MEDIUM: return "medium";
      default: return "low";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Select sub-component for light_mode (main illumination light).
class LightModeSelect : public select::Select {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

  static const char *mode_to_key(LightMode mode) {
    switch (mode) {
      case LightMode::LIGHT_ALWAYS_ON: return "always_on";
      case LightMode::LIGHT_ALWAYS_OFF: return "always_off";
      default: return "on_when_reading";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Select sub-component for locate_light_mode (aiming pattern light).
class LocateLightModeSelect : public select::Select {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

  static const char *mode_to_key(LocateLightMode mode) {
    switch (mode) {
      case LocateLightMode::LOCATE_LIGHT_ALWAYS_ON: return "always_on";
      case LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF: return "always_off";
      default: return "on_when_reading";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Select sub-component for scan_duration (how long the scanner tries before giving up).
class ScanDurationSelect : public select::Select {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

  static const char *duration_to_key(ScanDuration duration) {
    switch (duration) {
      case ScanDuration::MS_500: return "500ms";
      case ScanDuration::MS_1000: return "1s";
      case ScanDuration::MS_5000: return "5s";
      case ScanDuration::MS_10000: return "10s";
      case ScanDuration::MS_15000: return "15s";
      case ScanDuration::MS_20000: return "20s";
      case ScanDuration::UNLIMITED: return "unlimited";
      default: return "3s";
    }
  }

 protected:
  void control(const std::string &value) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Switch sub-component for sound_mode — maps the HA toggle to SOUND_ENABLED / SOUND_DISABLED.
class SoundSwitch : public switch_::Switch {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Switch sub-component for boot_sound_mode — controls whether the scanner beeps on power-up.
class BootSoundSwitch : public switch_::Switch {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Switch sub-component for decode_sound_mode — controls whether the scanner beeps on decode.
class DecodeSoundSwitch : public switch_::Switch {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Switch sub-component for decoding_success_light_mode — controls the success flash.
class DecodingSuccessLightSwitch : public switch_::Switch {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void write_state(bool state) override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Button sub-component that starts a HOST-mode scan when pressed in Home Assistant.
class StartButton : public button::Button {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void press_action() override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

/// Button sub-component that stops the current HOST-mode scan when pressed in Home Assistant.
class StopButton : public button::Button {
 public:
  void set_scanner(BarcodeScanner *scanner) { scanner_ = scanner; }

 protected:
  void press_action() override;

 private:
  BarcodeScanner *scanner_{nullptr};
};

}  // namespace m5stack_barcode
}  // namespace esphome
