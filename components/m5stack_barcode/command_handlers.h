#pragma once

#include <functional>
#include <memory>
#include <string>

#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Forward declarations
class BarcodeScanner;

/**
 * Base class for all command handlers
 */
class CommandBase {
 public:
  // Constructor and virtual destructor
  CommandBase() = default;
  virtual ~CommandBase() = default;

  // Command data access
  virtual const uint8_t *get_data() const = 0;

  // Return the command length
  virtual size_t get_length() const = 0;

  // Success callback - called when acknowledgment is received
  virtual void on_success(BarcodeScanner *scanner) = 0;

  // Failure callback - called when command times out
  virtual void on_failure(BarcodeScanner *scanner) = 0;

  // Get command description for logging
  virtual const char *get_description() const = 0;

  // Get the expected response type for this command
  virtual ResponseType get_expected_response() const { return ResponseType::NONE; }

  // Helper to log command data in hex
  void log_command_data(const char *tag, const char *prefix) const;
};

/**
 * Generic state command for setting scanner properties
 * Template parameter is the setting type (e.g., OperationMode, Terminator)
 */
template<typename ValueType> class StateCommand : public CommandBase {
 public:
  StateCommand(const uint8_t *data, size_t length, ValueType value, const char *description)
      : data_(data), length_(length), value_(value), description_(description) {}

  const uint8_t *get_data() const override { return this->data_; }
  size_t get_length() const override { return this->length_; }
  const char *get_description() const override { return this->description_; }

  // Each specialization will implement this for type-specific state updates
  void on_success(BarcodeScanner *scanner) override;

  void on_failure(BarcodeScanner *scanner) override {
    // Default failure handling - no state changes on failure
  }

 protected:
  const uint8_t *data_;
  size_t length_;
  ValueType value_;
  const char *description_;
};

/**
 * Specialized command for operations that don't update state
 * (e.g., start scan, stop scan, get version)
 */
class SimpleCommand : public CommandBase {
 public:
  using SuccessCallback = std::function<void(BarcodeScanner *)>;
  using FailureCallback = std::function<void(BarcodeScanner *)>;

  SimpleCommand(const uint8_t *data, size_t length, const char *description, SuccessCallback on_success = nullptr,
                FailureCallback on_failure = nullptr, ResponseType expected_response = ResponseType::NONE);

  const uint8_t *get_data() const override { return this->data_; }
  size_t get_length() const override { return this->length_; }
  void on_success(BarcodeScanner *scanner) override { this->on_success_(scanner); }
  void on_failure(BarcodeScanner *scanner) override { this->on_failure_(scanner); }
  const char *get_description() const override { return this->description_; }
  ResponseType get_expected_response() const override { return this->expected_response_; }

 protected:
  const uint8_t *data_;
  size_t length_;
  const char *description_;
  ResponseType expected_response_;
  SuccessCallback on_success_;
  FailureCallback on_failure_;
};

/**
 * Factory class to create commands of different types
 */
class CommandFactory {
  friend class BarcodeScanner;

 public:
  // Scanner basic commands
  static std::unique_ptr<CommandBase> create_start_command();
  static std::unique_ptr<CommandBase> create_stop_command();
  static std::unique_ptr<CommandBase> create_version_command();

  // Setting commands
  static std::unique_ptr<CommandBase> create_mode_command(OperationMode mode);
  static std::unique_ptr<CommandBase> create_terminator_command(Terminator term);
  static std::unique_ptr<CommandBase> create_light_command(LightMode mode);
  static std::unique_ptr<CommandBase> create_locate_light_command(LocateLightMode mode);
  static std::unique_ptr<CommandBase> create_sound_command(SoundMode mode);
  static std::unique_ptr<CommandBase> create_volume_command(BuzzerVolume volume);
  static std::unique_ptr<CommandBase> create_decoding_success_light_command(DecodingSuccessLightMode mode);
  static std::unique_ptr<CommandBase> create_boot_sound_command(BootSoundMode mode);
  static std::unique_ptr<CommandBase> create_decode_sound_command(DecodeSoundMode mode);
  static std::unique_ptr<CommandBase> create_scan_duration_command(ScanDuration duration);
  static std::unique_ptr<CommandBase> create_stable_induction_time_command(StableInductionTime time);
  static std::unique_ptr<CommandBase> create_reading_interval_command(ReadingInterval interval);
  static std::unique_ptr<CommandBase> create_same_code_interval_command(SameCodeInterval interval);
};

// Forward declare all StateCommand specializations
template<> void StateCommand<Terminator>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<LightMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<LocateLightMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<SoundMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<BuzzerVolume>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<DecodingSuccessLightMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<BootSoundMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<DecodeSoundMode>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<ScanDuration>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<StableInductionTime>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<ReadingInterval>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<SameCodeInterval>::on_success(BarcodeScanner *scanner);
template<> void StateCommand<OperationMode>::on_success(BarcodeScanner *scanner);

}  // namespace m5stack_barcode
}  // namespace esphome
