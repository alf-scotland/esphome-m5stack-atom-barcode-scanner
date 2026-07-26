#pragma once

#include <memory>
#include <vector>

#include "barcode_scanner_base.h"
#include "command.h"
#include "commands.h"

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

/// V1 M5Stack Atomic QRCode Scanner — UART at 9600 baud, checksum-framed command protocol.
///
/// Inherits all HA sub-component wiring and settings state from BarcodeScannerBase.
/// This class implements setup/loop and the UART wire protocol (command queue, ACK
/// handling, NVS persistence).
class BarcodeScanner : public BarcodeScannerBase {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // ── Pure-virtual overrides: scanner control ──────────────────────────────
  void start_scan() override;
  void stop_scan() override;
  void factory_reset() override;
  void process_current_buffer() override;

  // ── Pure-virtual overrides: settings ────────────────────────────────────
  void set_operation_mode(OperationMode mode) override;
  void set_terminator(Terminator term) override;
  void set_light_mode(LightMode mode) override;
  void set_locate_light_mode(LocateLightMode mode) override;
  void set_sound_mode(SoundMode mode) override;
  void set_buzzer_volume(BuzzerVolume volume) override;
  void set_decoding_success_light_mode(DecodingSuccessLightMode mode) override;
  void set_boot_sound_mode(BootSoundMode mode) override;
  void set_decode_sound_mode(DecodeSoundMode mode) override;
  void set_scan_duration(ScanDuration duration) override;
  void set_stable_induction_time(StableInductionTime time) override;
  void set_reading_interval(ReadingInterval interval) override;
  void set_same_code_interval(SameCodeInterval interval) override;
  void set_cmd_ack_sound_mode(CmdAckSoundMode mode) override;
  void set_config_code_scan_mode(ConfigCodeScanMode mode) override;

 protected:
  // CommandFactory lambdas call set_*_state() on ACK — the only external access needed.
  friend class CommandFactory;

  // Post-ACK state setters — called by CommandFactory lambdas after the scanner confirms a command.
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
  void set_operation_mode_state(OperationMode mode);

  void process_command_queue_();
  void do_factory_reset_();
  void queue_command(std::unique_ptr<Command> command);
  void wake_up_();
  bool is_ack_sequence_(const uint8_t *data, size_t len, size_t offset = 0) const;
  bool has_terminator_in_buffer_() const;
  void process_barcode_();
  void request_version_();
  void process_version_();
  void configure_defaults_();
  void save_settings_();
  void clear_buffer_();
  void read_buffer_();
  void set_expected_response_(ResponseType type);
  void write_command_(const std::unique_ptr<Command> &command);

  ESPPreferenceObject pref_;
  std::vector<uint8_t> rx_buffer_;
  std::vector<std::unique_ptr<Command>> command_queue_;

  bool waiting_for_ack_{false};
  bool initial_states_published_{false};
  uint32_t last_command_time_{0};
  uint32_t last_rx_time_{0};
  uint32_t scan_started_at_{0};
  CommandState command_state_{CommandState::IDLE};
  ResponseType expected_response_{ResponseType::NONE};
  uint8_t command_attempts_{0};
};

}  // namespace m5stack_barcode
}  // namespace esphome
