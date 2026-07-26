#pragma once

#include <vector>

#include "barcode_scanner_base.h"
#include "commands_v2.h"

namespace esphome {
namespace m5stack_barcode {

/// V2 (Atomic QRCode2 Base) barcode scanner — UART at 115200 baud, packet protocol.
///
/// Implements the BarcodeScannerBase interface over the V2 UART packet protocol.
/// Only trigger mode (operation_mode HOST / CONTINUOUS) and scan control are
/// fully supported in the initial implementation; light, sound, and timing
/// settings are accepted but log a warning and take no action pending
/// protocol documentation for those parameters.
class BarcodeScannerV2UART : public BarcodeScannerBase {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // ── Pure-virtual overrides: scanner control ──────────────────────────────
  void start_scan() override;
  void stop_scan() override;
  void factory_reset() override;

  // ── Pure-virtual overrides: settings ────────────────────────────────────
  // operation_mode: HOST→manual trigger, CONTINUOUS/AUTO_SENSE→auto trigger.
  // All other settings are stubbed (log warning, no UART command sent).
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
  void read_buffer_();
  void clear_buffer_();
  void process_rx_();
  bool is_known_response_prefix_() const;
  void apply_trigger_mode_(OperationMode mode);

  std::vector<uint8_t> rx_buffer_;
  uint32_t last_rx_time_{0};
  uint32_t scan_started_at_{0};
  bool initial_states_published_{false};
};

}  // namespace m5stack_barcode
}  // namespace esphome
