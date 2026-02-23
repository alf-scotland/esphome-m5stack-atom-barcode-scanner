#pragma once

#include <memory>
#include <vector>

#include "actions.h"
#include "command_handlers.h"
#include "commands.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/event/event.h"
#include "esphome/core/component.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Forward declarations
template<typename T> class StateCommand;

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
   */
  void configure_defaults_();

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

  // Component State
  text_sensor::TextSensor *text_sensor_{nullptr};     ///< Sensor for barcode output
  text_sensor::TextSensor *version_sensor_{nullptr};  ///< Sensor for firmware version
  event::Event *barcode_event_{nullptr};              ///< Event for barcode scans

  std::vector<uint8_t> rx_buffer_;                           ///< Buffer for received data
  std::vector<std::unique_ptr<CommandBase>> command_queue_;  ///< Queue of pending commands

  ScanState scan_state_{ScanState::IDLE};               ///< Current detailed scan state
  bool waiting_for_ack_{false};                         ///< Whether waiting for command acknowledgment
  uint32_t last_command_time_{0};                       ///< Timestamp of last command sent
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

}  // namespace m5stack_barcode
}  // namespace esphome
