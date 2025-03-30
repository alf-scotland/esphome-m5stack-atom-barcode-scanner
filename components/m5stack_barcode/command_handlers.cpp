#include "command_handlers.h"

#include "commands.h"
#include "esphome/core/log.h"
#include "m5stack_barcode.h"
#include "types.h"
#include <memory>

namespace esphome {
namespace m5stack_barcode {

static const char *const TAG_CMD = "m5stack_barcode.cmd";
extern const char *const TAG_SCANNER;

// Custom implementation of make_unique for C++11
template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Add helper method implementation from header
void CommandBase::log_command_data(const char *tag, const char *prefix) const {
  if (this->get_data() == nullptr || this->get_length() == 0) {
    ESP_LOGW(tag, "%s Command data is empty", prefix);
    return;
  }

  constexpr size_t MAX_LOG_LENGTH = 64;  // Maximum buffer size for logging
  char hex_buffer[MAX_LOG_LENGTH] = {0};
  size_t log_len = std::min(this->get_length(), size_t(16));  // Log at most 16 bytes

  for (size_t i = 0; i < log_len; i++) {
    snprintf(hex_buffer + (i * 3), 4, "%02X ", this->get_data()[i]);
  }

  if (log_len < this->get_length()) {
    snprintf(hex_buffer + (log_len * 3), 5, "...");
  }

  ESP_LOGD(tag, "%s Command [%s]: %s", prefix, this->get_description(), hex_buffer);
}

// SimpleCommand constructor implementation
SimpleCommand::SimpleCommand(const uint8_t *data, size_t length, const char *description, SuccessCallback on_success,
                             FailureCallback on_failure, ResponseType expected_response)
    : data_(data),
      length_(length),
      description_(description),
      expected_response_(expected_response),
      on_success_([on_success](BarcodeScanner *scanner) {
        if (on_success)
          on_success(scanner);
      }),
      on_failure_([on_failure](BarcodeScanner *scanner) {
        if (on_failure)
          on_failure(scanner);
      }) {}

// StateCommand template specializations
template<> void StateCommand<Terminator>::on_success(BarcodeScanner *scanner) { scanner->set_terminator_state(value_); }

template<> void StateCommand<LightMode>::on_success(BarcodeScanner *scanner) { scanner->set_light_mode_state(value_); }

template<> void StateCommand<LocateLightMode>::on_success(BarcodeScanner *scanner) {
  scanner->set_locate_light_mode_state(value_);
}

template<> void StateCommand<SoundMode>::on_success(BarcodeScanner *scanner) { scanner->set_sound_mode_state(value_); }

template<> void StateCommand<BuzzerVolume>::on_success(BarcodeScanner *scanner) {
  scanner->set_buzzer_volume_state(value_);
}

template<> void StateCommand<DecodingSuccessLightMode>::on_success(BarcodeScanner *scanner) {
  scanner->set_decoding_success_light_mode_state(value_);
}

template<> void StateCommand<BootSoundMode>::on_success(BarcodeScanner *scanner) {
  scanner->set_boot_sound_mode_state(value_);
}

template<> void StateCommand<DecodeSoundMode>::on_success(BarcodeScanner *scanner) {
  scanner->set_decode_sound_mode_state(value_);
}

template<> void StateCommand<ScanDuration>::on_success(BarcodeScanner *scanner) {
  scanner->set_scan_duration_state(value_);
}

template<> void StateCommand<StableInductionTime>::on_success(BarcodeScanner *scanner) {
  scanner->set_stable_induction_time_state(value_);
}

template<> void StateCommand<ReadingInterval>::on_success(BarcodeScanner *scanner) {
  scanner->set_reading_interval_state(value_);
}

template<> void StateCommand<SameCodeInterval>::on_success(BarcodeScanner *scanner) {
  scanner->set_same_code_interval_state(value_);
}

template<> void StateCommand<OperationMode>::on_success(BarcodeScanner *scanner) {
  scanner->set_operation_mode_state(value_);
}

// Scanner basic commands
std::unique_ptr<CommandBase> CommandFactory::create_start_command() {
  return make_unique<SimpleCommand>(
      Commands::START_SCAN, Commands::START_SCAN_SIZE, "Start scanning",
      [](BarcodeScanner *scanner) { scanner->set_scan_state(ScanState::MANUAL_SCANNING); },
      [](BarcodeScanner *scanner) { scanner->set_scan_state(ScanState::IDLE); }, ResponseType::BARCODE);
}

std::unique_ptr<CommandBase> CommandFactory::create_stop_command() {
  return make_unique<SimpleCommand>(
      Commands::STOP_SCAN, Commands::STOP_SCAN_SIZE, "Stop scanning",
      [](BarcodeScanner *scanner) { scanner->set_scan_state(ScanState::IDLE); }, nullptr);
}

std::unique_ptr<CommandBase> CommandFactory::create_version_command() {
  return make_unique<SimpleCommand>(Commands::GET_VERSION, Commands::GET_VERSION_SIZE, "Get Version", nullptr, nullptr,
                                    ResponseType::VERSION);
}

// Setting commands
std::unique_ptr<CommandBase> CommandFactory::create_mode_command(OperationMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case OperationMode::HOST:
      cmd_data = Commands::Mode::HOST;
      break;
    case OperationMode::LEVEL:
      cmd_data = Commands::Mode::LEVEL;
      break;
    case OperationMode::PULSE:
      cmd_data = Commands::Mode::PULSE;
      break;
    case OperationMode::CONTINUOUS:
      cmd_data = Commands::Mode::CONTINUOUS;
      break;
    case OperationMode::AUTO_SENSE:
      cmd_data = Commands::Mode::AUTO_SENSE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid operation mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<OperationMode>>(cmd_data, Commands::Mode::SIZE, mode, operation_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_terminator_command(Terminator term) {
  const uint8_t *cmd_data = nullptr;

  switch (term) {
    case Terminator::NONE:
      cmd_data = Commands::Terminator::NONE;
      break;
    case Terminator::CRLF:
      cmd_data = Commands::Terminator::CRLF;
      break;
    case Terminator::CR:
      cmd_data = Commands::Terminator::CR;
      break;
    case Terminator::TAB:
      cmd_data = Commands::Terminator::TAB;
      break;
    case Terminator::CRCR:
      cmd_data = Commands::Terminator::CRCR;
      break;
    case Terminator::CRLFCRLF:
      cmd_data = Commands::Terminator::CRLFCRLF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid terminator: %d", static_cast<uint8_t>(term));
      return nullptr;
  }

  return make_unique<StateCommand<Terminator>>(cmd_data, Commands::Terminator::SIZE, term, terminator_to_string(term));
}

std::unique_ptr<CommandBase> CommandFactory::create_light_command(LightMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case LightMode::LIGHT_ON_WHEN_READING:
      cmd_data = Commands::Light::ON_WHEN_READING;
      break;
    case LightMode::LIGHT_ALWAYS_ON:
      cmd_data = Commands::Light::ALWAYS_ON;
      break;
    case LightMode::LIGHT_ALWAYS_OFF:
      cmd_data = Commands::Light::ALWAYS_OFF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<LightMode>>(cmd_data, Commands::Light::SIZE, mode, light_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_locate_light_command(LocateLightMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING:
      cmd_data = Commands::LocateLight::ON_WHEN_READING;
      break;
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_ON:
      cmd_data = Commands::LocateLight::ALWAYS_ON;
      break;
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF:
      cmd_data = Commands::LocateLight::ALWAYS_OFF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid locate light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<LocateLightMode>>(cmd_data, Commands::LocateLight::SIZE, mode,
                                                    locate_light_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_sound_command(SoundMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case SoundMode::SOUND_DISABLED:
      cmd_data = Commands::Sound::SOUND_DISABLED;
      break;
    case SoundMode::SOUND_ENABLED:
      cmd_data = Commands::Sound::SOUND_ENABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<SoundMode>>(cmd_data, Commands::Sound::SIZE, mode, sound_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_volume_command(BuzzerVolume volume) {
  const uint8_t *cmd_data = nullptr;

  switch (volume) {
    case BuzzerVolume::BUZZER_VOLUME_HIGH:
      cmd_data = Commands::Volume::VOLUME_HIGH;
      break;
    case BuzzerVolume::BUZZER_VOLUME_MEDIUM:
      cmd_data = Commands::Volume::VOLUME_MEDIUM;
      break;
    case BuzzerVolume::BUZZER_VOLUME_LOW:
      cmd_data = Commands::Volume::VOLUME_LOW;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid buzzer volume: %d", static_cast<uint8_t>(volume));
      return nullptr;
  }

  return make_unique<StateCommand<BuzzerVolume>>(cmd_data, Commands::Volume::SIZE, volume,
                                                 buzzer_volume_to_string(volume));
}

// New command factory methods for additional settings

std::unique_ptr<CommandBase> CommandFactory::create_decoding_success_light_command(DecodingSuccessLightMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case DecodingSuccessLightMode::DECODING_LIGHT_ENABLED:
      cmd_data = Commands::DecodingSuccessLight::LIGHT_ENABLED;
      break;
    case DecodingSuccessLightMode::DECODING_LIGHT_DISABLED:
      cmd_data = Commands::DecodingSuccessLight::LIGHT_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid decoding success light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<DecodingSuccessLightMode>>(cmd_data, Commands::DecodingSuccessLight::SIZE, mode,
                                                             decoding_success_light_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_boot_sound_command(BootSoundMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case BootSoundMode::BOOT_SOUND_ENABLED:
      cmd_data = Commands::BootSound::BOOT_SOUND_ENABLED;
      break;
    case BootSoundMode::BOOT_SOUND_DISABLED:
      cmd_data = Commands::BootSound::BOOT_SOUND_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid boot sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<BootSoundMode>>(cmd_data, Commands::BootSound::SIZE, mode,
                                                  boot_sound_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_decode_sound_command(DecodeSoundMode mode) {
  const uint8_t *cmd_data = nullptr;

  switch (mode) {
    case DecodeSoundMode::DECODE_SOUND_ENABLED:
      cmd_data = Commands::DecodeSound::DECODE_SOUND_ENABLED;
      break;
    case DecodeSoundMode::DECODE_SOUND_DISABLED:
      cmd_data = Commands::DecodeSound::DECODE_SOUND_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid decode sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }

  return make_unique<StateCommand<DecodeSoundMode>>(cmd_data, Commands::DecodeSound::SIZE, mode,
                                                    decode_sound_mode_to_string(mode));
}

std::unique_ptr<CommandBase> CommandFactory::create_scan_duration_command(ScanDuration duration) {
  const uint8_t *cmd_data = nullptr;

  switch (duration) {
    case ScanDuration::MS_500:
      cmd_data = Commands::ScanDuration::MS_500;
      break;
    case ScanDuration::MS_1000:
      cmd_data = Commands::ScanDuration::MS_1000;
      break;
    case ScanDuration::MS_3000:
      cmd_data = Commands::ScanDuration::MS_3000;
      break;
    case ScanDuration::MS_5000:
      cmd_data = Commands::ScanDuration::MS_5000;
      break;
    case ScanDuration::MS_10000:
      cmd_data = Commands::ScanDuration::MS_10000;
      break;
    case ScanDuration::MS_15000:
      cmd_data = Commands::ScanDuration::MS_15000;
      break;
    case ScanDuration::MS_20000:
      cmd_data = Commands::ScanDuration::MS_20000;
      break;
    case ScanDuration::UNLIMITED:
      cmd_data = Commands::ScanDuration::UNLIMITED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid scan duration: %d", static_cast<uint8_t>(duration));
      return nullptr;
  }

  return make_unique<StateCommand<ScanDuration>>(cmd_data, Commands::ScanDuration::SIZE, duration,
                                                 scan_duration_to_string(duration));
}

std::unique_ptr<CommandBase> CommandFactory::create_stable_induction_time_command(StableInductionTime time) {
  const uint8_t *cmd_data = nullptr;

  switch (time) {
    case StableInductionTime::MS_0:
      cmd_data = Commands::StableInductionTime::MS_0;
      break;
    case StableInductionTime::MS_100:
      cmd_data = Commands::StableInductionTime::MS_100;
      break;
    case StableInductionTime::MS_300:
      cmd_data = Commands::StableInductionTime::MS_300;
      break;
    case StableInductionTime::MS_500:
      cmd_data = Commands::StableInductionTime::MS_500;
      break;
    case StableInductionTime::MS_1000:
      cmd_data = Commands::StableInductionTime::MS_1000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid stable induction time: %d", static_cast<uint8_t>(time));
      return nullptr;
  }

  return make_unique<StateCommand<StableInductionTime>>(cmd_data, Commands::StableInductionTime::SIZE, time,
                                                        stable_induction_time_to_string(time));
}

std::unique_ptr<CommandBase> CommandFactory::create_reading_interval_command(ReadingInterval interval) {
  const uint8_t *cmd_data = nullptr;

  switch (interval) {
    case ReadingInterval::MS_0:
      cmd_data = Commands::ReadingInterval::MS_0;
      break;
    case ReadingInterval::MS_100:
      cmd_data = Commands::ReadingInterval::MS_100;
      break;
    case ReadingInterval::MS_300:
      cmd_data = Commands::ReadingInterval::MS_300;
      break;
    case ReadingInterval::MS_500:
      cmd_data = Commands::ReadingInterval::MS_500;
      break;
    case ReadingInterval::MS_1000:
      cmd_data = Commands::ReadingInterval::MS_1000;
      break;
    case ReadingInterval::MS_1500:
      cmd_data = Commands::ReadingInterval::MS_1500;
      break;
    case ReadingInterval::MS_2000:
      cmd_data = Commands::ReadingInterval::MS_2000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid reading interval: %d", static_cast<uint8_t>(interval));
      return nullptr;
  }

  return make_unique<StateCommand<ReadingInterval>>(cmd_data, Commands::ReadingInterval::SIZE, interval,
                                                    reading_interval_to_string(interval));
}

std::unique_ptr<CommandBase> CommandFactory::create_same_code_interval_command(SameCodeInterval interval) {
  const uint8_t *cmd_data = nullptr;

  switch (interval) {
    case SameCodeInterval::MS_0:
      cmd_data = Commands::SameCodeInterval::MS_0;
      break;
    case SameCodeInterval::MS_100:
      cmd_data = Commands::SameCodeInterval::MS_100;
      break;
    case SameCodeInterval::MS_300:
      cmd_data = Commands::SameCodeInterval::MS_300;
      break;
    case SameCodeInterval::MS_500:
      cmd_data = Commands::SameCodeInterval::MS_500;
      break;
    case SameCodeInterval::MS_1000:
      cmd_data = Commands::SameCodeInterval::MS_1000;
      break;
    case SameCodeInterval::MS_1500:
      cmd_data = Commands::SameCodeInterval::MS_1500;
      break;
    case SameCodeInterval::MS_2000:
      cmd_data = Commands::SameCodeInterval::MS_2000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid same code interval: %d", static_cast<uint8_t>(interval));
      return nullptr;
  }

  return make_unique<StateCommand<SameCodeInterval>>(cmd_data, Commands::SameCodeInterval::SIZE, interval,
                                                     same_code_interval_to_string(interval));
}

}  // namespace m5stack_barcode
}  // namespace esphome
