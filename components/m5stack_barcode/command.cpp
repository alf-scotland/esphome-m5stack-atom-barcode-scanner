#include "command.h"

#include "commands.h"
#include "esphome/core/log.h"
#include "m5stack_barcode.h"
#include "types.h"

#include <memory>

namespace esphome {
namespace m5stack_barcode {

static const char *const TAG_CMD = "m5stack_barcode.cmd";

void Command::log_command_data(const char *tag, const char *prefix) const {
  if (this->data_ == nullptr || this->length_ == 0) {
    ESP_LOGW(tag, "%s Command data is empty", prefix);
    return;
  }

  constexpr size_t MAX_LOG_LENGTH = 64;  // hex buffer: 16 bytes × 3 chars + null
  constexpr size_t MAX_LOG_BYTES = 16;   // cap on bytes logged
  char hex_buffer[MAX_LOG_LENGTH] = {0};
  size_t log_len = std::min(this->length_, MAX_LOG_BYTES);

  for (size_t i = 0; i < log_len; i++) {
    snprintf(hex_buffer + (i * 3), 4, "%02X ", this->data_[i]);
  }
  if (log_len < this->length_) {
    snprintf(hex_buffer + (log_len * 3), 5, "...");
  }

  ESP_LOGD(tag, "%s Command [%s]: %s", prefix, this->description_, hex_buffer);
}

// ── Basic commands ────────────────────────────────────────────────────────────

std::unique_ptr<Command> CommandFactory::create_start_command() {
  return std::make_unique<Command>(
      Commands::START_SCAN, Commands::START_SCAN_SIZE, "Start scanning",
      [](BarcodeScanner *s) { s->set_scan_state(ScanState::MANUAL_SCANNING); },
      [](BarcodeScanner *s) { s->set_scan_state(ScanState::IDLE); }, ResponseType::BARCODE);
}

std::unique_ptr<Command> CommandFactory::create_stop_command() {
  return std::make_unique<Command>(Commands::STOP_SCAN, Commands::STOP_SCAN_SIZE, "Stop scanning",
                                   [](BarcodeScanner *s) { s->set_scan_state(ScanState::IDLE); });
}

std::unique_ptr<Command> CommandFactory::create_version_command() {
  return std::make_unique<Command>(Commands::GET_VERSION, Commands::GET_VERSION_SIZE, "Get Version", nullptr, nullptr,
                                   ResponseType::VERSION);
}

// ── Setting commands ──────────────────────────────────────────────────────────

std::unique_ptr<Command> CommandFactory::create_mode_command(OperationMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case OperationMode::HOST:
      data = Commands::Mode::HOST;
      break;
    case OperationMode::LEVEL:
      data = Commands::Mode::LEVEL;
      break;
    case OperationMode::PULSE:
      data = Commands::Mode::PULSE;
      break;
    case OperationMode::CONTINUOUS:
      data = Commands::Mode::CONTINUOUS;
      break;
    case OperationMode::AUTO_SENSE:
      data = Commands::Mode::AUTO_SENSE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid operation mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::Mode::SIZE, operation_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_operation_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_terminator_command(Terminator term) {
  const uint8_t *data = nullptr;
  switch (term) {
    case Terminator::NONE:
      data = Commands::Terminator::NONE;
      break;
    case Terminator::CRLF:
      data = Commands::Terminator::CRLF;
      break;
    case Terminator::CR:
      data = Commands::Terminator::CR;
      break;
    case Terminator::TAB:
      data = Commands::Terminator::TAB;
      break;
    case Terminator::CRCR:
      data = Commands::Terminator::CRCR;
      break;
    case Terminator::CRLFCRLF:
      data = Commands::Terminator::CRLFCRLF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid terminator: %d", static_cast<uint8_t>(term));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::Terminator::SIZE, terminator_to_string(term),
                                   [term](BarcodeScanner *s) { s->set_terminator_state(term); });
}

std::unique_ptr<Command> CommandFactory::create_light_command(LightMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case LightMode::LIGHT_ON_WHEN_READING:
      data = Commands::Light::ON_WHEN_READING;
      break;
    case LightMode::LIGHT_ALWAYS_ON:
      data = Commands::Light::ALWAYS_ON;
      break;
    case LightMode::LIGHT_ALWAYS_OFF:
      data = Commands::Light::ALWAYS_OFF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::Light::SIZE, light_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_light_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_locate_light_command(LocateLightMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case LocateLightMode::LOCATE_LIGHT_ON_WHEN_READING:
      data = Commands::LocateLight::ON_WHEN_READING;
      break;
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_ON:
      data = Commands::LocateLight::ALWAYS_ON;
      break;
    case LocateLightMode::LOCATE_LIGHT_ALWAYS_OFF:
      data = Commands::LocateLight::ALWAYS_OFF;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid locate light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::LocateLight::SIZE, locate_light_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_locate_light_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_sound_command(SoundMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case SoundMode::SOUND_DISABLED:
      data = Commands::Sound::SOUND_DISABLED;
      break;
    case SoundMode::SOUND_ENABLED:
      data = Commands::Sound::SOUND_ENABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::Sound::SIZE, sound_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_sound_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_volume_command(BuzzerVolume volume) {
  const uint8_t *data = nullptr;
  switch (volume) {
    case BuzzerVolume::BUZZER_VOLUME_HIGH:
      data = Commands::Volume::VOLUME_HIGH;
      break;
    case BuzzerVolume::BUZZER_VOLUME_MEDIUM:
      data = Commands::Volume::VOLUME_MEDIUM;
      break;
    case BuzzerVolume::BUZZER_VOLUME_LOW:
      data = Commands::Volume::VOLUME_LOW;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid buzzer volume: %d", static_cast<uint8_t>(volume));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::Volume::SIZE, buzzer_volume_to_string(volume),
                                   [volume](BarcodeScanner *s) { s->set_buzzer_volume_state(volume); });
}

std::unique_ptr<Command> CommandFactory::create_decoding_success_light_command(DecodingSuccessLightMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case DecodingSuccessLightMode::DECODING_LIGHT_ENABLED:
      data = Commands::DecodingSuccessLight::LIGHT_ENABLED;
      break;
    case DecodingSuccessLightMode::DECODING_LIGHT_DISABLED:
      data = Commands::DecodingSuccessLight::LIGHT_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid decoding success light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::DecodingSuccessLight::SIZE,
                                   decoding_success_light_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_decoding_success_light_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_boot_sound_command(BootSoundMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case BootSoundMode::BOOT_SOUND_ENABLED:
      data = Commands::BootSound::BOOT_SOUND_ENABLED;
      break;
    case BootSoundMode::BOOT_SOUND_DISABLED:
      data = Commands::BootSound::BOOT_SOUND_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid boot sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::BootSound::SIZE, boot_sound_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_boot_sound_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_decode_sound_command(DecodeSoundMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case DecodeSoundMode::DECODE_SOUND_ENABLED:
      data = Commands::DecodeSound::DECODE_SOUND_ENABLED;
      break;
    case DecodeSoundMode::DECODE_SOUND_DISABLED:
      data = Commands::DecodeSound::DECODE_SOUND_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid decode sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::DecodeSound::SIZE, decode_sound_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_decode_sound_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_scan_duration_command(ScanDuration duration) {
  const uint8_t *data = nullptr;
  switch (duration) {
    case ScanDuration::MS_500:
      data = Commands::ScanDuration::MS_500;
      break;
    case ScanDuration::MS_1000:
      data = Commands::ScanDuration::MS_1000;
      break;
    case ScanDuration::MS_3000:
      data = Commands::ScanDuration::MS_3000;
      break;
    case ScanDuration::MS_5000:
      data = Commands::ScanDuration::MS_5000;
      break;
    case ScanDuration::MS_10000:
      data = Commands::ScanDuration::MS_10000;
      break;
    case ScanDuration::MS_15000:
      data = Commands::ScanDuration::MS_15000;
      break;
    case ScanDuration::MS_20000:
      data = Commands::ScanDuration::MS_20000;
      break;
    case ScanDuration::UNLIMITED:
      data = Commands::ScanDuration::UNLIMITED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid scan duration: %d", static_cast<uint8_t>(duration));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::ScanDuration::SIZE, scan_duration_to_string(duration),
                                   [duration](BarcodeScanner *s) { s->set_scan_duration_state(duration); });
}

std::unique_ptr<Command> CommandFactory::create_stable_induction_time_command(StableInductionTime time) {
  const uint8_t *data = nullptr;
  switch (time) {
    case StableInductionTime::MS_0:
      data = Commands::StableInductionTime::MS_0;
      break;
    case StableInductionTime::MS_100:
      data = Commands::StableInductionTime::MS_100;
      break;
    case StableInductionTime::MS_300:
      data = Commands::StableInductionTime::MS_300;
      break;
    case StableInductionTime::MS_500:
      data = Commands::StableInductionTime::MS_500;
      break;
    case StableInductionTime::MS_1000:
      data = Commands::StableInductionTime::MS_1000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid stable induction time: %d", static_cast<uint8_t>(time));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::StableInductionTime::SIZE, stable_induction_time_to_string(time),
                                   [time](BarcodeScanner *s) { s->set_stable_induction_time_state(time); });
}

std::unique_ptr<Command> CommandFactory::create_reading_interval_command(ReadingInterval interval) {
  const uint8_t *data = nullptr;
  switch (interval) {
    case ReadingInterval::MS_0:
      data = Commands::ReadingInterval::MS_0;
      break;
    case ReadingInterval::MS_100:
      data = Commands::ReadingInterval::MS_100;
      break;
    case ReadingInterval::MS_300:
      data = Commands::ReadingInterval::MS_300;
      break;
    case ReadingInterval::MS_500:
      data = Commands::ReadingInterval::MS_500;
      break;
    case ReadingInterval::MS_1000:
      data = Commands::ReadingInterval::MS_1000;
      break;
    case ReadingInterval::MS_1500:
      data = Commands::ReadingInterval::MS_1500;
      break;
    case ReadingInterval::MS_2000:
      data = Commands::ReadingInterval::MS_2000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid reading interval: %d", static_cast<uint8_t>(interval));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::ReadingInterval::SIZE, reading_interval_to_string(interval),
                                   [interval](BarcodeScanner *s) { s->set_reading_interval_state(interval); });
}

std::unique_ptr<Command> CommandFactory::create_same_code_interval_command(SameCodeInterval interval) {
  const uint8_t *data = nullptr;
  switch (interval) {
    case SameCodeInterval::MS_0:
      data = Commands::SameCodeInterval::MS_0;
      break;
    case SameCodeInterval::MS_100:
      data = Commands::SameCodeInterval::MS_100;
      break;
    case SameCodeInterval::MS_300:
      data = Commands::SameCodeInterval::MS_300;
      break;
    case SameCodeInterval::MS_500:
      data = Commands::SameCodeInterval::MS_500;
      break;
    case SameCodeInterval::MS_1000:
      data = Commands::SameCodeInterval::MS_1000;
      break;
    case SameCodeInterval::MS_1500:
      data = Commands::SameCodeInterval::MS_1500;
      break;
    case SameCodeInterval::MS_2000:
      data = Commands::SameCodeInterval::MS_2000;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid same code interval: %d", static_cast<uint8_t>(interval));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::SameCodeInterval::SIZE, same_code_interval_to_string(interval),
                                   [interval](BarcodeScanner *s) { s->set_same_code_interval_state(interval); });
}

std::unique_ptr<Command> CommandFactory::create_cmd_ack_sound_command(CmdAckSoundMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case CmdAckSoundMode::CMD_ACK_SOUND_ENABLED:
      data = Commands::CmdAckSound::CMD_ACK_SOUND_ENABLED;
      break;
    case CmdAckSoundMode::CMD_ACK_SOUND_DISABLED:
      data = Commands::CmdAckSound::CMD_ACK_SOUND_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid cmd ack sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::CmdAckSound::SIZE, cmd_ack_sound_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_cmd_ack_sound_mode_state(mode); });
}

std::unique_ptr<Command> CommandFactory::create_config_code_scan_command(ConfigCodeScanMode mode) {
  const uint8_t *data = nullptr;
  switch (mode) {
    case ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED:
      data = Commands::ConfigCodeScan::CONFIG_CODE_SCAN_ENABLED;
      break;
    case ConfigCodeScanMode::CONFIG_CODE_SCAN_DISABLED:
      data = Commands::ConfigCodeScan::CONFIG_CODE_SCAN_DISABLED;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid config code scan mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  return std::make_unique<Command>(data, Commands::ConfigCodeScan::SIZE, config_code_scan_mode_to_string(mode),
                                   [mode](BarcodeScanner *s) { s->set_config_code_scan_mode_state(mode); });
}

}  // namespace m5stack_barcode
}  // namespace esphome
