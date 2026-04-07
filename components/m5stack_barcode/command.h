#pragma once

#include <functional>
#include <memory>

#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

class BarcodeScanner;

/// A scanner command: UART bytes to transmit plus optional callbacks invoked by BarcodeScanner
/// on ACK (on_success) or timeout (on_failure).  All scanner state transitions live inside
/// BarcodeScanner — the callbacks are wired up by CommandFactory, which is the only site that
/// knows which setting to apply once the scanner acknowledges a command.
class Command {
 public:
  using Callback = std::function<void(BarcodeScanner *)>;

  Command(const uint8_t *data, size_t length, const char *description, Callback on_success = nullptr,
          Callback on_failure = nullptr, ResponseType expected_response = ResponseType::NONE)
      : data_(data),
        length_(length),
        description_(description),
        expected_response_(expected_response),
        on_success_(std::move(on_success)),
        on_failure_(std::move(on_failure)) {}

  const uint8_t *get_data() const { return this->data_; }
  size_t get_length() const { return this->length_; }
  const char *get_description() const { return this->description_; }
  ResponseType get_expected_response() const { return this->expected_response_; }
  void on_success(BarcodeScanner *scanner) {
    if (this->on_success_)
      this->on_success_(scanner);
  }
  void on_failure(BarcodeScanner *scanner) {
    if (this->on_failure_)
      this->on_failure_(scanner);
  }
  void log_command_data(const char *tag, const char *prefix) const;

 private:
  const uint8_t *data_;
  size_t length_;
  const char *description_;
  ResponseType expected_response_;
  Callback on_success_;
  Callback on_failure_;
};

/// Creates Command instances for every scanner operation.  The on_success lambdas call
/// BarcodeScanner::set_*_state(), which is why CommandFactory is declared a friend of
/// BarcodeScanner — it is the only external class that requires access to those methods.
class CommandFactory {
 public:
  static std::unique_ptr<Command> create_start_command();
  static std::unique_ptr<Command> create_stop_command();
  static std::unique_ptr<Command> create_version_command();

  static std::unique_ptr<Command> create_mode_command(OperationMode mode);
  static std::unique_ptr<Command> create_terminator_command(Terminator term);
  static std::unique_ptr<Command> create_light_command(LightMode mode);
  static std::unique_ptr<Command> create_locate_light_command(LocateLightMode mode);
  static std::unique_ptr<Command> create_sound_command(SoundMode mode);
  static std::unique_ptr<Command> create_volume_command(BuzzerVolume volume);
  static std::unique_ptr<Command> create_decoding_success_light_command(DecodingSuccessLightMode mode);
  static std::unique_ptr<Command> create_boot_sound_command(BootSoundMode mode);
  static std::unique_ptr<Command> create_decode_sound_command(DecodeSoundMode mode);
  static std::unique_ptr<Command> create_scan_duration_command(ScanDuration duration);
  static std::unique_ptr<Command> create_stable_induction_time_command(StableInductionTime time);
  static std::unique_ptr<Command> create_reading_interval_command(ReadingInterval interval);
  static std::unique_ptr<Command> create_same_code_interval_command(SameCodeInterval interval);
};

}  // namespace m5stack_barcode
}  // namespace esphome
