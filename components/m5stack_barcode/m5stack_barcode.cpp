#include "m5stack_barcode.h"

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <iterator>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5stack_barcode {

static const char *const TAG = "m5stack_barcode";

// Time constants (in milliseconds)
static const uint32_t WAKEUP_DELAY_MS = 50;       // Delay between wake-up and command send
static const uint32_t COMMAND_TIMEOUT_MS = 2000;  // Timeout for a command's reply
// At 9600 baud the scanner's full version response (~150 bytes) takes ~160 ms to transmit.
// The scanner also takes ~70 ms before it starts sending.  Use 300 ms from command-send
// so all bytes have arrived before we parse, avoiding the tail being misrouted as barcode data.
static const uint32_t VERSION_SETTLE_MS = 300;

static const size_t MAX_QUEUE_SIZE = 20;        // Maximum number of commands queued at once
static const uint8_t MAX_COMMAND_ATTEMPTS = 2;  // Send attempts before dropping a command (1 retry)

// Home Assistant rejects entity states longer than 255 characters, so longer codes are
// truncated to 255 bytes (on a character boundary).
static const size_t MAX_BARCODE_LENGTH = 255;
// Maximum version response length (binary header + full product info string)
static const size_t MAX_VERSION_LENGTH = 256;
// Safety cap for the raw RX accumulation buffer: the longest publishable barcode plus the
// 4-byte CRLFCRLF terminator, with margin. Anything longer is an oversized QR code or noise;
// the rest of that frame is discarded so no truncated fragment is published as a barcode.
static const size_t MAX_RX_BUFFER_SIZE = 512;
// Idle gap that marks the end of a response frame. At 9600 baud the inter-byte gap is ~1 ms;
// 20 ms is well above that while still being short enough that data is processed promptly.
// Used for barcodes without a terminator and to detect the end of the version response.
static const uint32_t RX_IDLE_WINDOW_MS = 20;
// Longest frame logged in full (every command frame fits)
static const size_t MAX_LOGGED_FRAME = 32;

// NVS keys for the current and the legacy (LEGACY_SETTINGS_VERSION) preference layouts
static const char *const PREFERENCES_KEY = "m5stack_barcode_settings";
static const char *const LEGACY_PREFERENCES_KEY = "m5stack_barcode";

static const SettingInfo &info(size_t index) { return get_setting_info(static_cast<SettingId>(index)); }

// Component lifecycle

void BarcodeScanner::setup() {
  // Drain bytes that reached the UART before we started: after an OTA reboot the scanner
  // stays powered and may be mid-output, which would corrupt the first replies.
  while (this->available()) {
    this->read();
  }
  // Allocate the buffers once so a long-running device does not fragment the heap.
  this->rx_buffer_.reserve(MAX_RX_BUFFER_SIZE + 64);
  this->command_queue_.reserve(MAX_QUEUE_SIZE);

  this->pref_ = global_preferences->make_preference<ScannerPreferences>(fnv1_hash(PREFERENCES_KEY));
  this->restore_settings_();

  // Continuous and auto-sense modes scan on their own from power-up.
  if (this->is_continuous_mode())
    this->scan_state_ = ScanState::CONTINUOUS_SCANNING;
  if (this->scanning_binary_sensor_ != nullptr)
    this->scanning_binary_sensor_->publish_state(this->scan_state_ != ScanState::IDLE);

  if (this->version_sensor_ != nullptr)
    this->queue_command_({CommandType::GET_VERSION});
}

void BarcodeScanner::loop() {
  // Publish the restored settings on the first loop() rather than in setup(): publishing
  // during setup races with the API handshake and makes HA reconnect repeatedly at boot.
  if (!this->initial_states_published_) {
    this->initial_states_published_ = true;
    // Settings sent at boot are published when the scanner ACKs them, so HA never shows a
    // value the scanner has not confirmed.
    for (size_t i = 0; i < NUM_SETTINGS; i++) {
      if (this->confirmed_at_boot_ & (1u << i))
        this->publish_setting_(static_cast<SettingId>(i));
    }
  }

  this->read_buffer_();

  if (this->command_state_ == CommandState::COMMAND_SENT) {
    if (this->command_queue_.front().type == CommandType::GET_VERSION) {
      this->handle_version_response_();
    } else {
      this->handle_ack_or_timeout_();
    }
  } else {
    // Barcode output has the same framing in every operation mode: the decoded data followed
    // by the configured terminator (or nothing), sent without any protocol header.
    if (this->discard_frame_) {
      // Drop everything up to the end of the oversized frame, then resume normal framing.
      if (millis() - this->last_rx_time_ >= RX_IDLE_WINDOW_MS) {
        ESP_LOGW(TAG, "Dropped oversized frame");
        this->discard_frame_ = false;
        this->rx_buffer_.clear();
      }
    } else if (this->has_complete_frame_()) {
      this->process_barcode_();
    }

    // HOST-mode scan timeout: the scanner stops after scan_duration without sending anything.
    if (this->scan_timer_active_) {
      const uint32_t duration_ms = this->get_scan_duration_ms();
      if (duration_ms > 0 && millis() - this->scan_started_at_ > duration_ms) {
        ESP_LOGD(TAG, "Scan timed out after %" PRIu32 " ms", duration_ms);
        this->scan_timer_active_ = false;
        this->set_scan_state_(ScanState::IDLE);
        this->scan_timeout_callback_.call();
      }
    }
  }

  this->process_command_queue_();
}

void BarcodeScanner::dump_config() {
  ESP_LOGCONFIG(TAG, "M5Stack Barcode Scanner:");
  for (size_t i = 0; i < NUM_SETTINGS; i++) {
    ESP_LOGCONFIG(TAG, "  %s: %s", info(i).key, info(i).values[this->settings_[i]]);
  }
  if (this->version_sensor_ != nullptr && this->version_sensor_->has_state()) {
    ESP_LOGCONFIG(TAG, "  Scanner firmware version: %s", this->version_sensor_->get_state().c_str());
  }
  LOG_TEXT_SENSOR("  ", "Barcode", this->barcode_sensor_);
  LOG_TEXT_SENSOR("  ", "Version", this->version_sensor_);
  LOG_BINARY_SENSOR("  ", "Scanning", this->scanning_binary_sensor_);
}

// Settings

// Every stored value must be an option of its setting: anything else was written by other
// firmware or is corrupt, and the whole entry is ignored.
static bool settings_in_range(const SettingValues &values) {
  for (size_t i = 0; i < NUM_SETTINGS; i++) {
    if (values[i] >= info(i).num_values)
      return false;
  }
  return true;
}

void BarcodeScanner::restore_settings_() {
  ScannerPreferences stored{};
  LegacyScannerPreferences legacy{};
  ESPPreferenceObject legacy_pref =
      global_preferences->make_preference<LegacyScannerPreferences>(fnv1_hash(LEGACY_PREFERENCES_KEY));
  // restore: the settings the scanner already has; baseline: the YAML values they were
  // resolved against (nullptr = the current YAML values).
  const SettingValues *restore = nullptr;
  const SettingValues *baseline = nullptr;
  bool migrated = false;
  if (this->pref_.load(&stored) && stored.version == SETTINGS_VERSION && settings_in_range(stored.applied) &&
      settings_in_range(stored.yaml)) {
    restore = &stored.applied;
    baseline = &stored.yaml;
    ESP_LOGD(TAG, "Restoring scanner settings from flash");
  } else if (legacy_pref.load(&legacy) && legacy.version == LEGACY_SETTINGS_VERSION &&
             settings_in_range(legacy.applied)) {
    // Older firmware re-applied every YAML value at boot, so its stored values differ from
    // the YAML ones only where they were changed at runtime since: keep those changes.
    restore = &legacy.applied;
    migrated = true;
    ESP_LOGI(TAG, "Migrating scanner settings from the previous preferences format");
  } else {
    ESP_LOGD(TAG, "No stored scanner settings; applying all configured values");
  }

  this->yaml_settings_ = this->settings_;
  for (size_t i = 0; i < NUM_SETTINGS; i++) {
    // Keep the stored value unless the YAML value was edited since it was stored (or nothing
    // is stored), in which case the YAML value is sent to the scanner.
    if (restore != nullptr && (baseline == nullptr || (*baseline)[i] == this->yaml_settings_[i])) {
      this->settings_[i] = (*restore)[i];
      this->confirmed_at_boot_ |= 1u << i;
    } else {
      this->queue_command_({CommandType::SETTING, static_cast<SettingId>(i), this->settings_[i]});
    }
  }

  // Persist the new YAML baseline (and the migrated values); unchanged data is not rewritten.
  this->save_settings_();
  if (migrated) {
    LegacyScannerPreferences empty{};
    legacy_pref.save(&empty);
  }
}

void BarcodeScanner::save_settings_() {
  ScannerPreferences prefs{};
  prefs.version = SETTINGS_VERSION;
  prefs.applied = this->settings_;
  prefs.yaml = this->yaml_settings_;
  if (!this->pref_.save(&prefs)) {
    ESP_LOGW(TAG, "Failed to save scanner preferences to NVS");
  }
}

void BarcodeScanner::publish_setting_(SettingId id) {
  const size_t i = static_cast<size_t>(id);
  if (this->selects_[i] != nullptr)
    this->selects_[i]->publish_state(static_cast<size_t>(this->settings_[i]));
  if (this->switches_[i] != nullptr)
    this->switches_[i]->publish_state(this->settings_[i] != 0);
}

void BarcodeScanner::apply_setting_(SettingId id, uint8_t value) {
  const size_t i = static_cast<size_t>(id);
  ESP_LOGD(TAG, "%s set to %s", info(i).key, info(i).values[value]);
  const bool changed = this->settings_[i] != value;
  this->settings_[i] = value;
  this->save_settings_();
  if (id == SettingId::OPERATION_MODE && changed) {
    // Continuous/auto-sense scan on their own; leaving them (or host mode mid-scan) ends any
    // scan.  Re-applying the same mode (e.g. at boot) leaves a scan that was just started alone.
    this->scan_timer_active_ = false;
    this->set_scan_state_(this->is_continuous_mode() ? ScanState::CONTINUOUS_SCANNING : ScanState::IDLE);
  }
  this->publish_setting_(id);
}

void BarcodeScanner::set_setting(SettingId id, uint8_t value) {
  const size_t i = static_cast<size_t>(id);
  if (i >= NUM_SETTINGS || value >= info(i).num_values) {
    ESP_LOGW(TAG, "Ignoring invalid value %u for setting %u", value, static_cast<unsigned>(i));
    return;
  }
  this->queue_setting_(id, value);
}

void BarcodeScanner::set_setting(SettingId id, const std::string &key) {
  const SettingInfo &setting = get_setting_info(id);
  uint8_t value;
  if (!setting.parse(key, value)) {
    ESP_LOGW(TAG, "Ignoring invalid %s value '%s'", setting.key, key.c_str());
    return;
  }
  this->queue_setting_(id, value);
}

void BarcodeScanner::queue_setting_(SettingId id, uint8_t value) {
  const auto same_setting = [id](const Command &command) {
    return command.type == CommandType::SETTING && command.setting == id;
  };
  // A newer value replaces one for the same setting that has not been sent yet.
  const bool sending = this->command_state_ != CommandState::IDLE;
  const bool in_flight = sending && same_setting(this->command_queue_.front());
  const auto superseded =
      std::remove_if(this->command_queue_.begin() + (sending ? 1 : 0), this->command_queue_.end(), same_setting);
  const bool was_queued = superseded != this->command_queue_.end();
  this->command_queue_.erase(superseded, this->command_queue_.end());
  // Skip the command only when the scanner already has the value: when a command for this
  // setting is queued or in flight, the current value is not what the scanner will end up
  // with (e.g. a YAML value queued at boot, or a change being reverted before its ACK).
  if (value == this->get_setting(id) && !in_flight && !was_queued) {
    ESP_LOGD(TAG, "%s already set to %s", get_setting_info(id).key, get_setting_info(id).values[value]);
    return;
  }
  this->queue_command_({CommandType::SETTING, id, value});
}

// Commands

const uint8_t *BarcodeScanner::command_frame_(const Command &command) const {
  switch (command.type) {
    case CommandType::SETTING:
      return get_setting_info(command.setting).frame(command.value);
    case CommandType::START_SCAN:
      return frames::START_SCAN;
    case CommandType::STOP_SCAN:
      return frames::STOP_SCAN;
    case CommandType::GET_VERSION:
      return frames::GET_VERSION;
    case CommandType::FACTORY_RESET:
    default:
      return frames::FACTORY_RESET;
  }
}

void BarcodeScanner::log_command_(const char *action, const Command &command) const {
  static const char *const NAMES[] = {"", "start scan", "stop scan", "get version", "factory reset"};
  const uint8_t *frame = this->command_frame_(command);
  char hex[format_hex_pretty_size(MAX_LOGGED_FRAME)];
  format_hex_pretty_to(hex, frame, std::min(frame_length(frame), MAX_LOGGED_FRAME), ' ');
  if (command.type == CommandType::SETTING) {
    const SettingInfo &setting = get_setting_info(command.setting);
    ESP_LOGD(TAG, "%s %s=%s: %s", action, setting.key, setting.values[command.value], hex);
  } else {
    ESP_LOGD(TAG, "%s %s: %s", action, NAMES[static_cast<size_t>(command.type)], hex);
  }
}

void BarcodeScanner::queue_command_(const Command &command) {
  if (this->command_queue_.size() >= MAX_QUEUE_SIZE) {
    ESP_LOGW(TAG, "Command queue full, dropping a command");
    return;
  }
  this->log_command_("Queuing", command);
  this->command_queue_.push_back(command);
}

void BarcodeScanner::process_command_queue_() {
  if (this->command_queue_.empty() || this->command_state_ == CommandState::COMMAND_SENT)
    return;
  // Never interleave commands with a scanner that is mid-output: the reply would be mixed
  // into barcode data.  The frame is processed by loop() within RX_IDLE_WINDOW_MS.
  if (this->command_state_ == CommandState::IDLE && !this->rx_buffer_.empty())
    return;

  if (this->command_state_ == CommandState::IDLE) {
    this->write_byte(frames::WAKEUP);
    this->last_command_time_ = millis();
    this->command_state_ = CommandState::WAKEUP_SENT;
  } else if (millis() - this->last_command_time_ >= WAKEUP_DELAY_MS) {
    const Command &command = this->command_queue_.front();
    const uint8_t *frame = this->command_frame_(command);
    this->command_attempts_++;
    this->log_command_("Sending", command);
    this->write_array(frame, frame_length(frame));
    this->last_command_time_ = millis();
    this->command_state_ = CommandState::COMMAND_SENT;
  }
}

void BarcodeScanner::handle_ack_or_timeout_() {
  // Setting commands are ACKed in every operation mode.  Search the whole buffer: barcode
  // bytes from a continuous-mode scan may precede the ACK, and a HOST-mode start ACK may be
  // followed by the barcode in the same read.  Only the reply is consumed; surrounding data
  // stays for barcode framing.
  auto ack =
      std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), std::begin(frames::ACK), std::end(frames::ACK));
  if (ack != this->rx_buffer_.end()) {
    this->rx_buffer_.erase(ack, ack + std::size(frames::ACK));
    ESP_LOGD(TAG, "Command acknowledged");
    this->status_clear_warning();
    this->finish_command_(true);
    return;
  }

  auto nak = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), std::begin(frames::NAK_PREFIX),
                         std::end(frames::NAK_PREFIX));
  if (nak != this->rx_buffer_.end() && this->rx_buffer_.end() - nak >= static_cast<ptrdiff_t>(frames::NAK_LENGTH)) {
    // The scanner rejected the command; resending it would be rejected again.
    const uint8_t cause = nak[frames::NAK_CAUSE_INDEX];
    this->rx_buffer_.erase(nak, nak + frames::NAK_LENGTH);
    this->log_command_("Scanner rejected", this->command_queue_.front());
    ESP_LOGW(TAG, "Command rejected (cause 0x%02X)", cause);
    this->status_clear_warning();
    this->finish_command_(false);
    return;
  }

  if (millis() - this->last_command_time_ <= COMMAND_TIMEOUT_MS)
    return;

  if (this->command_attempts_ < MAX_COMMAND_ATTEMPTS) {
    ESP_LOGD(TAG, "No reply (attempt %u/%u), retrying", this->command_attempts_, MAX_COMMAND_ATTEMPTS);
    this->command_state_ = CommandState::IDLE;  // resend with a fresh wake-up
    return;
  }
  this->log_command_("No reply to", this->command_queue_.front());
  ESP_LOGW(TAG, "Scanner not responding; command dropped after %u attempts", this->command_attempts_);
  this->status_set_warning(LOG_STR("Scanner not responding"));
  this->finish_command_(false);
}

void BarcodeScanner::handle_version_response_() {
  // The version response is an unframed burst of ~150 bytes (~160 ms at 9600 baud) that the
  // scanner starts sending ~70 ms after the request.  Wait at least VERSION_SETTLE_MS and until
  // the line has been idle, so the tail is never misrouted as barcode data.  If nothing arrives
  // at all, give up after COMMAND_TIMEOUT_MS so the queue is not blocked indefinitely.
  const uint32_t now = millis();
  if (this->rx_buffer_.empty()) {
    if (now - this->last_command_time_ <= COMMAND_TIMEOUT_MS)
      return;
    ESP_LOGW(TAG, "Version request timed out with no response");
    this->status_set_warning(LOG_STR("Scanner not responding"));
    this->finish_command_(false);
    return;
  }
  if (now - this->last_command_time_ < VERSION_SETTLE_MS || now - this->last_rx_time_ < RX_IDLE_WINDOW_MS)
    return;
  this->process_version_();
  this->status_clear_warning();
  this->finish_command_(true);
}

void BarcodeScanner::finish_command_(bool success) {
  const Command command = this->command_queue_.front();
  this->command_queue_.erase(this->command_queue_.begin());
  this->command_state_ = CommandState::IDLE;
  this->command_attempts_ = 0;

  switch (command.type) {
    case CommandType::SETTING:
      if (success)
        this->apply_setting_(command.setting, command.value);
      break;
    case CommandType::START_SCAN:
      if (!success) {
        this->set_scan_state_(ScanState::IDLE);
      } else if (this->scan_state_ == ScanState::MANUAL_SCANNING) {
        // The scanner runs its scan_duration timer from now.  (A barcode that arrived together
        // with the ACK has already ended the scan.)
        this->scan_started_at_ = millis();
        this->scan_timer_active_ = true;
      }
      break;
    case CommandType::FACTORY_RESET:
      if (success) {
        // Invalidate the stored settings, so the next boot re-sends every YAML value.
        ScannerPreferences empty{};
        if (!this->pref_.save(&empty))
          ESP_LOGW(TAG, "Failed to invalidate NVS preferences; settings may not fully re-sync after reboot");
        ESP_LOGW(TAG, "Scanner factory reset acknowledged, rebooting to re-apply YAML settings");
        App.safe_reboot();
      }
      break;
    case CommandType::STOP_SCAN:
    case CommandType::GET_VERSION:
      break;
  }
}

// Scanning

bool BarcodeScanner::is_continuous_mode() const {
  const OperationMode mode = this->get_operation_mode();
  return mode == OperationMode::CONTINUOUS || mode == OperationMode::AUTO_SENSE;
}

void BarcodeScanner::set_scan_state_(ScanState state) {
  if (this->scan_state_ == state)
    return;
  static const char *const NAMES[] = {"idle", "manual scanning", "continuous scanning"};
  ESP_LOGD(TAG, "Scan state: %s", NAMES[static_cast<size_t>(state)]);
  this->scan_state_ = state;
  if (this->scanning_binary_sensor_ != nullptr)
    this->scanning_binary_sensor_->publish_state(state != ScanState::IDLE);
}

void BarcodeScanner::start_scan() {
  if (this->get_operation_mode() != OperationMode::HOST) {
    ESP_LOGW(TAG, "Cannot start a scan outside host mode");
    return;
  }
  if (this->is_scanning()) {
    ESP_LOGD(TAG, "Scan already in progress");
    return;
  }
  this->queue_command_({CommandType::START_SCAN});
  // Mark the scan as active now so repeated start requests are rejected.  The timeout starts
  // once the scanner ACKs (see finish_command_()).
  this->set_scan_state_(ScanState::MANUAL_SCANNING);
}

void BarcodeScanner::stop_scan() {
  if (this->get_operation_mode() != OperationMode::HOST) {
    ESP_LOGW(TAG, "Cannot stop a scan outside host mode");
    return;
  }
  if (!this->is_scanning()) {
    ESP_LOGD(TAG, "No scan in progress");
    return;
  }
  this->queue_command_({CommandType::STOP_SCAN});
  this->scan_timer_active_ = false;
  this->set_scan_state_(ScanState::IDLE);
}

void BarcodeScanner::factory_reset() {
  ESP_LOGW(TAG, "Factory reset requested: the scanner reverts to its defaults and the ESP reboots");
  this->queue_command_({CommandType::FACTORY_RESET});
}

void BarcodeScanner::process_current_buffer() {
  // Never treat a pending reply as barcode data.
  if (this->command_state_ == CommandState::COMMAND_SENT || this->discard_frame_)
    return;
  this->read_buffer_();
  this->process_barcode_();
}

// Receiving

void BarcodeScanner::read_buffer_() {
  bool got_bytes = false;
  uint8_t byte;
  while (this->available() && this->read_byte(&byte)) {
    this->rx_buffer_.push_back(byte);
    got_bytes = true;
  }
  if (got_bytes)
    this->last_rx_time_ = millis();
  if (this->rx_buffer_.size() > MAX_RX_BUFFER_SIZE) {
    ESP_LOGW(TAG, "RX buffer overflow (%zu bytes); discarding frame", this->rx_buffer_.size());
    this->rx_buffer_.clear();
    // Drop the rest of the frame too, up to the next idle gap, also when it overflowed while a
    // reply was awaited: otherwise its tail would be published as a barcode.  (A reply lost
    // in the dropped bytes is retried after the timeout.)
    this->discard_frame_ = true;
  }
}

size_t BarcodeScanner::terminator_length_in_buffer_() const {
  const char *term = terminator_bytes(this->get_setting(SettingId::TERMINATOR));
  const size_t term_len = strlen(term);
  const size_t len = this->rx_buffer_.size();
  if (term_len == 0 || len < term_len)
    return 0;
  return memcmp(this->rx_buffer_.data() + len - term_len, term, term_len) == 0 ? term_len : 0;
}

bool BarcodeScanner::has_complete_frame_() const {
  if (this->rx_buffer_.empty())
    return false;
  // A frame ends with the configured terminator.  The idle gap also ends it, which covers
  // terminator: none and a scanner whose terminator was changed behind our back (e.g. by
  // scanning a configuration barcode).
  return this->terminator_length_in_buffer_() > 0 || (millis() - this->last_rx_time_) >= RX_IDLE_WINDOW_MS;
}

// Copy data into a string, replacing each invalid UTF-8 sequence (maximal subpart) with U+FFFD.
// aioesphomeapi decodes states with google.protobuf, which rejects invalid UTF-8 — including
// overlong encodings and UTF-16 surrogates — and drops the API connection (HA reconnect loop).
// Barcodes are often Latin-1 or GBK, so replacing keeps the rest of the code usable.
static std::string sanitize_utf8(const uint8_t *data, size_t len, size_t &replaced) {
  static const char REPLACEMENT[] = "\xEF\xBF\xBD";
  std::string out;
  out.reserve(len);
  replaced = 0;
  size_t i = 0;
  while (i < len) {
    const uint8_t lead = data[i];
    if (lead < 0x80) {
      out += static_cast<char>(lead);
      i++;
      continue;
    }
    size_t continuation;
    uint8_t lo = 0x80, hi = 0xBF;  // allowed range of the first continuation byte
    if (lead >= 0xC2 && lead <= 0xDF) {
      continuation = 1;
    } else if (lead >= 0xE0 && lead <= 0xEF) {
      continuation = 2;
      lo = lead == 0xE0 ? 0xA0 : 0x80;  // no overlong forms
      hi = lead == 0xED ? 0x9F : 0xBF;  // no surrogates
    } else if (lead >= 0xF0 && lead <= 0xF4) {
      continuation = 3;
      lo = lead == 0xF0 ? 0x90 : 0x80;  // no overlong forms
      hi = lead == 0xF4 ? 0x8F : 0xBF;  // nothing above U+10FFFF
    } else {
      out += REPLACEMENT;
      replaced++;
      i++;
      continue;
    }
    size_t n = 1;
    while (n <= continuation && i + n < len) {
      const uint8_t byte = data[i + n];
      if (byte < (n == 1 ? lo : 0x80) || byte > (n == 1 ? hi : 0xBF))
        break;
      n++;
    }
    if (n == continuation + 1) {
      out.append(reinterpret_cast<const char *>(data + i), n);
    } else {
      out += REPLACEMENT;
      replaced++;
    }
    i += n;
  }
  return out;
}

void BarcodeScanner::process_barcode_() {
  if (this->rx_buffer_.empty())
    return;
  const size_t data_length = this->rx_buffer_.size() - this->terminator_length_in_buffer_();
  size_t replaced;
  std::string barcode = sanitize_utf8(this->rx_buffer_.data(), data_length, replaced);
  this->rx_buffer_.clear();
  if (barcode.empty())
    return;
  if (replaced > 0) {
    ESP_LOGW(TAG, "Barcode is not valid UTF-8; replaced %zu invalid sequence(s) with U+FFFD", replaced);
  }
  if (barcode.size() > MAX_BARCODE_LENGTH) {
    ESP_LOGW(TAG, "Barcode too long (%zu bytes), truncating to %zu bytes", barcode.size(), MAX_BARCODE_LENGTH);
    size_t cut = MAX_BARCODE_LENGTH;
    while (cut > 0 && (static_cast<uint8_t>(barcode[cut]) & 0xC0) == 0x80)
      cut--;  // never split a multi-byte character
    barcode.resize(cut);
  }

  // A HOST-mode scan is complete once its barcode arrives.
  this->scan_timer_active_ = false;
  if (this->scan_state_ == ScanState::MANUAL_SCANNING)
    this->set_scan_state_(ScanState::IDLE);

  if (barcode.rfind(CONFIG_CODE_PREFIX, 0) == 0) {
    this->apply_config_code_(barcode.substr(strlen(CONFIG_CODE_PREFIX)));
    return;
  }
  ESP_LOGD(TAG, "Barcode received: %s", barcode.c_str());

  if (this->barcode_sensor_ != nullptr)
    this->barcode_sensor_->publish_state(barcode);
  this->barcode_callback_.call(barcode);
  if (this->scan_event_ != nullptr)
    this->scan_event_->trigger("scan_successful");
}

void BarcodeScanner::apply_config_code_(const std::string &code) {
  // The scanner passed a configuration barcode on instead of applying it (config_code_scan_mode
  // disabled).  Apply it over UART, so the change is confirmed and published like any other.
  if (code == FACTORY_RESET_CONFIG_CODE) {
    ESP_LOGI(TAG, "Configuration barcode: factory reset");
    this->factory_reset();
    return;
  }
  SettingId id;
  uint8_t value;
  if (!find_config_code(code, id, value)) {
    ESP_LOGW(TAG, "Ignoring configuration barcode %s%s: not a setting this component manages", CONFIG_CODE_PREFIX,
             code.c_str());
    return;
  }
  const SettingInfo &setting = get_setting_info(id);
  ESP_LOGI(TAG, "Configuration barcode: %s = %s", setting.key, setting.values[value]);
  this->set_setting(id, value);
}

void BarcodeScanner::process_version_() {
  if (this->rx_buffer_.size() > MAX_VERSION_LENGTH) {
    ESP_LOGW(TAG, "Version response too long (%zu bytes), truncating to %zu bytes", this->rx_buffer_.size(),
             MAX_VERSION_LENGTH);
    this->rx_buffer_.resize(MAX_VERSION_LENGTH);
  }
  char hex[format_hex_pretty_size(MAX_LOGGED_FRAME)];
  format_hex_pretty_to(hex, this->rx_buffer_.data(), std::min(this->rx_buffer_.size(), MAX_LOGGED_FRAME), ' ');
  ESP_LOGD(TAG, "Version response (%zu bytes): %s%s", this->rx_buffer_.size(), hex,
           this->rx_buffer_.size() > MAX_LOGGED_FRAME ? " ..." : "");

  // The scanner response is a binary-framed product info string, e.g.:
  //   \x58\xA4\x00\x00Product Name:SE630 Product ID:... Hardware version:1.0 Firmware version:2.2.18\xe4D
  // Strategy: scan every contiguous printable-ASCII (0x20-0x7E) run in the buffer.
  // If any run contains "Firmware version:", extract the value that follows the colon.
  // Fall back to the last-run colon-value only if the key is not found.
  static const char FW_KEY[] = "Firmware version:";
  static const size_t FW_KEY_LEN = sizeof(FW_KEY) - 1;

  std::string version;
  size_t fallback_val_start = std::string::npos;
  size_t fallback_val_end = 0;

  size_t i = 0;
  while (i < this->rx_buffer_.size() && version.empty()) {
    // Skip non-printable bytes
    while (i < this->rx_buffer_.size() && (this->rx_buffer_[i] < 0x20 || this->rx_buffer_[i] > 0x7E)) {
      i++;
    }
    if (i >= this->rx_buffer_.size())
      break;

    size_t run_start = i;
    while (i < this->rx_buffer_.size() && this->rx_buffer_[i] >= 0x20 && this->rx_buffer_[i] <= 0x7E) {
      i++;
    }
    size_t run_end = i;
    size_t run_len = run_end - run_start;

    // Search for "Firmware version:" within this run
    if (run_len >= FW_KEY_LEN) {
      for (size_t j = run_start; j + FW_KEY_LEN <= run_end; j++) {
        if (memcmp(this->rx_buffer_.data() + j, FW_KEY, FW_KEY_LEN) == 0) {
          size_t val_start = j + FW_KEY_LEN;
          size_t val_end = run_end;
          while (val_end > val_start && this->rx_buffer_[val_end - 1] == ' ')
            val_end--;
          if (val_end > val_start) {
            version.assign(reinterpret_cast<char *>(this->rx_buffer_.data() + val_start), val_end - val_start);
          }
          break;
        }
      }
    }

    // Update fallback: last colon-value found in any run
    for (size_t j = run_end; j-- > run_start;) {
      if (this->rx_buffer_[j] == ':') {
        fallback_val_start = j + 1;
        fallback_val_end = run_end;
        break;
      }
    }
  }

  // If "Firmware version:" key was not found, fall back to the last colon-value in any run
  if (version.empty() && fallback_val_start != std::string::npos && fallback_val_end > fallback_val_start) {
    size_t vs = fallback_val_start;
    size_t ve = fallback_val_end;
    while (vs < ve && this->rx_buffer_[vs] == ' ')
      vs++;
    while (ve > vs && this->rx_buffer_[ve - 1] == ' ')
      ve--;
    if (ve > vs) {
      version.assign(reinterpret_cast<char *>(this->rx_buffer_.data() + vs), ve - vs);
    }
  }

  if (this->version_sensor_ != nullptr) {
    if (!version.empty()) {
      ESP_LOGD(TAG, "Publishing version: '%s'", version.c_str());
      this->version_sensor_->publish_state(version);
    } else {
      ESP_LOGW(TAG, "Version response yielded no publishable string (%zu raw bytes)", this->rx_buffer_.size());
    }
  }

  this->rx_buffer_.clear();
}

}  // namespace m5stack_barcode
}  // namespace esphome
