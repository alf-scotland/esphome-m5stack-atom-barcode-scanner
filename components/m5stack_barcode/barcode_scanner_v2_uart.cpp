#include "barcode_scanner_v2_uart.h"

#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Idle gap used to detect end-of-barcode output. V2 barcode bytes have no
// framing, so we rely on the inter-packet idle gap. At 115200 baud the
// inter-byte gap is ~87 µs; 20 ms is well above that while short enough
// that each barcode is processed promptly.
static const uint32_t V2_RX_IDLE_MS = 20;

// Safety cap for the raw RX accumulation buffer.
static const size_t V2_MAX_RX_BUFFER = 512;

// Timeout for HOST-mode scans when scan_duration is not UNLIMITED.
// The base class get_scan_duration_ms() returns 0 for UNLIMITED.

void BarcodeScannerV2UART::setup() {
  ESP_LOGCONFIG(TAG_SCANNER, "Setting up M5Stack Atomic QRCode2 Base (V2 UART)");

  // Drain any stale UART bytes from the hardware FIFO.
  while (this->available()) {
    this->read();
  }

  // Send the trigger mode read command to verify communication and read
  // the scanner's current state.
  this->write_array(CommandsV2::READ_TRIGGER_MODE, CommandsV2::READ_TRIGGER_MODE_SIZE);

  // Apply the configured trigger mode.
  this->apply_trigger_mode_(this->operation_mode_);
}

void BarcodeScannerV2UART::loop() {
  // Publish initial sub-component states on the first loop() tick.
  if (!this->initial_states_published_) {
    this->publish_initial_states_();
    if (this->scanning_binary_sensor_ != nullptr)
      this->scanning_binary_sensor_->publish_state(false);
    this->initial_states_published_ = true;
  }

  this->read_buffer_();
  this->process_rx_();

  // HOST-mode scan timeout.
  if (this->scan_state_ == ScanState::MANUAL_SCANNING && this->scan_started_at_ != 0) {
    const uint32_t duration_ms = this->get_scan_duration_ms();
    if (duration_ms > 0 && (millis() - this->scan_started_at_) > duration_ms) {
      ESP_LOGD(TAG_SCANNER, "[V2] Scan timed out after %u ms", duration_ms);
      this->scan_started_at_ = 0;
      this->set_scan_state(ScanState::IDLE);
      this->scan_timeout_callback_();
    }
  }
}

void BarcodeScannerV2UART::dump_config() {
  ESP_LOGCONFIG(TAG_SCANNER, "M5Stack Atomic QRCode2 Base (V2 UART):");
  ESP_LOGCONFIG(TAG_SCANNER, "  Trigger Mode: %s",
                (this->operation_mode_ == OperationMode::HOST) ? "manual" : "auto");
}

// ── Scanner control ─────────────────────────────────────────────────────────

void BarcodeScannerV2UART::start_scan() {
  if (this->operation_mode_ != OperationMode::HOST) {
    ESP_LOGW(TAG_SCANNER, "[V2] start_scan() only applies in HOST (manual) mode");
    return;
  }
  if (this->is_scanning()) {
    ESP_LOGD(TAG_SCANNER, "[V2] Scan already in progress");
    return;
  }
  ESP_LOGD(TAG_SCANNER, "[V2] Starting scan");
  this->write_array(CommandsV2::START_SCAN, CommandsV2::START_SCAN_SIZE);
  this->scan_started_at_ = millis();
  this->set_scan_state(ScanState::MANUAL_SCANNING);
}

void BarcodeScannerV2UART::stop_scan() {
  if (this->operation_mode_ != OperationMode::HOST) {
    ESP_LOGW(TAG_SCANNER, "[V2] stop_scan() only applies in HOST (manual) mode");
    return;
  }
  if (!this->is_scanning()) {
    ESP_LOGD(TAG_SCANNER, "[V2] No scan in progress");
    return;
  }
  ESP_LOGD(TAG_SCANNER, "[V2] Stopping scan");
  this->write_array(CommandsV2::STOP_SCAN, CommandsV2::STOP_SCAN_SIZE);
  this->scan_started_at_ = 0;
  this->set_scan_state(ScanState::IDLE);
  this->clear_buffer_();
}

void BarcodeScannerV2UART::factory_reset() {
  ESP_LOGW(TAG_SCANNER, "[V2] Factory reset requested — scanner will revert to hardware defaults");
  this->write_array(CommandsV2::FACTORY_RESET, CommandsV2::FACTORY_RESET_SIZE);
}

// ── Settings ─────────────────────────────────────────────────────────────────

void BarcodeScannerV2UART::set_operation_mode(OperationMode mode) {
  if (mode == this->operation_mode_) {
    return;
  }
  this->operation_mode_ = mode;
  this->apply_trigger_mode_(mode);

  if (mode == OperationMode::CONTINUOUS || mode == OperationMode::AUTO_SENSE) {
    this->set_scan_state(ScanState::CONTINUOUS_SCANNING);
  } else if (this->scan_state_ == ScanState::CONTINUOUS_SCANNING) {
    this->set_scan_state(ScanState::IDLE);
  }

  if (this->operation_mode_select_ != nullptr) {
    this->operation_mode_select_->publish_state(OperationModeSelect::to_key(mode));
  }
}

void BarcodeScannerV2UART::apply_trigger_mode_(OperationMode mode) {
  if (mode == OperationMode::HOST || mode == OperationMode::LEVEL || mode == OperationMode::PULSE) {
    ESP_LOGD(TAG_SCANNER, "[V2] Setting trigger mode: manual");
    this->write_array(CommandsV2::SET_TRIGGER_MANUAL, CommandsV2::SET_TRIGGER_MANUAL_SIZE);
  } else {
    ESP_LOGD(TAG_SCANNER, "[V2] Setting trigger mode: auto");
    this->write_array(CommandsV2::SET_TRIGGER_AUTO, CommandsV2::SET_TRIGGER_AUTO_SIZE);
  }
}

// ── Stubbed settings (V2 protocol details not yet confirmed) ─────────────────

void BarcodeScannerV2UART::set_terminator(Terminator term) {
  this->terminator_ = term;
}

void BarcodeScannerV2UART::set_light_mode(LightMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_light_mode: not implemented for V2 UART");
  this->light_mode_ = mode;
}

void BarcodeScannerV2UART::set_locate_light_mode(LocateLightMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_locate_light_mode: not implemented for V2 UART");
  this->locate_light_mode_ = mode;
}

void BarcodeScannerV2UART::set_sound_mode(SoundMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_sound_mode: not implemented for V2 UART");
  this->sound_mode_ = mode;
}

void BarcodeScannerV2UART::set_buzzer_volume(BuzzerVolume volume) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_buzzer_volume: not implemented for V2 UART");
  this->buzzer_volume_ = volume;
}

void BarcodeScannerV2UART::set_decoding_success_light_mode(DecodingSuccessLightMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_decoding_success_light_mode: not implemented for V2 UART");
  this->decoding_success_light_mode_ = mode;
}

void BarcodeScannerV2UART::set_boot_sound_mode(BootSoundMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_boot_sound_mode: not implemented for V2 UART");
  this->boot_sound_mode_ = mode;
}

void BarcodeScannerV2UART::set_decode_sound_mode(DecodeSoundMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_decode_sound_mode: not implemented for V2 UART");
  this->decode_sound_mode_ = mode;
}

void BarcodeScannerV2UART::set_scan_duration(ScanDuration duration) {
  this->scan_duration_ = duration;
}

void BarcodeScannerV2UART::set_stable_induction_time(StableInductionTime time) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_stable_induction_time: not implemented for V2 UART");
  this->stable_induction_time_ = time;
}

void BarcodeScannerV2UART::set_reading_interval(ReadingInterval interval) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_reading_interval: not implemented for V2 UART");
  this->reading_interval_ = interval;
}

void BarcodeScannerV2UART::set_same_code_interval(SameCodeInterval interval) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_same_code_interval: not implemented for V2 UART");
  this->same_code_interval_ = interval;
}

void BarcodeScannerV2UART::set_cmd_ack_sound_mode(CmdAckSoundMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_cmd_ack_sound_mode: not applicable to V2 UART");
  this->cmd_ack_sound_mode_ = mode;
}

void BarcodeScannerV2UART::set_config_code_scan_mode(ConfigCodeScanMode mode) {
  ESP_LOGW(TAG_SCANNER, "[V2] set_config_code_scan_mode: not applicable to V2 UART");
  this->config_code_scan_mode_ = mode;
}

// ── Buffer management ─────────────────────────────────────────────────────────

void BarcodeScannerV2UART::read_buffer_() {
  bool got_bytes = false;
  while (this->available()) {
    this->rx_buffer_.push_back(this->read());
    got_bytes = true;
  }
  if (got_bytes) {
    this->last_rx_time_ = millis();
  }
  if (this->rx_buffer_.size() > V2_MAX_RX_BUFFER) {
    ESP_LOGW(TAG_SCANNER, "[V2] RX buffer overflow (%u bytes); discarding", this->rx_buffer_.size());
    this->clear_buffer_();
  }
}

void BarcodeScannerV2UART::clear_buffer_() { this->rx_buffer_.clear(); }

// Returns true if the rx_buffer_ starts with a known protocol response prefix.
bool BarcodeScannerV2UART::is_known_response_prefix_() const {
  const size_t len = this->rx_buffer_.size();
  if (len == 0) {
    return false;
  }
  // Check for TYPE=0x22 (config write reply), TYPE=0x24 (config read reply), TYPE=0x33 (control reply)
  const uint8_t first = this->rx_buffer_[0];
  return first == CommandsV2::Responses::CONFIG_WRITE_REPLY_TYPE ||
         first == CommandsV2::Responses::TRIGGER_MODE_PREFIX[0] ||
         first == CommandsV2::Responses::STOP_OR_FAILED[0];
}

void BarcodeScannerV2UART::process_rx_() {
  if (this->rx_buffer_.empty()) {
    return;
  }

  const size_t len = this->rx_buffer_.size();
  const uint8_t *buf = this->rx_buffer_.data();

  // Check for stop-or-failed-decode response: {0x33, 0x75, 0x02}
  if (len >= CommandsV2::Responses::STOP_OR_FAILED_SIZE &&
      memcmp(buf, CommandsV2::Responses::STOP_OR_FAILED, CommandsV2::Responses::STOP_OR_FAILED_SIZE) == 0) {
    ESP_LOGD(TAG_SCANNER, "[V2] Received stop/failed-decode response");
    if (this->scan_state_ == ScanState::MANUAL_SCANNING) {
      this->scan_started_at_ = 0;
      this->set_scan_state(ScanState::IDLE);
      this->scan_timeout_callback_();
    }
    this->clear_buffer_();
    return;
  }

  // Check for trigger mode read reply: {0x24, 0x61, 0x41, value}
  if (len >= CommandsV2::Responses::TRIGGER_MODE_TOTAL &&
      memcmp(buf, CommandsV2::Responses::TRIGGER_MODE_PREFIX,
             CommandsV2::Responses::TRIGGER_MODE_PREFIX_SIZE) == 0) {
    const uint8_t mode_val = buf[CommandsV2::Responses::TRIGGER_MODE_PREFIX_SIZE];
    ESP_LOGD(TAG_SCANNER, "[V2] Trigger mode read reply: 0x%02X (%s)", mode_val,
             (mode_val == CommandsV2::TRIGGER_MODE_AUTO) ? "auto" : "manual");
    this->clear_buffer_();
    return;
  }

  // Check for config write reply: TYPE=0x22 + 2 more bytes
  if (len >= 3 && buf[0] == CommandsV2::Responses::CONFIG_WRITE_REPLY_TYPE) {
    ESP_LOGD(TAG_SCANNER, "[V2] Config write reply: PID=0x%02X FID=0x%02X", buf[1], buf[2]);
    this->clear_buffer_();
    return;
  }

  // Nothing matched — treat as barcode data if an idle gap has elapsed.
  // At 115200 baud one byte takes ~87 µs; 20 ms silence means all bytes arrived.
  if ((millis() - this->last_rx_time_) < V2_RX_IDLE_MS) {
    return;  // still receiving
  }

  // Process as barcode.  Ignore zero-length or protocol-looking data.
  if (len == 0 || this->is_known_response_prefix_()) {
    this->clear_buffer_();
    return;
  }

  std::string barcode(reinterpret_cast<const char *>(buf), len);
  ESP_LOGD(TAG_SCANNER, "[V2] Barcode received: %s", barcode.c_str());
  this->scan_started_at_ = 0;
  if (this->scan_state_ == ScanState::MANUAL_SCANNING) {
    this->set_scan_state(ScanState::IDLE);
  }
  this->fire_barcode_(barcode);
  this->clear_buffer_();
}

}  // namespace m5stack_barcode
}  // namespace esphome
