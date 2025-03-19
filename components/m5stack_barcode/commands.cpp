#include "commands.h"

namespace esphome {
namespace m5stack_barcode {

// Basic scanner control commands
constexpr uint8_t Commands::WAKEUP[];
constexpr uint8_t Commands::START_SCAN[];
constexpr uint8_t Commands::STOP_SCAN[];
constexpr uint8_t Commands::GET_VERSION[];

// Operation mode setting commands
constexpr uint8_t Commands::Mode::HOST[];
constexpr uint8_t Commands::Mode::LEVEL[];
constexpr uint8_t Commands::Mode::PULSE[];
constexpr uint8_t Commands::Mode::CONTINUOUS[];
constexpr uint8_t Commands::Mode::AUTO_SENSE[];

// Terminator setting commands
constexpr uint8_t Commands::Terminator::NONE[];
constexpr uint8_t Commands::Terminator::CRLF[];
constexpr uint8_t Commands::Terminator::CR[];
constexpr uint8_t Commands::Terminator::TAB[];
constexpr uint8_t Commands::Terminator::CRCR[];
constexpr uint8_t Commands::Terminator::CRLFCRLF[];

// Lighting control commands
constexpr uint8_t Commands::Light::ON_WHEN_READING[];
constexpr uint8_t Commands::Light::ALWAYS_ON[];
constexpr uint8_t Commands::Light::ALWAYS_OFF[];

// Locate lighting control commands
constexpr uint8_t Commands::LocateLight::ON_WHEN_READING[];
constexpr uint8_t Commands::LocateLight::ALWAYS_ON[];
constexpr uint8_t Commands::LocateLight::ALWAYS_OFF[];

// Sound control commands
constexpr uint8_t Commands::Sound::SOUND_DISABLED[];
constexpr uint8_t Commands::Sound::ENABLED[];

// Volume settings
constexpr uint8_t Commands::Volume::VOLUME_HIGH[];
constexpr uint8_t Commands::Volume::MEDIUM[];
constexpr uint8_t Commands::Volume::VOLUME_LOW[];

// Response codes
constexpr uint8_t Commands::Responses::ACK[];
constexpr uint8_t Commands::Responses::NON_HOST_ACK[];

}  // namespace m5stack_barcode
}  // namespace esphome 