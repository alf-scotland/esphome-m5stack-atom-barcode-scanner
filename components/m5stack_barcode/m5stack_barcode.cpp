#include "m5stack_barcode.h"

#include <algorithm>
#include <cinttypes>
#include <cstring>

#include "command.h"
#include "commands.h"
#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Logging tag for this component
static const char *const TAG = "m5stack_barcode";
const char *const TAG_SCANNER = TAG;

// Time constants (in milliseconds)
static const uint32_t WAKEUP_DELAY_MS = 50;       // Delay between wake-up and command send
static const uint32_t COMMAND_TIMEOUT_MS = 2000;  // Timeout for command acknowledgment
// At 9600 baud the scanner's full version response (~150 bytes) takes ~160 ms to transmit.
// The scanner also takes ~70 ms before it starts sending.  Use 300 ms from command-send
// so all bytes have arrived before we parse, avoiding the tail being misrouted as barcode data.
static const uint32_t VERSION_SETTLE_MS = 300;

// Command queue size
static const size_t MAX_QUEUE_SIZE = 20;        // Maximum number of commands queued at once
static const uint8_t MAX_COMMAND_ATTEMPTS = 2;  // Send attempts before dropping a command (1 retry)

// Data size limits
// Home Assistant rejects entity states longer than 255 characters, so longer codes are truncated.
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

bool BarcodeScanner::is_continuous_mode() const {
  return this->operation_mode_ == OperationMode::CONTINUOUS || this->operation_mode_ == OperationMode::AUTO_SENSE;
}

void BarcodeScanner::set_scan_state(ScanState state) {
  if (this->scan_state_ != state) {
    ESP_LOGD(TAG_SCANNER, "Scan state changed from %s to %s", scan_state_to_string(this->scan_state_),
             scan_state_to_string(state));
    this->scan_state_ = state;
    if (this->scanning_binary_sensor_ != nullptr)
      this->scanning_binary_sensor_->publish_state(state != ScanState::IDLE);
  }
}

// Component Lifecycle Methods
void BarcodeScanner::setup() {
  ESP_LOGCONFIG(TAG_SCANNER, "Setting up M5Stack Barcode Scanner");

  // Drain any bytes that accumulated in the UART hardware FIFO before our firmware
  // started.  After an OTA software reset the scanner stays powered and may be
  // transmitting; stale bytes left in the buffer corrupt ACK detection and version
  // parsing for the first commands we send.
  while (this->available()) {
    this->read();
  }

  // Initialise preference storage keyed to this component instance
  this->pref_ = global_preferences->make_preference<ScannerPreferences>(fnv1_hash("m5stack_barcode"));

  // Configure settings, skipping any that the scanner already has from a previous boot
  this->configure_defaults_();

  // Continuous and auto-sense modes scan on their own from power-up; reflect that in the
  // scan state so is_idle / is_continuous_mode and the scanning binary sensor are correct.
  if (this->is_continuous_mode())
    this->scan_state_ = ScanState::CONTINUOUS_SCANNING;
  if (this->scanning_binary_sensor_ != nullptr)
    this->scanning_binary_sensor_->publish_state(this->scan_state_ != ScanState::IDLE);

  // Request firmware version if a version sensor is wired
  if (this->version_sensor_ != nullptr) {
    this->queue_command(CommandFactory::create_version_command());
  }
}

// Returns true only if every stored enum field falls within its declared range.
// An out-of-range value means the struct was written by a different firmware version
// or the NVS slot is corrupt — treat the whole entry as invalid rather than casting
// a garbage byte into a C++ enum.
static bool prefs_in_range(const ScannerPreferences &p) {
  return p.operation_mode <= static_cast<uint8_t>(OperationMode::AUTO_SENSE) &&
         p.terminator <= static_cast<uint8_t>(Terminator::CRLFCRLF) &&
         p.light_mode <= static_cast<uint8_t>(LightMode::LIGHT_ALWAYS_OFF) &&
         p.locate_light_mode <= static_cast<uint8_t>(LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF) &&
         p.sound_mode <= static_cast<uint8_t>(SoundMode::SOUND_ENABLED) &&
         p.buzzer_volume <= static_cast<uint8_t>(BuzzerVolume::BUZZER_VOLUME_LOW) &&
         p.decoding_success_light_mode <= static_cast<uint8_t>(DecodingSuccessLightMode::DECODING_LIGHT_ENABLED) &&
         p.boot_sound_mode <= static_cast<uint8_t>(BootSoundMode::BOOT_SOUND_ENABLED) &&
         p.decode_sound_mode <= static_cast<uint8_t>(DecodeSoundMode::DECODE_SOUND_ENABLED) &&
         p.scan_duration <= static_cast<uint8_t>(ScanDuration::UNLIMITED) &&
         p.stable_induction_time <= static_cast<uint8_t>(StableInductionTime::MS_1000) &&
         p.reading_interval <= static_cast<uint8_t>(ReadingInterval::MS_2000) &&
         p.same_code_interval <= static_cast<uint8_t>(SameCodeInterval::MS_2000) &&
         p.cmd_ack_sound_mode <= static_cast<uint8_t>(CmdAckSoundMode::CMD_ACK_SOUND_ENABLED) &&
         p.config_code_scan_mode <= static_cast<uint8_t>(ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED);
}

void BarcodeScanner::configure_defaults_() {
  // Load previously-saved settings from NVS flash.  A valid entry means the
  // scanner was already programmed with those values and does not need them
  // resent unless they have changed in the YAML configuration.
  ScannerPreferences stored{};
  const bool has_valid_prefs =
      this->pref_.load(&stored) && stored.version == SETTINGS_VERSION && prefs_in_range(stored);

  ESP_LOGD(TAG_SCANNER, "Configuring scanner defaults (cached prefs valid=%s)", has_valid_prefs ? "yes" : "no");

// Helper macro: queue a command only when the stored value differs from the
// desired value (or when there are no valid stored preferences at all).
#define QUEUE_IF_CHANGED(field, create_fn, value) \
  if (!has_valid_prefs || stored.field != static_cast<uint8_t>(value)) { \
    this->queue_command(create_fn(value)); \
  }

  QUEUE_IF_CHANGED(operation_mode, CommandFactory::create_mode_command, this->operation_mode_)
  QUEUE_IF_CHANGED(terminator, CommandFactory::create_terminator_command, this->terminator_)
  QUEUE_IF_CHANGED(light_mode, CommandFactory::create_light_command, this->light_mode_)
  QUEUE_IF_CHANGED(locate_light_mode, CommandFactory::create_locate_light_command, this->locate_light_mode_)
  QUEUE_IF_CHANGED(sound_mode, CommandFactory::create_sound_command, this->sound_mode_)
  QUEUE_IF_CHANGED(buzzer_volume, CommandFactory::create_volume_command, this->buzzer_volume_)
  QUEUE_IF_CHANGED(decoding_success_light_mode, CommandFactory::create_decoding_success_light_command,
                   this->decoding_success_light_mode_)
  QUEUE_IF_CHANGED(boot_sound_mode, CommandFactory::create_boot_sound_command, this->boot_sound_mode_)
  QUEUE_IF_CHANGED(decode_sound_mode, CommandFactory::create_decode_sound_command, this->decode_sound_mode_)
  QUEUE_IF_CHANGED(scan_duration, CommandFactory::create_scan_duration_command, this->scan_duration_)
  QUEUE_IF_CHANGED(stable_induction_time, CommandFactory::create_stable_induction_time_command,
                   this->stable_induction_time_)
  QUEUE_IF_CHANGED(reading_interval, CommandFactory::create_reading_interval_command, this->reading_interval_)
  QUEUE_IF_CHANGED(same_code_interval, CommandFactory::create_same_code_interval_command, this->same_code_interval_)
  QUEUE_IF_CHANGED(cmd_ack_sound_mode, CommandFactory::create_cmd_ack_sound_command, this->cmd_ack_sound_mode_)
  QUEUE_IF_CHANGED(config_code_scan_mode, CommandFactory::create_config_code_scan_command, this->config_code_scan_mode_)

#undef QUEUE_IF_CHANGED
}

static void publish_select(select::Select *sel, uint8_t index) {
  if (sel != nullptr)
    sel->publish_state(static_cast<size_t>(index));
}

static void publish_switch(switch_::Switch *sw, uint8_t enabled) {
  if (sw != nullptr)
    sw->publish_state(enabled != 0);
}

void BarcodeScanner::publish_initial_states_() {
  publish_select(this->operation_mode_select_, static_cast<uint8_t>(this->operation_mode_));
  publish_select(this->terminator_select_, static_cast<uint8_t>(this->terminator_));
  publish_select(this->light_mode_select_, static_cast<uint8_t>(this->light_mode_));
  publish_select(this->locate_light_mode_select_, static_cast<uint8_t>(this->locate_light_mode_));
  publish_select(this->buzzer_volume_select_, static_cast<uint8_t>(this->buzzer_volume_));
  publish_select(this->scan_duration_select_, static_cast<uint8_t>(this->scan_duration_));
  publish_select(this->stable_induction_time_select_, static_cast<uint8_t>(this->stable_induction_time_));
  publish_select(this->reading_interval_select_, static_cast<uint8_t>(this->reading_interval_));
  publish_select(this->same_code_interval_select_, static_cast<uint8_t>(this->same_code_interval_));
  publish_switch(this->sound_switch_, static_cast<uint8_t>(this->sound_mode_));
  publish_switch(this->boot_sound_switch_, static_cast<uint8_t>(this->boot_sound_mode_));
  publish_switch(this->decode_sound_switch_, static_cast<uint8_t>(this->decode_sound_mode_));
  publish_switch(this->decoding_success_light_switch_, static_cast<uint8_t>(this->decoding_success_light_mode_));
  publish_switch(this->cmd_ack_sound_switch_, static_cast<uint8_t>(this->cmd_ack_sound_mode_));
  publish_switch(this->config_code_scan_switch_, static_cast<uint8_t>(this->config_code_scan_mode_));
}

void BarcodeScanner::save_settings_() {
  ScannerPreferences prefs{};
  prefs.version = SETTINGS_VERSION;
  prefs.operation_mode = static_cast<uint8_t>(this->operation_mode_);
  prefs.terminator = static_cast<uint8_t>(this->terminator_);
  prefs.light_mode = static_cast<uint8_t>(this->light_mode_);
  prefs.locate_light_mode = static_cast<uint8_t>(this->locate_light_mode_);
  prefs.sound_mode = static_cast<uint8_t>(this->sound_mode_);
  prefs.buzzer_volume = static_cast<uint8_t>(this->buzzer_volume_);
  prefs.decoding_success_light_mode = static_cast<uint8_t>(this->decoding_success_light_mode_);
  prefs.boot_sound_mode = static_cast<uint8_t>(this->boot_sound_mode_);
  prefs.decode_sound_mode = static_cast<uint8_t>(this->decode_sound_mode_);
  prefs.scan_duration = static_cast<uint8_t>(this->scan_duration_);
  prefs.stable_induction_time = static_cast<uint8_t>(this->stable_induction_time_);
  prefs.reading_interval = static_cast<uint8_t>(this->reading_interval_);
  prefs.same_code_interval = static_cast<uint8_t>(this->same_code_interval_);
  prefs.cmd_ack_sound_mode = static_cast<uint8_t>(this->cmd_ack_sound_mode_);
  prefs.config_code_scan_mode = static_cast<uint8_t>(this->config_code_scan_mode_);
  if (!this->pref_.save(&prefs)) {
    ESP_LOGW(TAG_SCANNER, "Failed to save scanner preferences to NVS");
  }
}

void BarcodeScanner::loop() {
  // Publish initial sub-component states on the first loop() tick, after HA has had time
  // to complete the entity list exchange.  Doing this in setup() races with the API
  // connection handshake and causes HA to repeatedly reconnect during boot.
  if (!this->initial_states_published_) {
    this->publish_initial_states_();
    this->initial_states_published_ = true;
  }

  this->read_buffer_();

  if (this->waiting_for_ack_) {
    this->handle_ack_or_timeout_();
  } else if (this->expected_response_ == ResponseType::VERSION) {
    this->handle_version_response_();
  } else {
    // Barcode output has the same framing in every operation mode: the decoded data followed
    // by the configured terminator (or nothing), sent without any protocol header.
    if (this->discard_frame_) {
      // Drop everything up to the end of the oversized frame, then resume normal framing.
      if (millis() - this->last_rx_time_ >= RX_IDLE_WINDOW_MS) {
        ESP_LOGW(TAG_SCANNER, "Dropped oversized frame");
        this->discard_frame_ = false;
        this->clear_buffer_();
      }
    } else if (this->has_complete_frame_()) {
      this->process_barcode_();
    }

    // HOST-mode scan timeout: fire on_scan_timeout if scan_duration has elapsed without a result.
    // scan_duration_to_ms() returns 0 for UNLIMITED, in which case we never time out.
    if (this->scan_state_ == ScanState::MANUAL_SCANNING && this->scan_started_at_ != 0) {
      const uint32_t duration_ms = this->get_scan_duration_ms();
      if (duration_ms > 0 && (millis() - this->scan_started_at_) > duration_ms) {
        ESP_LOGD(TAG_SCANNER, "Scan timed out after %" PRIu32 " ms", duration_ms);
        this->scan_started_at_ = 0;
        this->set_scan_state(ScanState::IDLE);
        this->scan_timeout_callback_();
      }
    }
  }

  this->process_command_queue_();
}

void BarcodeScanner::handle_ack_or_timeout_() {
  // Configuration commands always elicit the 6-byte HOST ACK regardless of the current
  // operation mode.  Search the whole buffer: barcode bytes from a continuous-mode scan may
  // precede the ACK, and a HOST-mode start ACK may be followed by the barcode in the same
  // read.  Only the ACK bytes are consumed; surrounding data stays for barcode framing.
  const auto *ack_begin = Commands::Responses::ACK;
  const auto *ack_end = ack_begin + Commands::Responses::ACK_SIZE;
  auto ack = std::search(this->rx_buffer_.begin(), this->rx_buffer_.end(), ack_begin, ack_end);
  if (ack != this->rx_buffer_.end()) {
    this->rx_buffer_.erase(ack, ack + Commands::Responses::ACK_SIZE);
    ESP_LOGD(TAG_SCANNER, "Command acknowledged");
    // Take ownership before invoking the callback: it may queue further commands.
    std::unique_ptr<Command> command = std::move(this->command_queue_.front());
    this->finish_command_();
    command->on_success(this);
    return;
  }

  if (millis() - this->last_command_time_ <= COMMAND_TIMEOUT_MS)
    return;

  const Command *command = this->command_queue_.front().get();
  if (this->command_attempts_ < MAX_COMMAND_ATTEMPTS) {
    // Reset to IDLE so process_command_queue_() retries with a fresh wake-up + send cycle.
    ESP_LOGD(TAG_SCANNER, "Command '%s %s' timed out (attempt %u/%u), retrying", command->get_name(),
             command->get_value(), this->command_attempts_, MAX_COMMAND_ATTEMPTS);
    this->waiting_for_ack_ = false;
    this->command_state_ = CommandState::IDLE;
    return;
  }

  // All attempts exhausted: log at WARN so the failure is visible in normal log output,
  // invoke the failure callback, then drop the command.
  ESP_LOGW(TAG_SCANNER, "Command '%s %s' failed after %u attempts — scanner not responding", command->get_name(),
           command->get_value(), this->command_attempts_);
  std::unique_ptr<Command> failed = std::move(this->command_queue_.front());
  this->finish_command_();
  failed->on_failure(this);
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
    ESP_LOGW(TAG_SCANNER, "Version request timed out with no response");
  } else if (now - this->last_command_time_ < VERSION_SETTLE_MS || now - this->last_rx_time_ < RX_IDLE_WINDOW_MS) {
    return;
  } else {
    this->process_version_();
  }
  this->finish_command_();
}

void BarcodeScanner::finish_command_() {
  this->command_queue_.erase(this->command_queue_.begin());
  this->waiting_for_ack_ = false;
  this->command_state_ = CommandState::IDLE;
  this->expected_response_ = ResponseType::NONE;
  this->command_attempts_ = 0;
}

void BarcodeScanner::dump_config() {
  ESP_LOGCONFIG(
      TAG_SCANNER,
      "M5Stack Barcode Scanner:\n"
      "  Operation Mode: %s\n"
      "  Terminator: %s\n"
      "  Light Mode: %s\n"
      "  Locate Light Mode: %s\n"
      "  Decoding Success Light: %s\n"
      "  Sound: %s\n"
      "  Buzzer Volume: %s\n"
      "  Boot Sound: %s\n"
      "  Decode Sound: %s\n"
      "  Command ACK Sound: %s\n"
      "  Config Code Scanning: %s\n"
      "  Scan Duration: %s\n"
      "  Stable Induction Time: %s\n"
      "  Reading Interval: %s\n"
      "  Same Code Interval: %s",
      operation_mode_to_string(this->operation_mode_), terminator_to_string(this->terminator_),
      light_mode_to_string(this->light_mode_), light_mode_to_string(static_cast<LightMode>(this->locate_light_mode_)),
      enabled_to_string(this->decoding_success_light_mode_ == DecodingSuccessLightMode::DECODING_LIGHT_ENABLED),
      enabled_to_string(this->sound_mode_ == SoundMode::SOUND_ENABLED), buzzer_volume_to_string(this->buzzer_volume_),
      enabled_to_string(this->boot_sound_mode_ == BootSoundMode::BOOT_SOUND_ENABLED),
      enabled_to_string(this->decode_sound_mode_ == DecodeSoundMode::DECODE_SOUND_ENABLED),
      enabled_to_string(this->cmd_ack_sound_mode_ == CmdAckSoundMode::CMD_ACK_SOUND_ENABLED),
      enabled_to_string(this->config_code_scan_mode_ == ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED),
      scan_duration_to_string(this->scan_duration_),
      interval_to_string(static_cast<uint8_t>(this->stable_induction_time_)),
      interval_to_string(static_cast<uint8_t>(this->reading_interval_)),
      interval_to_string(static_cast<uint8_t>(this->same_code_interval_)));
  if (this->version_sensor_ != nullptr && this->version_sensor_->has_state()) {
    ESP_LOGCONFIG(TAG_SCANNER, "  Scanner Firmware Version: %s", this->version_sensor_->get_state().c_str());
  }
  LOG_TEXT_SENSOR("  ", "Barcode", this->barcode_sensor_);
  LOG_TEXT_SENSOR("  ", "Version", this->version_sensor_);
  LOG_BINARY_SENSOR("  ", "Scanning", this->scanning_binary_sensor_);
}

// Buffer Management Methods
void BarcodeScanner::read_buffer_() {
  bool got_bytes = false;
  while (this->available()) {
    uint8_t byte;
    if (!this->read_byte(&byte))
      break;
    this->rx_buffer_.push_back(byte);
    got_bytes = true;
  }
  if (got_bytes) {
    this->last_rx_time_ = millis();
  }
  if (this->rx_buffer_.size() > MAX_RX_BUFFER_SIZE) {
    ESP_LOGW(TAG_SCANNER, "RX buffer overflow (%zu bytes); discarding frame", this->rx_buffer_.size());
    this->clear_buffer_();
    // Drop the remainder of this frame too, unless we are waiting for a protocol response
    // (a garbage burst there must not cause the next genuine barcode to be dropped).
    this->discard_frame_ = !this->waiting_for_ack_ && this->expected_response_ != ResponseType::VERSION;
  }
}

// Command Processing Methods
void BarcodeScanner::process_command_queue_() {
  // Only process commands if we're not waiting for an acknowledgment or a version response
  if (this->waiting_for_ack_ || this->command_queue_.empty()) {
    return;
  }
  if (this->expected_response_ == ResponseType::VERSION) {
    return;  // Block queue while waiting for version data to arrive
  }
  // Never interleave commands with a scanner that is mid-output: the ACK would be mixed
  // into barcode data.  The frame is processed by loop() within RX_IDLE_WINDOW_MS.
  if (this->command_state_ == CommandState::IDLE && !this->rx_buffer_.empty()) {
    return;
  }

  // Get the current command
  auto &command = this->command_queue_.front();

  // Check current state to determine actions
  switch (this->command_state_) {
    case CommandState::IDLE: {
      // Wake up the scanner
      this->wake_up_();
      break;
    }

    case CommandState::WAKEUP_SENT: {
      // Check if we've waited at least WAKEUP_DELAY_MS since wake-up was sent
      if (millis() - this->last_command_time_ >= WAKEUP_DELAY_MS) {
        this->write_command_(command);
      }
      break;
    }

    case CommandState::COMMAND_SENT:
      // Unreachable: COMMAND_SENT always has waiting_for_ack_ or an expected VERSION response.
      break;
  }
}

void BarcodeScanner::write_command_(const std::unique_ptr<Command> &command) {
  this->command_attempts_++;
  command->log_command_data(TAG_SCANNER, "Sending");

  // Set the expected response type based on the command
  this->expected_response_ = command->get_expected_response();

  this->write_array(command->get_data(), command->get_length());

  // VERSION commands are special: the scanner responds with raw version data (not an ACK
  // byte sequence), which handle_version_response_() collects instead of the ACK path.
  this->waiting_for_ack_ = (this->expected_response_ != ResponseType::VERSION);
  this->last_command_time_ = millis();
  this->command_state_ = CommandState::COMMAND_SENT;
}

void BarcodeScanner::wake_up_() {
  // First, send wake-up command to wake up the scanner from sleep state
  ESP_LOGD(TAG_SCANNER, "Sending wake-up command");

  // Send the wakeup command
  this->write_array(Commands::WAKEUP, Commands::WAKEUP_SIZE);

  // Record when we sent the wake-up command
  this->last_command_time_ = millis();

  // Move to WAKEUP_SENT state
  this->command_state_ = CommandState::WAKEUP_SENT;
}

void BarcodeScanner::queue_command(std::unique_ptr<Command> command) {
  if (command == nullptr) {
    ESP_LOGW(TAG_SCANNER, "Attempted to queue null command");
    return;
  }

  // Check if queue is full
  if (this->command_queue_.size() >= MAX_QUEUE_SIZE) {
    ESP_LOGW(TAG_SCANNER, "Command queue full (size=%zu), dropping command: %s", this->command_queue_.size(),
             command->get_name());
    return;
  }

  command->log_command_data(TAG_SCANNER, "Queuing");
  this->command_queue_.push_back(std::move(command));
}

size_t BarcodeScanner::terminator_length_in_buffer_() const {
  const char *term = terminator_to_bytes(this->terminator_);
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

// Response Processing Methods

// Returns true if s is valid UTF-8.  aioesphomeapi deserialises TextSensorStateResponse
// with google.protobuf, which rejects strings containing invalid UTF-8 sequences and
// closes the API connection — causing HA to reconnect every ~160 ms.
static bool is_valid_utf8(const std::string &s) {
  const auto *bytes = reinterpret_cast<const uint8_t *>(s.data());
  const size_t len = s.size();
  for (size_t i = 0; i < len;) {
    uint8_t b = bytes[i];
    size_t seqlen;
    if (b <= 0x7F) {
      seqlen = 1;
    } else if (b >= 0xC2 && b <= 0xDF) {
      seqlen = 2;
    } else if (b >= 0xE0 && b <= 0xEF) {
      seqlen = 3;
    } else if (b >= 0xF0 && b <= 0xF4) {
      seqlen = 4;
    } else {
      return false;  // invalid lead byte
    }
    if (i + seqlen > len)
      return false;  // truncated sequence
    for (size_t j = 1; j < seqlen; j++) {
      if ((bytes[i + j] & 0xC0) != 0x80)
        return false;  // invalid continuation byte
    }
    i += seqlen;
  }
  return true;
}

void BarcodeScanner::process_barcode_() {
  if (this->rx_buffer_.empty()) {
    return;
  }
  size_t data_length = this->rx_buffer_.size() - this->terminator_length_in_buffer_();
  if (data_length > MAX_BARCODE_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Barcode too long (%zu bytes), truncating to %zu bytes", data_length, MAX_BARCODE_LENGTH);
    data_length = MAX_BARCODE_LENGTH;
  }
  std::string barcode(reinterpret_cast<const char *>(this->rx_buffer_.data()), data_length);
  this->clear_buffer_();
  if (barcode.empty())
    return;

  // A HOST-mode scan is complete once its barcode arrives — cancel the pending timeout.
  this->scan_started_at_ = 0;
  if (this->scan_state_ == ScanState::MANUAL_SCANNING)
    this->set_scan_state(ScanState::IDLE);

  // Guard against publishing invalid UTF-8: aioesphomeapi uses google.protobuf which
  // rejects malformed strings and closes the API connection (HA reconnect loop).
  if (!is_valid_utf8(barcode)) {
    ESP_LOGW(TAG_SCANNER, "Barcode contains invalid UTF-8 (%zu bytes) — discarding to protect HA API connection",
             barcode.size());
    return;
  }
  ESP_LOGD(TAG_SCANNER, "Barcode received: %s", barcode.c_str());

  if (this->barcode_sensor_ != nullptr)
    this->barcode_sensor_->publish_state(barcode);

  // Fire on_barcode automation trigger (runs homeassistant.event etc.)
  this->barcode_callback_(barcode);

  if (this->scan_event_ != nullptr)
    this->scan_event_->trigger("scan_successful");
}

void BarcodeScanner::process_version_() {
  if (this->rx_buffer_.empty()) {
    return;
  }

  if (this->rx_buffer_.size() > MAX_VERSION_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Version response too long (%zu bytes), truncating to %zu bytes", this->rx_buffer_.size(),
             MAX_VERSION_LENGTH);
    this->rx_buffer_.resize(MAX_VERSION_LENGTH);
  }

  // Raw hex dump for debugging the scanner's undocumented response format.
  {
    const size_t log_len = std::min(this->rx_buffer_.size(), static_cast<size_t>(32));
    char hex_buf[3 * 32 + 1];
    char *p = hex_buf;
    for (size_t i = 0; i < log_len; i++) {
      p += snprintf(p, 4, "%02X ", this->rx_buffer_[i]);
    }
    ESP_LOGD(TAG_SCANNER, "Version raw (%zu bytes)%s: %s", this->rx_buffer_.size(),
             this->rx_buffer_.size() > 32 ? " (truncated)" : "", hex_buf);
  }

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

  // Reject if not valid UTF-8 — same protection applied to barcodes.
  if (!version.empty() && !is_valid_utf8(version)) {
    ESP_LOGW(TAG_SCANNER, "Version string contains invalid UTF-8 — discarding to protect HA API connection");
    version.clear();
  }

  if (this->version_sensor_ != nullptr) {
    if (!version.empty()) {
      ESP_LOGD(TAG_SCANNER, "Publishing version: '%s'", version.c_str());
      this->version_sensor_->publish_state(version);
    } else {
      ESP_LOGW(TAG_SCANNER, "Version response yielded no publishable string (%zu raw bytes)", this->rx_buffer_.size());
    }
  }

  this->clear_buffer_();
}

// Scanner Control Methods
void BarcodeScanner::start_scan() {
  if (this->operation_mode_ != OperationMode::HOST) {
    ESP_LOGW(TAG_SCANNER, "Cannot start scan in non-HOST mode");
    return;
  }

  if (this->is_scanning()) {
    ESP_LOGD(TAG_SCANNER, "Scan already in progress");
    return;
  }

  ESP_LOGD(TAG_SCANNER, "Starting scan in HOST mode");

  this->queue_command(CommandFactory::create_start_command());
  // Mark the scan as active immediately so repeated start requests are rejected.  The
  // scan_duration timeout starts only once the scanner ACKs (see on_scan_started_()), which
  // is when the scanner itself starts its own scan_duration timer.
  this->set_scan_state(ScanState::MANUAL_SCANNING);
}

void BarcodeScanner::on_scan_started_() {
  // A barcode may already have been processed if it arrived together with the ACK.
  if (this->scan_state_ == ScanState::MANUAL_SCANNING)
    this->scan_started_at_ = millis();
}

void BarcodeScanner::stop_scan() {
  if (this->operation_mode_ != OperationMode::HOST) {
    ESP_LOGW(TAG_SCANNER, "Cannot stop scan in non-HOST mode");
    return;
  }

  if (!this->is_scanning()) {
    ESP_LOGD(TAG_SCANNER, "No scan in progress");
    return;
  }

  ESP_LOGD(TAG_SCANNER, "Stopping scan in HOST mode");

  this->queue_command(CommandFactory::create_stop_command());

  // Cancel any pending scan timeout
  this->scan_started_at_ = 0;

  // Update state
  this->set_scan_state(ScanState::IDLE);
}

// Setting changes: queue the command; the value is applied once the scanner ACKs it.

void BarcodeScanner::set_operation_mode(OperationMode value) {
  if (value == this->operation_mode_) {
    ESP_LOGD(TAG_SCANNER, "Operation mode already set to %s", operation_mode_to_string(value));
    return;
  }
  this->queue_command(CommandFactory::create_mode_command(value));
}

void BarcodeScanner::set_terminator(Terminator value) {
  if (value == this->terminator_) {
    ESP_LOGD(TAG_SCANNER, "Terminator already set to %s", terminator_to_string(value));
    return;
  }
  this->queue_command(CommandFactory::create_terminator_command(value));
}

void BarcodeScanner::set_light_mode(LightMode value) {
  if (value == this->light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Light mode already set to %s", light_mode_to_string(value));
    return;
  }
  this->queue_command(CommandFactory::create_light_command(value));
}

void BarcodeScanner::set_locate_light_mode(LocateLightMode value) {
  if (value == this->locate_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Locate light mode already set to %s", light_mode_to_string(static_cast<LightMode>(value)));
    return;
  }
  this->queue_command(CommandFactory::create_locate_light_command(value));
}

void BarcodeScanner::set_sound_mode(SoundMode value) {
  if (value == this->sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Sound mode already set to %s", enabled_to_string(value == SoundMode::SOUND_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_sound_command(value));
}

void BarcodeScanner::set_buzzer_volume(BuzzerVolume value) {
  if (value == this->buzzer_volume_) {
    ESP_LOGD(TAG_SCANNER, "Buzzer volume already set to %s", buzzer_volume_to_string(value));
    return;
  }
  this->queue_command(CommandFactory::create_volume_command(value));
}

void BarcodeScanner::set_decoding_success_light_mode(DecodingSuccessLightMode value) {
  if (value == this->decoding_success_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decoding success light mode already set to %s",
             enabled_to_string(value == DecodingSuccessLightMode::DECODING_LIGHT_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_decoding_success_light_command(value));
}

void BarcodeScanner::set_boot_sound_mode(BootSoundMode value) {
  if (value == this->boot_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Boot sound mode already set to %s",
             enabled_to_string(value == BootSoundMode::BOOT_SOUND_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_boot_sound_command(value));
}

void BarcodeScanner::set_decode_sound_mode(DecodeSoundMode value) {
  if (value == this->decode_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decode sound mode already set to %s",
             enabled_to_string(value == DecodeSoundMode::DECODE_SOUND_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_decode_sound_command(value));
}

void BarcodeScanner::set_scan_duration(ScanDuration value) {
  if (value == this->scan_duration_) {
    ESP_LOGD(TAG_SCANNER, "Scan duration already set to %s", scan_duration_to_string(value));
    return;
  }
  this->queue_command(CommandFactory::create_scan_duration_command(value));
}

void BarcodeScanner::set_stable_induction_time(StableInductionTime value) {
  if (value == this->stable_induction_time_) {
    ESP_LOGD(TAG_SCANNER, "Stable induction time already set to %s", interval_to_string(static_cast<uint8_t>(value)));
    return;
  }
  this->queue_command(CommandFactory::create_stable_induction_time_command(value));
}

void BarcodeScanner::set_reading_interval(ReadingInterval value) {
  if (value == this->reading_interval_) {
    ESP_LOGD(TAG_SCANNER, "Reading interval already set to %s", interval_to_string(static_cast<uint8_t>(value)));
    return;
  }
  this->queue_command(CommandFactory::create_reading_interval_command(value));
}

void BarcodeScanner::set_same_code_interval(SameCodeInterval value) {
  if (value == this->same_code_interval_) {
    ESP_LOGD(TAG_SCANNER, "Same code interval already set to %s", interval_to_string(static_cast<uint8_t>(value)));
    return;
  }
  this->queue_command(CommandFactory::create_same_code_interval_command(value));
}

void BarcodeScanner::set_cmd_ack_sound_mode(CmdAckSoundMode value) {
  if (value == this->cmd_ack_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Command ACK sound mode already set to %s",
             enabled_to_string(value == CmdAckSoundMode::CMD_ACK_SOUND_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_cmd_ack_sound_command(value));
}

void BarcodeScanner::set_config_code_scan_mode(ConfigCodeScanMode value) {
  if (value == this->config_code_scan_mode_) {
    ESP_LOGD(TAG_SCANNER, "Config code scan mode already set to %s",
             enabled_to_string(value == ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED));
    return;
  }
  this->queue_command(CommandFactory::create_config_code_scan_command(value));
}

void BarcodeScanner::process_current_buffer() {
  // Never treat a pending ACK or version response as barcode data.
  if (this->waiting_for_ack_ || this->expected_response_ == ResponseType::VERSION || this->discard_frame_)
    return;
  this->read_buffer_();
  this->process_barcode_();
}

// ACKed setting changes: update the in-memory state, persist all settings to NVS so the next
// boot can skip re-sending them, and publish the confirmed value to the HA entity.

void BarcodeScanner::set_operation_mode_state(OperationMode value) {
  ESP_LOGD(TAG_SCANNER, "Operation mode set to %s", operation_mode_to_string(value));
  this->operation_mode_ = value;
  this->save_settings_();
  // Continuous/auto-sense scan on their own; leaving them (or HOST mode mid-scan) ends any scan.
  this->scan_started_at_ = 0;
  this->set_scan_state(this->is_continuous_mode() ? ScanState::CONTINUOUS_SCANNING : ScanState::IDLE);
  publish_select(this->operation_mode_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_terminator_state(Terminator value) {
  ESP_LOGD(TAG_SCANNER, "Terminator set to %s", terminator_to_string(value));
  this->terminator_ = value;
  this->save_settings_();
  publish_select(this->terminator_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_light_mode_state(LightMode value) {
  ESP_LOGD(TAG_SCANNER, "Light mode set to %s", light_mode_to_string(value));
  this->light_mode_ = value;
  this->save_settings_();
  publish_select(this->light_mode_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_locate_light_mode_state(LocateLightMode value) {
  ESP_LOGD(TAG_SCANNER, "Locate light mode set to %s", light_mode_to_string(static_cast<LightMode>(value)));
  this->locate_light_mode_ = value;
  this->save_settings_();
  publish_select(this->locate_light_mode_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_sound_mode_state(SoundMode value) {
  ESP_LOGD(TAG_SCANNER, "Sound mode set to %s", enabled_to_string(value == SoundMode::SOUND_ENABLED));
  this->sound_mode_ = value;
  this->save_settings_();
  publish_switch(this->sound_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_buzzer_volume_state(BuzzerVolume value) {
  ESP_LOGD(TAG_SCANNER, "Buzzer volume set to %s", buzzer_volume_to_string(value));
  this->buzzer_volume_ = value;
  this->save_settings_();
  publish_select(this->buzzer_volume_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_decoding_success_light_mode_state(DecodingSuccessLightMode value) {
  ESP_LOGD(TAG_SCANNER, "Decoding success light mode set to %s",
           enabled_to_string(value == DecodingSuccessLightMode::DECODING_LIGHT_ENABLED));
  this->decoding_success_light_mode_ = value;
  this->save_settings_();
  publish_switch(this->decoding_success_light_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_boot_sound_mode_state(BootSoundMode value) {
  ESP_LOGD(TAG_SCANNER, "Boot sound mode set to %s", enabled_to_string(value == BootSoundMode::BOOT_SOUND_ENABLED));
  this->boot_sound_mode_ = value;
  this->save_settings_();
  publish_switch(this->boot_sound_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_decode_sound_mode_state(DecodeSoundMode value) {
  ESP_LOGD(TAG_SCANNER, "Decode sound mode set to %s",
           enabled_to_string(value == DecodeSoundMode::DECODE_SOUND_ENABLED));
  this->decode_sound_mode_ = value;
  this->save_settings_();
  publish_switch(this->decode_sound_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_scan_duration_state(ScanDuration value) {
  ESP_LOGD(TAG_SCANNER, "Scan duration set to %s", scan_duration_to_string(value));
  this->scan_duration_ = value;
  this->save_settings_();
  publish_select(this->scan_duration_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_stable_induction_time_state(StableInductionTime value) {
  ESP_LOGD(TAG_SCANNER, "Stable induction time set to %s", interval_to_string(static_cast<uint8_t>(value)));
  this->stable_induction_time_ = value;
  this->save_settings_();
  publish_select(this->stable_induction_time_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_reading_interval_state(ReadingInterval value) {
  ESP_LOGD(TAG_SCANNER, "Reading interval set to %s", interval_to_string(static_cast<uint8_t>(value)));
  this->reading_interval_ = value;
  this->save_settings_();
  publish_select(this->reading_interval_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_same_code_interval_state(SameCodeInterval value) {
  ESP_LOGD(TAG_SCANNER, "Same code interval set to %s", interval_to_string(static_cast<uint8_t>(value)));
  this->same_code_interval_ = value;
  this->save_settings_();
  publish_select(this->same_code_interval_select_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_cmd_ack_sound_mode_state(CmdAckSoundMode value) {
  ESP_LOGD(TAG_SCANNER, "Command ACK sound mode set to %s",
           enabled_to_string(value == CmdAckSoundMode::CMD_ACK_SOUND_ENABLED));
  this->cmd_ack_sound_mode_ = value;
  this->save_settings_();
  publish_switch(this->cmd_ack_sound_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::set_config_code_scan_mode_state(ConfigCodeScanMode value) {
  ESP_LOGD(TAG_SCANNER, "Config code scan mode set to %s",
           enabled_to_string(value == ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED));
  this->config_code_scan_mode_ = value;
  this->save_settings_();
  publish_switch(this->config_code_scan_switch_, static_cast<uint8_t>(value));
}

void BarcodeScanner::factory_reset() {
  ESP_LOGW(TAG_SCANNER, "Factory reset requested — scanner will revert to hardware defaults and ESP will reboot");
  this->queue_command(CommandFactory::create_factory_reset_command());
}

void BarcodeScanner::do_factory_reset_() {
  // Invalidate NVS by saving a zeroed struct (version=0).  On next boot
  // configure_defaults_() will see a version mismatch and re-send every setting.
  ScannerPreferences empty{};
  if (!this->pref_.save(&empty)) {
    ESP_LOGW(TAG_SCANNER, "Failed to invalidate NVS preferences; settings may not fully re-sync after reboot");
  }
  ESP_LOGW(TAG_SCANNER, "Scanner factory reset acknowledged — rebooting to re-apply YAML settings");
  App.safe_reboot();
}

}  // namespace m5stack_barcode
}  // namespace esphome
