#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

class BarcodeScanner;

// Declare logging tag in UPPER_SNAKE_CASE for constants
extern const char* const TAG_ACTION;

// Basic control actions
template <typename... Ts>
class StartAction : public Action<Ts...> {
public:
  explicit StartAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

template <typename... Ts>
class StopAction : public Action<Ts...> {
public:
  explicit StopAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

// Setting actions
template <typename... Ts>
class SetModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  /**
   * Set the operation mode directly
   * @param mode The operation mode to set
   */
  void set_mode(OperationMode mode) { this->mode_ = mode; }

  /**
   * Set the operation mode using a string
   * @param mode_str String representation of the mode (e.g., "host", "level", etc.)
   */
  void set_mode(const std::string& mode_str) {
    if (mode_str == "host")
      this->mode_ = OperationMode::HOST;
    else if (mode_str == "level")
      this->mode_ = OperationMode::LEVEL;
    else if (mode_str == "pulse")
      this->mode_ = OperationMode::PULSE;
    else if (mode_str == "continuous")
      this->mode_ = OperationMode::CONTINUOUS;
    else if (mode_str == "auto_sense")
      this->mode_ = OperationMode::AUTO_SENSE;
    else
      ESP_LOGW(TAG_ACTION, "Unknown operation mode: %s", mode_str.c_str());
  }

  /**
   * Set the operation mode using a lambda function
   * Handles the lambda format used by ESPHome's templating system
   * @param f Lambda function that returns a string representation of the mode
   */
  template <typename F>
  void set_mode(F f) {
    // Lambda from ESPHome's templating system expects (std::string, size_t) -> std::string
    std::string value = f(std::string(), 0);
    this->set_mode(value);
  }

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  OperationMode mode_{OperationMode::HOST};  // Default to HOST mode
};

template <typename... Ts>
class SetTerminatorAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetTerminatorAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetTerminatorAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  /**
   * Set the terminator directly
   * @param term The terminator to set
   */
  void set_terminator(Terminator term) { this->terminator_ = term; }

  /**
   * Set the terminator using a string
   * @param term_str String representation of the terminator (e.g., "none", "crlf", etc.)
   */
  void set_terminator(const std::string& term_str) {
    if (term_str == "none")
      this->terminator_ = Terminator::NONE;
    else if (term_str == "crlf")
      this->terminator_ = Terminator::CRLF;
    else if (term_str == "cr")
      this->terminator_ = Terminator::CR;
    else if (term_str == "tab")
      this->terminator_ = Terminator::TAB;
    else if (term_str == "crcr")
      this->terminator_ = Terminator::CRCR;
    else if (term_str == "crlfcrlf")
      this->terminator_ = Terminator::CRLFCRLF;
    else
      ESP_LOGW(TAG_ACTION, "Unknown terminator: %s", term_str.c_str());
  }

  /**
   * Set the terminator using a lambda function
   * Handles the lambda format used by ESPHome's templating system
   * @param f Lambda function that returns a string representation of the terminator
   */
  template <typename F>
  void set_terminator(F f) {
    // Lambda from ESPHome's templating system expects (std::string, size_t) -> std::string
    std::string value = f(std::string(), 0);
    this->set_terminator(value);
  }

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  Terminator terminator_{Terminator::CRLF};  // Default to CRLF terminator
};

// Light control actions
template <typename... Ts>
class SetLightModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetLightModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetLightModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  /**
   * Set the light mode directly
   * @param mode The light mode to set
   */
  void set_mode(LightMode mode) { this->mode_ = mode; }

  /**
   * Set the light mode using a string
   * @param mode_str String representation of the mode (e.g., "on_when_reading", "always_on", etc.)
   */
  void set_mode(const std::string& mode_str) {
    if (mode_str == "on_when_reading")
      this->mode_ = LightMode::ON_WHEN_READING;
    else if (mode_str == "always_on")
      this->mode_ = LightMode::ALWAYS_ON;
    else if (mode_str == "always_off")
      this->mode_ = LightMode::ALWAYS_OFF;
    else
      ESP_LOGW(TAG_ACTION, "Unknown light mode: %s", mode_str.c_str());
  }

  /**
   * Set the light mode using a lambda function
   * Handles the lambda format used by ESPHome's templating system
   * @param f Lambda function that returns a string representation of the mode
   */
  template <typename F>
  void set_mode(F f) {
    // Lambda from ESPHome's templating system expects (std::string, size_t) -> std::string
    std::string value = f(std::string(), 0);
    this->set_mode(value);
  }

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  LightMode mode_{LightMode::ON_WHEN_READING};  // Default to ON_WHEN_READING
};

template <typename... Ts>
class SetLocateLightModeAction : public Action<Ts...> {
public:
  explicit SetLocateLightModeAction(BarcodeScanner* scanner)
      : scanner_(scanner), mode_(LocateLightMode::ON_WHEN_READING) {}
  void set_mode(LocateLightMode mode) { this->mode_ = mode; }

  // Add a string-based setter for YAML configuration
  void set_mode(const std::string& mode_str) {
    if (mode_str == "on_when_reading")
      this->mode_ = LocateLightMode::ON_WHEN_READING;
    else if (mode_str == "always_on")
      this->mode_ = LocateLightMode::ALWAYS_ON;
    else if (mode_str == "always_off")
      this->mode_ = LocateLightMode::ALWAYS_OFF;
    else
      ESP_LOGW(TAG_ACTION, "Unknown locate light mode: %s", mode_str.c_str());
  }

  // Add a template-based setter for lambda functions
  template <typename F>
  void set_mode(F f) {
    auto val = f(std::string(), 0);
    this->set_mode(val);
  }

  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  LocateLightMode mode_;
};

// Sound control actions
template <typename... Ts>
class SetSoundModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetSoundModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetSoundModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  /**
   * Set the sound mode directly
   * @param mode The sound mode to set
   */
  void set_mode(SoundMode mode) { this->mode_ = mode; }

  /**
   * Set the sound mode using a string
   * @param mode_str String representation of the mode (e.g., "disabled", "enabled")
   */
  void set_mode(const std::string& mode_str) {
    if (mode_str == "disabled")
      this->mode_ = SoundMode::SOUND_DISABLED;
    else if (mode_str == "enabled")
      this->mode_ = SoundMode::ENABLED;
    else
      ESP_LOGW(TAG_ACTION, "Unknown sound mode: %s", mode_str.c_str());
  }

  /**
   * Set the sound mode using a lambda function
   * Handles the lambda format used by ESPHome's templating system
   * @param f Lambda function that returns a string representation of the mode
   */
  template <typename F>
  void set_mode(F f) {
    // Lambda from ESPHome's templating system expects (std::string, size_t) -> std::string
    std::string value = f(std::string(), 0);
    this->set_mode(value);
  }

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  SoundMode mode_{SoundMode::SOUND_DISABLED};  // Default to SOUND_DISABLED
};

template <typename... Ts>
class SetBuzzerVolumeAction : public Action<Ts...> {
public:
  explicit SetBuzzerVolumeAction(BarcodeScanner* scanner)
      : scanner_(scanner), volume_(BuzzerVolume::VOLUME_HIGH) {}
  void set_volume(BuzzerVolume volume) { this->volume_ = volume; }

  // Add a string-based setter for YAML configuration
  void set_volume(const std::string& volume_str) {
    if (volume_str == "high")
      this->volume_ = BuzzerVolume::VOLUME_HIGH;
    else if (volume_str == "medium")
      this->volume_ = BuzzerVolume::MEDIUM;
    else if (volume_str == "low")
      this->volume_ = BuzzerVolume::VOLUME_LOW;
    else
      ESP_LOGW(TAG_ACTION, "Unknown buzzer volume: %s", volume_str.c_str());
  }

  // Add a template-based setter for lambda functions
  template <typename F>
  void set_volume(F f) {
    auto val = f(std::string(), 0);
    this->set_volume(val);
  }

  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
  BuzzerVolume volume_;
};

}  // namespace m5stack_barcode
}  // namespace esphome
