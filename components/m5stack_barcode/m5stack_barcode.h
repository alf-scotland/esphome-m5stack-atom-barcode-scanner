#pragma once

#include <memory>
#include <vector>

#include "actions.h"
#include "command_handlers.h"
#include "commands.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Add constants for direct use in main.cpp
constexpr auto HOST = OperationMode::HOST;
constexpr auto CRLF = Terminator::CRLF;

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
 *
 * Requirements:
 * - UART communication configured in YAML
 * - Optional text sensor for barcode output
 * - Optional text sensor for firmware version
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
  void set_text_sensor(text_sensor::TextSensor* text_sensor) { this->text_sensor_ = text_sensor; }

  /**
   * @brief Set the text sensor for firmware version.
   * @param version_sensor Pointer to the text sensor component
   */
  void set_version_sensor(text_sensor::TextSensor* version_sensor) {
    this->version_sensor_ = version_sensor;
  }

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
  bool set_operation_mode(OperationMode mode);

  /**
   * @brief Set the terminator sequence for barcode output.
   * @param term The desired terminator type
   * @return bool True if command was queued successfully
   */
  bool set_terminator(Terminator term);

  /**
   * @brief Set the main light mode of the scanner.
   * @param mode The desired light mode
   * @return bool True if command was queued successfully
   */
  bool set_light_mode(LightMode mode);

  /**
   * @brief Set the locate light mode of the scanner.
   * @param mode The desired locate light mode
   * @return bool True if command was queued successfully
   */
  bool set_locate_light_mode(LocateLightMode mode);

  /**
   * @brief Set the sound mode of the scanner.
   * @param mode The desired sound mode
   * @return bool True if command was queued successfully
   */
  bool set_sound_mode(SoundMode mode);

  /**
   * @brief Set the buzzer volume level.
   * @param volume The desired volume level
   * @return bool True if command was queued successfully
   */
  bool set_buzzer_volume(BuzzerVolume volume);

  // State Accessors
  /**
   * @brief Check if the scanner is currently scanning.
   * @return bool True if scanning is active
   */
  bool is_scanning() const { return this->scanning_; }

  /**
   * @brief Set the scanning state.
   * @param scanning True to indicate scanning is active
   */
  void set_scanning(bool scanning) { this->scanning_ = scanning; }

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

  // Action Creators for ESPHome Automations
  /**
   * @brief Create a start scanning action.
   * @return StartAction<>* Pointer to the created action
   */
  StartAction<>* make_start_action() { return new StartAction<>(this); }

  /**
   * @brief Create a stop scanning action.
   * @return StopAction<>* Pointer to the created action
   */
  StopAction<>* make_stop_action() { return new StopAction<>(this); }

  /**
   * @brief Create a set operation mode action.
   * @return SetModeAction<>* Pointer to the created action
   */
  SetModeAction<>* make_mode_action() { return new SetModeAction<>(this); }

  /**
   * @brief Create a set terminator action.
   * @return SetTerminatorAction<>* Pointer to the created action
   */
  SetTerminatorAction<>* make_terminator_action() { return new SetTerminatorAction<>(this); }

  /**
   * @brief Create a set light mode action.
   * @return SetLightModeAction<>* Pointer to the created action
   */
  SetLightModeAction<>* make_light_mode_action() { return new SetLightModeAction<>(this); }

  /**
   * @brief Create a set locate light mode action.
   * @return SetLocateLightModeAction<>* Pointer to the created action
   */
  SetLocateLightModeAction<>* make_locate_light_mode_action() {
    return new SetLocateLightModeAction<>(this);
  }

  /**
   * @brief Create a set sound mode action.
   * @return SetSoundModeAction<>* Pointer to the created action
   */
  SetSoundModeAction<>* make_sound_mode_action() { return new SetSoundModeAction<>(this); }

  /**
   * @brief Create a set buzzer volume action.
   * @return SetBuzzerVolumeAction<>* Pointer to the created action
   */
  SetBuzzerVolumeAction<>* make_buzzer_volume_action() { return new SetBuzzerVolumeAction<>(this); }

protected:
  friend class CommandBase;
  friend class Command<OperationMode>;
  friend class Command<Terminator>;
  friend class Command<LightMode>;
  friend class Command<LocateLightMode>;
  friend class Command<SoundMode>;
  friend class Command<BuzzerVolume>;
  friend class SimpleCommand;

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
  bool is_ack_sequence_(const uint8_t* data, size_t len, size_t offset = 0) const;

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
   * @brief Set the expected response type for the current command.
   * @param type The expected response type
   */
  void set_expected_response_(ResponseType type);

  // Command Factories
  /**
   * @brief Create a mode command.
   * @param mode The operation mode to set
   * @return std::unique_ptr<CommandBase> The created command
   */
  std::unique_ptr<CommandBase> create_mode_command(OperationMode mode);

  /**
   * @brief Create a terminator command.
   * @param term The terminator to set
   * @return std::unique_ptr<CommandBase> The created command
   */
  std::unique_ptr<CommandBase> create_terminator_command(Terminator term);

  /**
   * @brief Create a start scanning command.
   * @return std::unique_ptr<CommandBase> The created command
   */
  std::unique_ptr<CommandBase> create_start_command();

  /**
   * @brief Create a stop scanning command.
   * @return std::unique_ptr<CommandBase> The created command
   */
  std::unique_ptr<CommandBase> create_stop_command();

  /**
   * @brief Create a version request command.
   * @return std::unique_ptr<CommandBase> The created command
   */
  std::unique_ptr<CommandBase> create_version_command();

  // Component State
  text_sensor::TextSensor* text_sensor_{nullptr};     ///< Sensor for barcode output
  text_sensor::TextSensor* version_sensor_{nullptr};  ///< Sensor for firmware version

  std::vector<uint8_t> rx_buffer_;                           ///< Buffer for received data
  std::vector<std::unique_ptr<CommandBase>> command_queue_;  ///< Queue of pending commands

  bool scanning_{false};                            ///< Current scanning state
  bool waiting_for_ack_{false};                     ///< Whether waiting for command acknowledgment
  uint32_t last_command_time_{0};                   ///< Timestamp of last command sent
  CommandState command_state_{CommandState::IDLE};  ///< Current command processing state
  ResponseType expected_response_{ResponseType::NONE};  ///< Expected response type

  // Scanner Settings
  OperationMode operation_mode_{OperationMode::HOST};  ///< Current operation mode
  Terminator terminator_{Terminator::NONE};            ///< Current terminator setting
  LightMode light_mode_{LightMode::ON_WHEN_READING};   ///< Current light mode
  LocateLightMode locate_light_mode_{
      LocateLightMode::ON_WHEN_READING};                   ///< Current locate light mode
  SoundMode sound_mode_{SoundMode::SOUND_DISABLED};        ///< Current sound mode
  BuzzerVolume buzzer_volume_{BuzzerVolume::VOLUME_HIGH};  ///< Current buzzer volume
};

}  // namespace m5stack_barcode
}  // namespace esphome
