#include "command_handlers.h"
#include "commands.h"
#include "types.h"

#include "m5stack_barcode.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5stack_barcode {

static const char *const TAG_CMD = "m5stack_barcode.cmd";

// Custom implementation of make_unique for C++11
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Scanner basic commands
std::unique_ptr<CommandBase> CommandFactory::create_start_command() {
  return make_unique<SimpleCommand>(
    Commands::START_SCAN, 
    Commands::START_SCAN_SIZE,
    "Start scanning",
    [](BarcodeScanner* scanner) {
      scanner->set_scanning(true);
    },
    [](BarcodeScanner* scanner) {
      scanner->set_scanning(false);
    },
    ResponseType::BARCODE
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_stop_command() {
  return make_unique<SimpleCommand>(
    Commands::STOP_SCAN, 
    Commands::STOP_SCAN_SIZE,
    "Stop scanning",
    [](BarcodeScanner* scanner) {
      scanner->set_scanning(false);
    },
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_version_command() {
  return make_unique<SimpleCommand>(
    Commands::GET_VERSION, 
    Commands::GET_VERSION_SIZE,
    "Get version",
    nullptr,
    nullptr,
    ResponseType::VERSION
  );
}

// Setting commands
std::unique_ptr<CommandBase> CommandFactory::create_mode_command(OperationMode mode) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (mode) {
    case OperationMode::HOST:
      cmd_data = Commands::Mode::HOST;
      cmd_size = Commands::Mode::HOST_SIZE;
      break;
    case OperationMode::LEVEL:
      cmd_data = Commands::Mode::LEVEL;
      cmd_size = Commands::Mode::LEVEL_SIZE;
      break;
    case OperationMode::PULSE:
      cmd_data = Commands::Mode::PULSE;
      cmd_size = Commands::Mode::PULSE_SIZE;
      break;
    case OperationMode::CONTINUOUS:
      cmd_data = Commands::Mode::CONTINUOUS;
      cmd_size = Commands::Mode::CONTINUOUS_SIZE;
      break;
    case OperationMode::AUTO_SENSE:
      cmd_data = Commands::Mode::AUTO_SENSE;
      cmd_size = Commands::Mode::AUTO_SENSE_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid operation mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  
  return make_unique<Command<OperationMode>>(
    cmd_data, cmd_size, mode, 
    operation_mode_to_string(mode),
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_terminator_command(Terminator term) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (term) {
    case Terminator::NONE:
      cmd_data = Commands::Terminator::NONE;
      cmd_size = Commands::Terminator::NONE_SIZE;
      break;
    case Terminator::CRLF:
      cmd_data = Commands::Terminator::CRLF;
      cmd_size = Commands::Terminator::CRLF_SIZE;
      break;
    case Terminator::CR:
      cmd_data = Commands::Terminator::CR;
      cmd_size = Commands::Terminator::CR_SIZE;
      break;
    case Terminator::TAB:
      cmd_data = Commands::Terminator::TAB;
      cmd_size = Commands::Terminator::TAB_SIZE;
      break;
    case Terminator::CRCR:
      cmd_data = Commands::Terminator::CRCR;
      cmd_size = Commands::Terminator::CRCR_SIZE;
      break;
    case Terminator::CRLFCRLF:
      cmd_data = Commands::Terminator::CRLFCRLF;
      cmd_size = Commands::Terminator::CRLFCRLF_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid terminator: %d", static_cast<uint8_t>(term));
      return nullptr;
  }
  
  return make_unique<Command<Terminator>>(
    cmd_data, cmd_size, term, 
    terminator_to_string(term),
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_light_command(LightMode mode) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (mode) {
    case LightMode::ON_WHEN_READING:
      cmd_data = Commands::Light::ON_WHEN_READING;
      cmd_size = Commands::Light::ON_WHEN_READING_SIZE;
      break;
    case LightMode::ALWAYS_ON:
      cmd_data = Commands::Light::ALWAYS_ON;
      cmd_size = Commands::Light::ALWAYS_ON_SIZE;
      break;
    case LightMode::ALWAYS_OFF:
      cmd_data = Commands::Light::ALWAYS_OFF;
      cmd_size = Commands::Light::ALWAYS_OFF_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  
  return make_unique<Command<LightMode>>(
    cmd_data, cmd_size, mode, 
    light_mode_to_string(mode),
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_locate_light_command(LocateLightMode mode) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (mode) {
    case LocateLightMode::ON_WHEN_READING:
      cmd_data = Commands::LocateLight::ON_WHEN_READING;
      cmd_size = Commands::LocateLight::ON_WHEN_READING_SIZE;
      break;
    case LocateLightMode::ALWAYS_ON:
      cmd_data = Commands::LocateLight::ALWAYS_ON;
      cmd_size = Commands::LocateLight::ALWAYS_ON_SIZE;
      break;
    case LocateLightMode::ALWAYS_OFF:
      cmd_data = Commands::LocateLight::ALWAYS_OFF;
      cmd_size = Commands::LocateLight::ALWAYS_OFF_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid locate light mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  
  return make_unique<Command<LocateLightMode>>(
    cmd_data, cmd_size, mode, 
    locate_light_mode_to_string(mode),
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_sound_command(SoundMode mode) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (mode) {
    case SoundMode::SOUND_DISABLED:
      cmd_data = Commands::Sound::SOUND_DISABLED;
      cmd_size = Commands::Sound::SOUND_DISABLED_SIZE;
      break;
    case SoundMode::ENABLED:
      cmd_data = Commands::Sound::ENABLED;
      cmd_size = Commands::Sound::ENABLED_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid sound mode: %d", static_cast<uint8_t>(mode));
      return nullptr;
  }
  
  return make_unique<Command<SoundMode>>(
    cmd_data, cmd_size, mode, 
    sound_mode_to_string(mode),
    nullptr
  );
}

std::unique_ptr<CommandBase> CommandFactory::create_volume_command(BuzzerVolume volume) {
  const uint8_t* cmd_data = nullptr;
  size_t cmd_size = 0;
  
  switch (volume) {
    case BuzzerVolume::VOLUME_HIGH:
      cmd_data = Commands::Volume::VOLUME_HIGH;
      cmd_size = Commands::Volume::VOLUME_HIGH_SIZE;
      break;
    case BuzzerVolume::MEDIUM:
      cmd_data = Commands::Volume::MEDIUM;
      cmd_size = Commands::Volume::MEDIUM_SIZE;
      break;
    case BuzzerVolume::VOLUME_LOW:
      cmd_data = Commands::Volume::VOLUME_LOW;
      cmd_size = Commands::Volume::VOLUME_LOW_SIZE;
      break;
    default:
      ESP_LOGW(TAG_CMD, "Invalid buzzer volume: %d", static_cast<uint8_t>(volume));
      return nullptr;
  }
  
  return make_unique<Command<BuzzerVolume>>(
    cmd_data, cmd_size, volume, 
    buzzer_volume_to_string(volume),
    nullptr
  );
}

}  // namespace m5stack_barcode
}  // namespace esphome 