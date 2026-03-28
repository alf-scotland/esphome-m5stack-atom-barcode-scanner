#include "m5stack_barcode.h"

#include "command_handlers.h"
#include "commands.h"
#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

// Logging tag for this component
const char *const TAG_SCANNER = "m5stack_barcode";

// Time constants (in milliseconds)
static const uint32_t WAKEUP_DELAY_MS = 50;       // Delay between wake-up and command send
static const uint32_t COMMAND_TIMEOUT_MS = 2000;  // Timeout for command acknowledgment

// Command queue size
static const size_t MAX_QUEUE_SIZE = 20;  // Maximum number of commands queued at once

// Data size limits
static const size_t MAX_BARCODE_LENGTH = 128;  // Maximum barcode data length
static const size_t MAX_VERSION_LENGTH = 32;   // Maximum version string length

// Methods moved from header file
bool BarcodeScanner::is_continuous_mode() const {
  return this->operation_mode_ == OperationMode::CONTINUOUS || this->operation_mode_ == OperationMode::AUTO_SENSE;
}

bool BarcodeScanner::is_scanning() const { return this->scan_state_ != ScanState::IDLE; }

ScanState BarcodeScanner::get_scan_state() const { return this->scan_state_; }

void BarcodeScanner::set_scan_state(ScanState state) {
  if (this->scan_state_ != state) {
    ESP_LOGD(TAG_SCANNER, "Scan state changed from %d to %d", (int) this->scan_state_, (int) state);
    this->scan_state_ = state;
    if (this->scanning_binary_sensor_ != nullptr)
      this->scanning_binary_sensor_->publish_state(state != ScanState::IDLE);
  }
}

uint32_t BarcodeScanner::get_scan_duration_ms() const {
  return esphome::m5stack_barcode::scan_duration_to_ms(this->scan_duration_);
}

// Component Lifecycle Methods
void BarcodeScanner::setup() {
  ESP_LOGCONFIG(TAG_SCANNER, "Setting up M5Stack Barcode Scanner");

  // Initialise preference storage keyed to this component instance
  this->pref_ = global_preferences->make_preference<ScannerPreferences>(fnv1_hash("m5stack_barcode"));

  // Configure settings, skipping any that the scanner already has from a previous boot
  this->configure_defaults_();

  // Publish initial states to all optional sub-components so HA shows the correct values
  // immediately after boot, before any UART commands are sent.
  if (this->operation_mode_select_ != nullptr)
    this->operation_mode_select_->publish_state(OperationModeSelect::mode_to_key(this->operation_mode_));
  if (this->buzzer_volume_select_ != nullptr)
    this->buzzer_volume_select_->publish_state(BuzzerVolumeSelect::volume_to_key(this->buzzer_volume_));
  if (this->light_mode_select_ != nullptr)
    this->light_mode_select_->publish_state(LightModeSelect::mode_to_key(this->light_mode_));
  if (this->locate_light_mode_select_ != nullptr)
    this->locate_light_mode_select_->publish_state(LocateLightModeSelect::mode_to_key(this->locate_light_mode_));
  if (this->scan_duration_select_ != nullptr)
    this->scan_duration_select_->publish_state(ScanDurationSelect::duration_to_key(this->scan_duration_));
  if (this->sound_switch_ != nullptr)
    this->sound_switch_->publish_state(this->sound_mode_ == SoundMode::SOUND_ENABLED);
  if (this->boot_sound_switch_ != nullptr)
    this->boot_sound_switch_->publish_state(this->boot_sound_mode_ == BootSoundMode::BOOT_SOUND_ENABLED);
  if (this->decode_sound_switch_ != nullptr)
    this->decode_sound_switch_->publish_state(this->decode_sound_mode_ == DecodeSoundMode::DECODE_SOUND_ENABLED);
  if (this->decoding_success_light_switch_ != nullptr)
    this->decoding_success_light_switch_->publish_state(this->decoding_success_light_mode_ ==
                                                        DecodingSuccessLightMode::DECODING_LIGHT_ENABLED);
  if (this->scanning_binary_sensor_ != nullptr)
    this->scanning_binary_sensor_->publish_state(this->scan_state_ != ScanState::IDLE);

  // Request firmware version if a version sensor is wired
  if (this->version_sensor_ != nullptr) {
    this->request_version_();
  }
}

void BarcodeScanner::configure_defaults_() {
  // Load previously-saved settings from NVS flash.  A valid entry means the
  // scanner was already programmed with those values and does not need them
  // resent unless they have changed in the YAML configuration.
  ScannerPreferences stored{};
  const bool has_valid_prefs = this->pref_.load(&stored) && stored.version == SETTINGS_VERSION;

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

#undef QUEUE_IF_CHANGED
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
  this->pref_.save(&prefs);
}

void BarcodeScanner::loop() {
  // Process any pending commands first
  this->process_command_queue_();

  // Read available data
  this->read_buffer_();

  // Handle command acknowledgments
  if (this->waiting_for_ack_) {
    // Configuration commands always elicit the 6-byte HOST ACK regardless of the
    // current operation mode (is_ack_sequence_ uses the mode to select the format,
    // which is wrong here — mode may not have changed yet when checking the ACK for
    // a mode-change command).
    const bool got_ack = this->rx_buffer_.size() >= Commands::Responses::ACK_SIZE &&
                         memcmp(this->rx_buffer_.data(), Commands::Responses::ACK, Commands::Responses::ACK_SIZE) == 0;
    if (got_ack) {
      ESP_LOGD(TAG_SCANNER, "Command acknowledged");

      // Get the current command
      auto &command = this->command_queue_.front();

      // Mark as successful and invoke callback
      command->on_success(this);

      // Remove from queue
      this->command_queue_.erase(this->command_queue_.begin());

      // Reset state
      this->waiting_for_ack_ = false;
      this->command_state_ = CommandState::IDLE;

      // Clear the buffer except for version responses
      if (this->expected_response_ != ResponseType::VERSION) {
        this->clear_buffer_();
      }
    }
    // Check for command timeout (COMMAND_TIMEOUT_MS)
    else if (millis() - this->last_command_time_ > COMMAND_TIMEOUT_MS) {
      // Get the current command
      auto &command = this->command_queue_.front();

      // Mark as failed and invoke callback
      ESP_LOGD(TAG_SCANNER, "Command '%s' timed out", command->get_description());
      command->on_failure(this);
      this->command_queue_.erase(this->command_queue_.begin());

      // Reset state
      this->waiting_for_ack_ = false;
      this->command_state_ = CommandState::IDLE;
      this->expected_response_ = ResponseType::NONE;

      this->clear_buffer_();
    }

    // Skip other processing while waiting for acknowledgment
    return;
  }

  // Process version information if requested
  if (this->expected_response_ == ResponseType::VERSION) {
    // Wait COMMAND_TIMEOUT_MS for all version data to arrive, then process
    // whatever is in the buffer (may be empty if the scanner did not respond).
    if (millis() - this->last_command_time_ > COMMAND_TIMEOUT_MS) {
      if (!this->rx_buffer_.empty()) {
        this->process_version_();
      } else {
        ESP_LOGW(TAG_SCANNER, "Version request timed out with no response");
        this->clear_buffer_();
      }
      this->expected_response_ = ResponseType::NONE;
    }
    return;
  }

  // Process barcode data if available
  if (this->expected_response_ == ResponseType::BARCODE || this->expected_response_ == ResponseType::NONE) {
    bool should_process = false;

    // In host mode, we process the barcode immediately when we have data
    if (this->operation_mode_ == OperationMode::HOST) {
      should_process = true;
    } else {
      // In non-host modes, we look for the acknowledgment sequence at the end
      if (!this->rx_buffer_.empty() && this->is_ack_sequence_(this->rx_buffer_.data(), this->rx_buffer_.size())) {
        should_process = true;
      }
    }

    if (should_process && !this->rx_buffer_.empty()) {
      this->process_barcode_();

      // Reset scanning state after barcode is processed in HOST mode
      if (this->operation_mode_ == OperationMode::HOST) {
        this->set_scan_state(ScanState::IDLE);
      }

      // Set expected response type to NONE
      this->expected_response_ = ResponseType::NONE;
    }
  }

  // HOST-mode scan timeout: fire on_scan_timeout if scan_duration has elapsed without a result.
  // scan_duration_to_ms() returns 0 for UNLIMITED, in which case we never time out.
  if (this->scan_state_ == ScanState::MANUAL_SCANNING && this->scan_started_at_ != 0) {
    const uint32_t duration_ms = this->get_scan_duration_ms();
    if (duration_ms > 0 && (millis() - this->scan_started_at_) > duration_ms) {
      ESP_LOGD(TAG_SCANNER, "Scan timed out after %u ms", duration_ms);
      this->scan_started_at_ = 0;
      this->set_scan_state(ScanState::IDLE);
      this->expected_response_ = ResponseType::NONE;
      this->scan_timeout_callback_();
    }
  }
}

void BarcodeScanner::dump_config() {
  ESP_LOGCONFIG(TAG_SCANNER, "M5Stack Barcode Scanner:");
  ESP_LOGCONFIG(TAG_SCANNER, "  Operation Mode: %s", operation_mode_to_string(this->operation_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Terminator: %s", terminator_to_string(this->terminator_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Light Mode: %s", light_mode_to_string(this->light_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Locate Light Mode: %s", locate_light_mode_to_string(this->locate_light_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Sound Mode: %s", sound_mode_to_string(this->sound_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Buzzer Volume: %s", buzzer_volume_to_string(this->buzzer_volume_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Decoding Success Light: %s",
                decoding_success_light_mode_to_string(this->decoding_success_light_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Boot Sound: %s", boot_sound_mode_to_string(this->boot_sound_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Decode Sound: %s", decode_sound_mode_to_string(this->decode_sound_mode_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Scan Duration: %s", scan_duration_to_string(this->scan_duration_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Stable Induction Time: %s",
                stable_induction_time_to_string(this->stable_induction_time_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Reading Interval: %s", reading_interval_to_string(this->reading_interval_));
  ESP_LOGCONFIG(TAG_SCANNER, "  Same Code Interval: %s", same_code_interval_to_string(this->same_code_interval_));

  if (this->version_sensor_ != nullptr && this->version_sensor_->has_state()) {
    ESP_LOGCONFIG(TAG_SCANNER, "  Firmware Version: %s", this->version_sensor_->get_state().c_str());
  }
}

// Buffer Management Methods
void BarcodeScanner::clear_buffer_() { this->rx_buffer_.clear(); }

void BarcodeScanner::read_buffer_() {
  // Read available data
  while (this->available()) {
    uint8_t byte = this->read();
    this->rx_buffer_.push_back(byte);
  }
}

void BarcodeScanner::set_expected_response_(ResponseType type) { this->expected_response_ = type; }

// Command Processing Methods
void BarcodeScanner::process_command_queue_() {
  // Only process commands if we're not waiting for an acknowledgment or a version response
  if (this->waiting_for_ack_ || this->command_queue_.empty()) {
    return;
  }
  if (this->expected_response_ == ResponseType::VERSION) {
    return;  // Block queue while waiting for version data to arrive
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
      // Should not get here - this state is handled in the response path
      this->command_state_ = CommandState::IDLE;
      break;
  }
}

// Add new method implementation before wake_up_
void BarcodeScanner::write_command_(const std::unique_ptr<CommandBase> &command) {
  // Now we can send the actual command
  command->log_command_data(TAG_SCANNER, "Sending");

  // Set the expected response type based on the command
  this->set_expected_response_(command->get_expected_response());

  this->write_array(command->get_data(), command->get_length());

  // Update state tracking.  VERSION commands are special: the scanner responds
  // with raw version data (not an ACK byte sequence), so we must not block on
  // the ACK path.  The VERSION response is handled by the dedicated branch in
  // loop() once COMMAND_TIMEOUT_MS has elapsed to let all data arrive.
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

void BarcodeScanner::queue_command(std::unique_ptr<CommandBase> command) {
  if (command == nullptr) {
    ESP_LOGW(TAG_SCANNER, "Attempted to queue null command");
    return;
  }

  // Check if queue is full
  if (this->command_queue_.size() >= MAX_QUEUE_SIZE) {
    ESP_LOGW(TAG_SCANNER, "Command queue full (size=%u), dropping command: %s", this->command_queue_.size(),
             command->get_description());
    return;
  }

  command->log_command_data(TAG_SCANNER, "Queuing");
  this->command_queue_.push_back(std::move(command));
}

bool BarcodeScanner::is_ack_sequence_(const uint8_t *data, size_t len, size_t offset) const {
  // Enhanced input validation
  if (data == nullptr) {
    ESP_LOGW(TAG_SCANNER, "Null data pointer passed to is_ack_sequence_");
    return false;
  }

  // Check if there's enough data to contain an ACK sequence
  if (this->operation_mode_ == OperationMode::HOST) {
    // Host mode uses 6-byte ACK
    if (len < Commands::Responses::ACK_SIZE + offset) {
      return false;
    }

    // Check against standard ACK
    const uint8_t *ack = Commands::Responses::ACK;
    return memcmp(data + offset, ack, Commands::Responses::ACK_SIZE) == 0;
  } else {
    // Non-host mode uses 7-byte ACK
    if (len < Commands::Responses::NON_HOST_ACK_SIZE + offset) {
      return false;
    }

    const uint8_t *ack = Commands::Responses::NON_HOST_ACK;
    return memcmp(data + offset, ack, Commands::Responses::NON_HOST_ACK_SIZE) == 0;
  }
}

// Response Processing Methods
void BarcodeScanner::process_barcode_() {
  if (this->rx_buffer_.empty()) {
    return;
  }

  // If we're in continuous mode and have a terminator, we need to handle it
  size_t data_length = this->rx_buffer_.size();
  if (this->is_continuous_mode() && this->terminator_ != Terminator::NONE) {
    // Find the terminator sequence
    size_t term_pos = 0;
    switch (this->terminator_) {
      case Terminator::CRLF:
        term_pos = data_length - 2;
        if (data_length >= 2 && this->rx_buffer_[term_pos] == '\r' && this->rx_buffer_[term_pos + 1] == '\n') {
          data_length = term_pos;
        }
        break;
      case Terminator::CR:
        term_pos = data_length - 1;
        if (data_length >= 1 && this->rx_buffer_[term_pos] == '\r') {
          data_length = term_pos;
        }
        break;
      case Terminator::TAB:
        term_pos = data_length - 1;
        if (data_length >= 1 && this->rx_buffer_[term_pos] == '\t') {
          data_length = term_pos;
        }
        break;
      case Terminator::CRCR:
        term_pos = data_length - 2;
        if (data_length >= 2 && this->rx_buffer_[term_pos] == '\r' && this->rx_buffer_[term_pos + 1] == '\r') {
          data_length = term_pos;
        }
        break;
      case Terminator::CRLFCRLF:
        term_pos = data_length - 4;
        if (data_length >= 4 && this->rx_buffer_[term_pos] == '\r' && this->rx_buffer_[term_pos + 1] == '\n' &&
            this->rx_buffer_[term_pos + 2] == '\r' && this->rx_buffer_[term_pos + 3] == '\n') {
          data_length = term_pos;
        }
        break;
      default:
        break;
    }
  }

  // Add bounds checking to prevent buffer overflows
  if (data_length > MAX_BARCODE_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Barcode too long (%u bytes), truncating to %u bytes", data_length, MAX_BARCODE_LENGTH);
    data_length = MAX_BARCODE_LENGTH;
  }

  // Create a string from the received data with explicit size limit
  std::string barcode;
  if (data_length > 0) {
    barcode.assign(reinterpret_cast<char *>(this->rx_buffer_.data()), data_length);

    // Barcode received — cancel any pending scan timeout
    this->scan_started_at_ = 0;

    ESP_LOGD(TAG_SCANNER, "Barcode received: %s", barcode.c_str());

    // Publish the barcode to optional text sensor
    if (this->text_sensor_ != nullptr) {
      this->text_sensor_->publish_state(barcode);
    }

    // Fire on_barcode automation trigger
    this->barcode_callback_(barcode);

    // Trigger the barcode scanned event
    if (this->barcode_event_ != nullptr) {
      this->barcode_event_->trigger("scan_successful");
      ESP_LOGD(TAG_SCANNER, "Scanner event triggered: scan_successful");
    }
  }

  // Clear the buffer for the next barcode
  this->clear_buffer_();
}

void BarcodeScanner::process_version_() {
  if (this->rx_buffer_.empty()) {
    return;
  }

  // Add bounds checking to prevent buffer overflows
  if (this->rx_buffer_.size() > MAX_VERSION_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Version string too long (%u bytes), truncating to %u bytes", this->rx_buffer_.size(),
             MAX_VERSION_LENGTH);
    this->rx_buffer_.resize(MAX_VERSION_LENGTH);
  }

  // Convert the buffer to a string with explicit size limit
  std::string version;
  version.assign(reinterpret_cast<char *>(this->rx_buffer_.data()), this->rx_buffer_.size());

  // Publish the version
  if (this->version_sensor_ != nullptr) {
    this->version_sensor_->publish_state(version);
    ESP_LOGD(TAG_SCANNER, "Firmware version: %s", version.c_str());
  }

  // Clear the buffer
  this->clear_buffer_();
}

void BarcodeScanner::request_version_() {
  // Request the firmware version
  auto command = CommandFactory::create_version_command();

  // Queue the command
  this->queue_command(std::move(command));
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

  // Create and queue the start command
  auto command = CommandFactory::create_start_command();
  this->queue_command(std::move(command));

  // Record when scanning started so we can fire on_scan_timeout after scan_duration elapses
  this->scan_started_at_ = millis();

  // Update state
  this->set_scan_state(ScanState::MANUAL_SCANNING);

  // Set expected response type to BARCODE
  this->set_expected_response_(ResponseType::BARCODE);
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

  // Create and queue the stop command
  auto command = CommandFactory::create_stop_command();
  this->queue_command(std::move(command));

  // Cancel any pending scan timeout
  this->scan_started_at_ = 0;

  // Update state
  this->set_scan_state(ScanState::IDLE);
}

// Scanner Settings Methods
void BarcodeScanner::set_operation_mode(OperationMode mode) {
  if (mode == this->operation_mode_) {
    ESP_LOGD(TAG_SCANNER, "Operation mode already set to %s", operation_mode_to_string(mode));
    return;
  }

  // Create and queue the mode command
  auto command = CommandFactory::create_mode_command(mode);
  this->queue_command(std::move(command));

  // Update scan state based on new mode (this needs to happen immediately for proper operation)
  if (mode == OperationMode::CONTINUOUS || mode == OperationMode::AUTO_SENSE) {
    this->set_scan_state(ScanState::CONTINUOUS_SCANNING);
  } else if (this->get_scan_state() == ScanState::CONTINUOUS_SCANNING) {
    // If we're leaving continuous mode, reset the scan state
    this->set_scan_state(ScanState::IDLE);
  }
}

void BarcodeScanner::set_terminator(Terminator term) {
  if (term == this->terminator_) {
    ESP_LOGD(TAG_SCANNER, "Terminator already set to %s", terminator_to_string(term));
    return;
  }

  // Create and queue the terminator command
  auto command = CommandFactory::create_terminator_command(term);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_light_mode(LightMode mode) {
  if (mode == this->light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Light mode already set to %s", light_mode_to_string(mode));
    return;
  }

  // Create and queue the light mode command
  auto command = CommandFactory::create_light_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_locate_light_mode(LocateLightMode mode) {
  if (mode == this->locate_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Locate light mode already set to %s", locate_light_mode_to_string(mode));
    return;
  }

  // Create and queue the locate light mode command
  auto command = CommandFactory::create_locate_light_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_sound_mode(SoundMode mode) {
  if (mode == this->sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Sound mode already set to %s", sound_mode_to_string(mode));
    return;
  }

  // Create and queue the sound mode command
  auto command = CommandFactory::create_sound_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_buzzer_volume(BuzzerVolume volume) {
  if (volume == this->buzzer_volume_) {
    ESP_LOGD(TAG_SCANNER, "Buzzer volume already set to %s", buzzer_volume_to_string(volume));
    return;
  }

  // Create and queue the volume command
  auto command = CommandFactory::create_volume_command(volume);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_decoding_success_light_mode(DecodingSuccessLightMode mode) {
  if (mode == this->decoding_success_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decoding success light mode already set to %s", decoding_success_light_mode_to_string(mode));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_decoding_success_light_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_boot_sound_mode(BootSoundMode mode) {
  if (mode == this->boot_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Boot sound mode already set to %s", boot_sound_mode_to_string(mode));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_boot_sound_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_decode_sound_mode(DecodeSoundMode mode) {
  if (mode == this->decode_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decode sound mode already set to %s", decode_sound_mode_to_string(mode));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_decode_sound_command(mode);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_scan_duration(ScanDuration duration) {
  if (duration == this->scan_duration_) {
    ESP_LOGD(TAG_SCANNER, "Scan duration already set to %s", scan_duration_to_string(duration));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_scan_duration_command(duration);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_stable_induction_time(StableInductionTime time) {
  if (time == this->stable_induction_time_) {
    ESP_LOGD(TAG_SCANNER, "Stable induction time already set to %s", stable_induction_time_to_string(time));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_stable_induction_time_command(time);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_reading_interval(ReadingInterval interval) {
  if (interval == this->reading_interval_) {
    ESP_LOGD(TAG_SCANNER, "Reading interval already set to %s", reading_interval_to_string(interval));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_reading_interval_command(interval);
  this->queue_command(std::move(command));
}

void BarcodeScanner::set_same_code_interval(SameCodeInterval interval) {
  if (interval == this->same_code_interval_) {
    ESP_LOGD(TAG_SCANNER, "Same code interval already set to %s", same_code_interval_to_string(interval));
    return;
  }

  // Create and queue the command
  auto command = CommandFactory::create_same_code_interval_command(interval);
  this->queue_command(std::move(command));
}

void BarcodeScanner::process_current_buffer() {
  // First ensure we have the latest data
  this->read_buffer_();

  // Then process the barcode using existing logic
  this->process_barcode_();

  // In HOST mode, we should set the scanner back to IDLE state after processing
  if (this->operation_mode_ == OperationMode::HOST && this->get_scan_state() != ScanState::IDLE) {
    this->set_scan_state(ScanState::IDLE);
  }
}

// Protected state setter implementations.
// Each setter is called from StateCommand::on_success() after the scanner ACKs the command.
// After updating the in-memory state, save_settings_() persists all settings to NVS so that
// the next boot can skip re-sending settings that are already applied.

void BarcodeScanner::set_terminator_state(Terminator term) {
  ESP_LOGD(TAG_SCANNER, "Setting terminator to %s", terminator_to_string(term));
  this->terminator_ = term;
  this->save_settings_();
}

void BarcodeScanner::set_light_mode_state(LightMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting light mode to %s", light_mode_to_string(mode));
  this->light_mode_ = mode;
  this->save_settings_();
  if (this->light_mode_select_ != nullptr)
    this->light_mode_select_->publish_state(LightModeSelect::mode_to_key(mode));
}

void BarcodeScanner::set_locate_light_mode_state(LocateLightMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting locate light mode to %s", locate_light_mode_to_string(mode));
  this->locate_light_mode_ = mode;
  this->save_settings_();
  if (this->locate_light_mode_select_ != nullptr)
    this->locate_light_mode_select_->publish_state(LocateLightModeSelect::mode_to_key(mode));
}

void BarcodeScanner::set_sound_mode_state(SoundMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting sound mode to %s", sound_mode_to_string(mode));
  this->sound_mode_ = mode;
  this->save_settings_();
  if (this->sound_switch_ != nullptr)
    this->sound_switch_->publish_state(mode == SoundMode::SOUND_ENABLED);
}

void BarcodeScanner::set_buzzer_volume_state(BuzzerVolume volume) {
  ESP_LOGD(TAG_SCANNER, "Setting buzzer volume to %s", buzzer_volume_to_string(volume));
  this->buzzer_volume_ = volume;
  this->save_settings_();
  if (this->buzzer_volume_select_ != nullptr)
    this->buzzer_volume_select_->publish_state(BuzzerVolumeSelect::volume_to_key(volume));
}

void BarcodeScanner::set_decoding_success_light_mode_state(DecodingSuccessLightMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting decoding success light mode to %s", decoding_success_light_mode_to_string(mode));
  this->decoding_success_light_mode_ = mode;
  this->save_settings_();
  if (this->decoding_success_light_switch_ != nullptr)
    this->decoding_success_light_switch_->publish_state(mode == DecodingSuccessLightMode::DECODING_LIGHT_ENABLED);
}

void BarcodeScanner::set_boot_sound_mode_state(BootSoundMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting boot sound mode to %s", boot_sound_mode_to_string(mode));
  this->boot_sound_mode_ = mode;
  this->save_settings_();
  if (this->boot_sound_switch_ != nullptr)
    this->boot_sound_switch_->publish_state(mode == BootSoundMode::BOOT_SOUND_ENABLED);
}

void BarcodeScanner::set_decode_sound_mode_state(DecodeSoundMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting decode sound mode to %s", decode_sound_mode_to_string(mode));
  this->decode_sound_mode_ = mode;
  this->save_settings_();
  if (this->decode_sound_switch_ != nullptr)
    this->decode_sound_switch_->publish_state(mode == DecodeSoundMode::DECODE_SOUND_ENABLED);
}

void BarcodeScanner::set_scan_duration_state(ScanDuration duration) {
  ESP_LOGD(TAG_SCANNER, "Setting scan duration to %s", scan_duration_to_string(duration));
  this->scan_duration_ = duration;
  this->save_settings_();
  if (this->scan_duration_select_ != nullptr)
    this->scan_duration_select_->publish_state(ScanDurationSelect::duration_to_key(duration));
}

void BarcodeScanner::set_stable_induction_time_state(StableInductionTime time) {
  ESP_LOGD(TAG_SCANNER, "Setting stable induction time to %s", stable_induction_time_to_string(time));
  this->stable_induction_time_ = time;
  this->save_settings_();
}

void BarcodeScanner::set_reading_interval_state(ReadingInterval interval) {
  ESP_LOGD(TAG_SCANNER, "Setting reading interval to %s", reading_interval_to_string(interval));
  this->reading_interval_ = interval;
  this->save_settings_();
}

void BarcodeScanner::set_same_code_interval_state(SameCodeInterval interval) {
  ESP_LOGD(TAG_SCANNER, "Setting same code interval to %s", same_code_interval_to_string(interval));
  this->same_code_interval_ = interval;
  this->save_settings_();
}

void BarcodeScanner::set_operation_mode_state(OperationMode mode) {
  ESP_LOGD(TAG_SCANNER, "Setting operation mode to %s", operation_mode_to_string(mode));
  this->operation_mode_ = mode;
  this->save_settings_();
  // Keep the HA select entity in sync after the scanner ACKs the command
  if (this->operation_mode_select_ != nullptr) {
    this->operation_mode_select_->publish_state(OperationModeSelect::mode_to_key(mode));
  }
}

// OperationModeSelect — routes HA select changes to the scanner command queue
void OperationModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "OperationModeSelect: no scanner attached");
    return;
  }
  OperationMode mode;
  if (!parse_operation_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "OperationModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_operation_mode(mode);
  // publish_state is deferred until set_operation_mode_state() fires after the scanner ACKs
}

void BuzzerVolumeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "BuzzerVolumeSelect: no scanner attached");
    return;
  }
  BuzzerVolume vol;
  if (!parse_buzzer_volume(value, vol)) {
    ESP_LOGW(TAG_SCANNER, "BuzzerVolumeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_buzzer_volume(vol);
}

void LightModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "LightModeSelect: no scanner attached");
    return;
  }
  LightMode mode;
  if (!parse_light_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "LightModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_light_mode(mode);
}

void LocateLightModeSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "LocateLightModeSelect: no scanner attached");
    return;
  }
  LocateLightMode mode;
  if (!parse_locate_light_mode(value, mode)) {
    ESP_LOGW(TAG_SCANNER, "LocateLightModeSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_locate_light_mode(mode);
}

void ScanDurationSelect::control(const std::string &value) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "ScanDurationSelect: no scanner attached");
    return;
  }
  ScanDuration dur;
  if (!parse_scan_duration(value, dur)) {
    ESP_LOGW(TAG_SCANNER, "ScanDurationSelect: unknown value '%s'", value.c_str());
    return;
  }
  scanner_->set_scan_duration(dur);
}

void SoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "SoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_sound_mode(state ? SoundMode::SOUND_ENABLED : SoundMode::SOUND_DISABLED);
  // publish_state deferred until set_sound_mode_state() fires after the scanner ACKs
}

void BootSoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "BootSoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_boot_sound_mode(state ? BootSoundMode::BOOT_SOUND_ENABLED : BootSoundMode::BOOT_SOUND_DISABLED);
}

void DecodeSoundSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "DecodeSoundSwitch: no scanner attached");
    return;
  }
  scanner_->set_decode_sound_mode(state ? DecodeSoundMode::DECODE_SOUND_ENABLED
                                        : DecodeSoundMode::DECODE_SOUND_DISABLED);
}

void DecodingSuccessLightSwitch::write_state(bool state) {
  if (scanner_ == nullptr) {
    ESP_LOGW(TAG_SCANNER, "DecodingSuccessLightSwitch: no scanner attached");
    return;
  }
  scanner_->set_decoding_success_light_mode(state ? DecodingSuccessLightMode::DECODING_LIGHT_ENABLED
                                                  : DecodingSuccessLightMode::DECODING_LIGHT_DISABLED);
}

void StartButton::press_action() {
  if (scanner_ != nullptr)
    scanner_->start_scan();
}

void StopButton::press_action() {
  if (scanner_ != nullptr)
    scanner_->stop_scan();
}

}  // namespace m5stack_barcode
}  // namespace esphome
