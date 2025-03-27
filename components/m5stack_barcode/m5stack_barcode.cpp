#include "m5stack_barcode.h"

#include "command_handlers.h"
#include "commands.h"
#include "esphome/core/hal.h"
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

// Custom implementation of make_unique for C++11 compatibility
template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Component Lifecycle Methods
void BarcodeScanner::setup() {
  ESP_LOGCONFIG(TAG_SCANNER, "Setting up M5Stack Barcode Scanner");

  // Configure default settings first
  this->configure_defaults_();

  // Then request version information (optional)
  if (this->version_sensor_ != nullptr) {
    this->request_version_();
  }
}

void BarcodeScanner::configure_defaults_() {
  // Set operation mode
  auto mode_command = CommandFactory::create_mode_command(this->operation_mode_);
  this->queue_command(std::move(mode_command));

  // Set terminator
  auto term_command = CommandFactory::create_terminator_command(this->terminator_);
  this->queue_command(std::move(term_command));

  // Set light mode
  auto light_command = CommandFactory::create_light_command(this->light_mode_);
  this->queue_command(std::move(light_command));

  // Set locate light mode
  auto locate_light_command = CommandFactory::create_locate_light_command(this->locate_light_mode_);
  this->queue_command(std::move(locate_light_command));

  // Set sound mode
  auto sound_command = CommandFactory::create_sound_command(this->sound_mode_);
  this->queue_command(std::move(sound_command));

  // Set buzzer volume
  auto volume_command = CommandFactory::create_volume_command(this->buzzer_volume_);
  this->queue_command(std::move(volume_command));

  // Set decoding success light mode
  auto decode_light_command = CommandFactory::create_decoding_success_light_command(this->decoding_success_light_mode_);
  this->queue_command(std::move(decode_light_command));

  // Set boot sound mode
  auto boot_sound_command = CommandFactory::create_boot_sound_command(this->boot_sound_mode_);
  this->queue_command(std::move(boot_sound_command));

  // Set decode sound mode
  auto decode_sound_command = CommandFactory::create_decode_sound_command(this->decode_sound_mode_);
  this->queue_command(std::move(decode_sound_command));

  // Set scan duration
  auto scan_duration_command = CommandFactory::create_scan_duration_command(this->scan_duration_);
  this->queue_command(std::move(scan_duration_command));

  // Set stable induction time
  auto induction_time_command = CommandFactory::create_stable_induction_time_command(this->stable_induction_time_);
  this->queue_command(std::move(induction_time_command));

  // Set reading interval
  auto reading_interval_command = CommandFactory::create_reading_interval_command(this->reading_interval_);
  this->queue_command(std::move(reading_interval_command));

  // Set same code interval
  auto same_code_interval_command = CommandFactory::create_same_code_interval_command(this->same_code_interval_);
  this->queue_command(std::move(same_code_interval_command));
}

void BarcodeScanner::loop() {
  // Process any pending commands first
  this->process_command_queue_();

  // Read available data
  this->read_buffer_();

  // Handle command acknowledgments
  if (this->waiting_for_ack_) {
    // Check if we've received an acknowledgment
    if (!this->rx_buffer_.empty() && this->is_ack_sequence_(this->rx_buffer_.data(), this->rx_buffer_.size())) {
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
      ESP_LOGW(TAG_SCANNER, "Command timed out");

      // Get the current command
      auto &command = this->command_queue_.front();

      // Mark as failed and invoke callback
      ESP_LOGD(TAG_SCANNER, "Command '%s' failed", command->get_description());
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
    // If we have data and we're not waiting for more (wait for COMMAND_TIMEOUT_MS)
    if (!this->rx_buffer_.empty() && millis() - this->last_command_time_ > COMMAND_TIMEOUT_MS) {
      // Process any data received as version information
      this->process_version_();
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

      // Reset scanning state after barcode is processed
      this->scanning_ = false;

      // Set expected response type to NONE
      this->expected_response_ = ResponseType::NONE;
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
  // Only process commands if we're not waiting for an acknowledgment
  if (this->waiting_for_ack_ || this->command_queue_.empty()) {
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
      // Should not get here - this state is handled in the response path
      this->command_state_ = CommandState::IDLE;
      break;
  }
}

// Add new method implementation before wake_up_
void BarcodeScanner::write_command_(const std::unique_ptr<CommandBase> &command) {
  // Now we can send the actual command
  ESP_LOGD(TAG_SCANNER, "Sending command: %s", command->get_description());
  command->log_command_data(TAG_SCANNER, "Sending");

  // Set the expected response type based on the command
  this->set_expected_response_(command->get_expected_response());

  this->write_array(command->get_data(), command->get_length());

  // Update state tracking
  this->waiting_for_ack_ = true;
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

  ESP_LOGD(TAG_SCANNER, "Queuing command: %s", command->get_description());
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
  static const size_t MAX_BARCODE_LENGTH = 128;
  if (data_length > MAX_BARCODE_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Barcode too long (%u bytes), truncating to %u bytes", data_length, MAX_BARCODE_LENGTH);
    data_length = MAX_BARCODE_LENGTH;
  }

  // Create a string from the received data with explicit size limit
  std::string barcode;
  if (data_length > 0) {
    barcode.assign(reinterpret_cast<char *>(this->rx_buffer_.data()), data_length);

    // Publish the barcode
    if (this->text_sensor_ != nullptr) {
      this->text_sensor_->publish_state(barcode);
      ESP_LOGD(TAG_SCANNER, "Barcode received: %s", barcode.c_str());
    }
  }

  // Update scan state in HOST mode only - continuous modes keep scanning
  if (this->operation_mode_ == OperationMode::HOST) {
    this->scanning_ = false;
    this->set_scan_state(ScanState::IDLE);
  }

  // Clear the buffer for the next barcode
  this->clear_buffer_();
}

void BarcodeScanner::process_version_() {
  if (this->rx_buffer_.empty()) {
    return;
  }

  // Add bounds checking to prevent buffer overflows
  static const size_t MAX_VERSION_LENGTH = 32;
  size_t version_length = this->rx_buffer_.size();
  if (version_length > MAX_VERSION_LENGTH) {
    ESP_LOGW(TAG_SCANNER, "Version string too long (%u bytes), truncating to %u bytes", version_length,
             MAX_VERSION_LENGTH);
    version_length = MAX_VERSION_LENGTH;
  }

  // Convert the buffer to a string with explicit size limit
  std::string version;
  version.assign(reinterpret_cast<char *>(this->rx_buffer_.data()), version_length);

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

  if (this->scanning_) {
    ESP_LOGD(TAG_SCANNER, "Scan already in progress");
    return;
  }

  ESP_LOGD(TAG_SCANNER, "Starting scan in HOST mode");

  // Create and queue the start command
  auto command = CommandFactory::create_start_command();
  this->queue_command(std::move(command));

  // Update state
  this->scanning_ = true;
  this->set_scan_state(ScanState::MANUAL_SCANNING);

  // Set expected response type to BARCODE
  this->set_expected_response_(ResponseType::BARCODE);
}

void BarcodeScanner::stop_scan() {
  if (this->operation_mode_ != OperationMode::HOST) {
    ESP_LOGW(TAG_SCANNER, "Cannot stop scan in non-HOST mode");
    return;
  }

  if (!this->scanning_) {
    ESP_LOGD(TAG_SCANNER, "No scan in progress");
    return;
  }

  ESP_LOGD(TAG_SCANNER, "Stopping scan in HOST mode");

  // Create and queue the stop command
  auto command = CommandFactory::create_stop_command();
  this->queue_command(std::move(command));

  // Update state
  this->scanning_ = false;
  this->set_scan_state(ScanState::IDLE);
}

// Scanner Settings Methods
bool BarcodeScanner::set_operation_mode(OperationMode mode) {
  if (mode == this->operation_mode_) {
    ESP_LOGD(TAG_SCANNER, "Operation mode already set to %s", operation_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting operation mode to %s", operation_mode_to_string(mode));

  // Create and queue the mode command
  auto command = CommandFactory::create_mode_command(mode);
  this->queue_command(std::move(command));

  // Update scan state based on new mode
  if (mode == OperationMode::CONTINUOUS || mode == OperationMode::AUTO_SENSE) {
    this->scanning_ = true;
    this->set_scan_state(ScanState::CONTINUOUS_SCANNING);
  } else if (this->scan_state_ == ScanState::CONTINUOUS_SCANNING) {
    // If we're leaving continuous mode, reset the scan state
    this->scanning_ = false;
    this->set_scan_state(ScanState::IDLE);
  }

  // Update internal state immediately
  this->operation_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_terminator(Terminator term) {
  if (term == this->terminator_) {
    ESP_LOGD(TAG_SCANNER, "Terminator already set to %s", terminator_to_string(term));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting terminator to %s", terminator_to_string(term));

  // Create and queue the terminator command
  auto command = CommandFactory::create_terminator_command(term);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->terminator_ = term;

  return true;
}

bool BarcodeScanner::set_light_mode(LightMode mode) {
  if (mode == this->light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Light mode already set to %s", light_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting light mode to %s", light_mode_to_string(mode));

  // Create and queue the light mode command
  auto command = CommandFactory::create_light_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->light_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_locate_light_mode(LocateLightMode mode) {
  if (mode == this->locate_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Locate light mode already set to %s", locate_light_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting locate light mode to %s", locate_light_mode_to_string(mode));

  // Create and queue the locate light mode command
  auto command = CommandFactory::create_locate_light_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->locate_light_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_sound_mode(SoundMode mode) {
  if (mode == this->sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Sound mode already set to %s", sound_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting sound mode to %s", sound_mode_to_string(mode));

  // Create and queue the sound mode command
  auto command = CommandFactory::create_sound_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->sound_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_buzzer_volume(BuzzerVolume volume) {
  if (volume == this->buzzer_volume_) {
    ESP_LOGD(TAG_SCANNER, "Buzzer volume already set to %s", buzzer_volume_to_string(volume));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting buzzer volume to %s", buzzer_volume_to_string(volume));

  // Create and queue the volume command
  auto command = CommandFactory::create_volume_command(volume);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->buzzer_volume_ = volume;

  return true;
}

bool BarcodeScanner::set_decoding_success_light_mode(DecodingSuccessLightMode mode) {
  if (mode == this->decoding_success_light_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decoding success light mode already set to %s", decoding_success_light_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting decoding success light mode to %s", decoding_success_light_mode_to_string(mode));

  // Create and queue the command
  auto command = CommandFactory::create_decoding_success_light_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->decoding_success_light_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_boot_sound_mode(BootSoundMode mode) {
  if (mode == this->boot_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Boot sound mode already set to %s", boot_sound_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting boot sound mode to %s", boot_sound_mode_to_string(mode));

  // Create and queue the command
  auto command = CommandFactory::create_boot_sound_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->boot_sound_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_decode_sound_mode(DecodeSoundMode mode) {
  if (mode == this->decode_sound_mode_) {
    ESP_LOGD(TAG_SCANNER, "Decode sound mode already set to %s", decode_sound_mode_to_string(mode));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting decode sound mode to %s", decode_sound_mode_to_string(mode));

  // Create and queue the command
  auto command = CommandFactory::create_decode_sound_command(mode);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->decode_sound_mode_ = mode;

  return true;
}

bool BarcodeScanner::set_scan_duration(ScanDuration duration) {
  if (duration == this->scan_duration_) {
    ESP_LOGD(TAG_SCANNER, "Scan duration already set to %s", scan_duration_to_string(duration));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting scan duration to %s", scan_duration_to_string(duration));

  // Create and queue the command
  auto command = CommandFactory::create_scan_duration_command(duration);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->scan_duration_ = duration;

  return true;
}

bool BarcodeScanner::set_stable_induction_time(StableInductionTime time) {
  if (time == this->stable_induction_time_) {
    ESP_LOGD(TAG_SCANNER, "Stable induction time already set to %s", stable_induction_time_to_string(time));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting stable induction time to %s", stable_induction_time_to_string(time));

  // Create and queue the command
  auto command = CommandFactory::create_stable_induction_time_command(time);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->stable_induction_time_ = time;

  return true;
}

bool BarcodeScanner::set_reading_interval(ReadingInterval interval) {
  if (interval == this->reading_interval_) {
    ESP_LOGD(TAG_SCANNER, "Reading interval already set to %s", reading_interval_to_string(interval));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting reading interval to %s", reading_interval_to_string(interval));

  // Create and queue the command
  auto command = CommandFactory::create_reading_interval_command(interval);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->reading_interval_ = interval;

  return true;
}

bool BarcodeScanner::set_same_code_interval(SameCodeInterval interval) {
  if (interval == this->same_code_interval_) {
    ESP_LOGD(TAG_SCANNER, "Same code interval already set to %s", same_code_interval_to_string(interval));
    return true;
  }

  ESP_LOGD(TAG_SCANNER, "Setting same code interval to %s", same_code_interval_to_string(interval));

  // Create and queue the command
  auto command = CommandFactory::create_same_code_interval_command(interval);
  this->queue_command(std::move(command));

  // Update internal state immediately
  this->same_code_interval_ = interval;

  return true;
}

// Command Factory Methods
std::unique_ptr<CommandBase> BarcodeScanner::create_mode_command(OperationMode mode) {
  return CommandFactory::create_mode_command(mode);
}

std::unique_ptr<CommandBase> BarcodeScanner::create_terminator_command(Terminator term) {
  return CommandFactory::create_terminator_command(term);
}

std::unique_ptr<CommandBase> BarcodeScanner::create_start_command() { return CommandFactory::create_start_command(); }

std::unique_ptr<CommandBase> BarcodeScanner::create_stop_command() { return CommandFactory::create_stop_command(); }

std::unique_ptr<CommandBase> BarcodeScanner::create_version_command() {
  return CommandFactory::create_version_command();
}

void BarcodeScanner::process_current_buffer() {
  // First ensure we have the latest data
  this->read_buffer_();

  // Then process the barcode using existing logic
  this->process_barcode_();
}

}  // namespace m5stack_barcode
}  // namespace esphome
