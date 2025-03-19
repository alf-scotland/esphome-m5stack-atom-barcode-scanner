#include "commands.h"

#include <cstdint>

namespace esphome {
namespace m5stack_barcode {

// Basic scanner control commands
constexpr uint8_t Commands::WAKEUP[];
constexpr uint8_t Commands::START_SCAN[];
constexpr uint8_t Commands::STOP_SCAN[];
constexpr uint8_t Commands::GET_VERSION[];

constexpr size_t Commands::WAKEUP_SIZE;
constexpr size_t Commands::START_SCAN_SIZE;
constexpr size_t Commands::STOP_SCAN_SIZE;
constexpr size_t Commands::GET_VERSION_SIZE;

// Operation mode setting commands
constexpr uint8_t Commands::Mode::HOST[];
constexpr uint8_t Commands::Mode::LEVEL[];
constexpr uint8_t Commands::Mode::PULSE[];
constexpr uint8_t Commands::Mode::CONTINUOUS[];
constexpr uint8_t Commands::Mode::AUTO_SENSE[];

constexpr size_t Commands::Mode::MODE_CMD_SIZE;
constexpr size_t Commands::Mode::HOST_SIZE;
constexpr size_t Commands::Mode::LEVEL_SIZE;
constexpr size_t Commands::Mode::PULSE_SIZE;
constexpr size_t Commands::Mode::CONTINUOUS_SIZE;
constexpr size_t Commands::Mode::AUTO_SENSE_SIZE;

// Terminator setting commands
constexpr uint8_t Commands::Terminator::NONE[];
constexpr uint8_t Commands::Terminator::CRLF[];
constexpr uint8_t Commands::Terminator::CR[];
constexpr uint8_t Commands::Terminator::TAB[];
constexpr uint8_t Commands::Terminator::CRCR[];
constexpr uint8_t Commands::Terminator::CRLFCRLF[];

constexpr size_t Commands::Terminator::TERMINATOR_CMD_SIZE;
constexpr size_t Commands::Terminator::NONE_SIZE;
constexpr size_t Commands::Terminator::CRLF_SIZE;
constexpr size_t Commands::Terminator::CR_SIZE;
constexpr size_t Commands::Terminator::TAB_SIZE;
constexpr size_t Commands::Terminator::CRCR_SIZE;
constexpr size_t Commands::Terminator::CRLFCRLF_SIZE;

// Lighting control commands
constexpr uint8_t Commands::Light::ON_WHEN_READING[];
constexpr uint8_t Commands::Light::ALWAYS_ON[];
constexpr uint8_t Commands::Light::ALWAYS_OFF[];

constexpr size_t Commands::Light::LIGHT_CMD_SIZE;
constexpr size_t Commands::Light::ON_WHEN_READING_SIZE;
constexpr size_t Commands::Light::ALWAYS_ON_SIZE;
constexpr size_t Commands::Light::ALWAYS_OFF_SIZE;

// Locate lighting control commands
constexpr uint8_t Commands::LocateLight::ON_WHEN_READING[];
constexpr uint8_t Commands::LocateLight::ALWAYS_ON[];
constexpr uint8_t Commands::LocateLight::ALWAYS_OFF[];

constexpr size_t Commands::LocateLight::LOCATE_LIGHT_CMD_SIZE;
constexpr size_t Commands::LocateLight::ON_WHEN_READING_SIZE;
constexpr size_t Commands::LocateLight::ALWAYS_ON_SIZE;
constexpr size_t Commands::LocateLight::ALWAYS_OFF_SIZE;

// Sound control commands
constexpr uint8_t Commands::Sound::SOUND_DISABLED[];
constexpr uint8_t Commands::Sound::ENABLED[];

constexpr size_t Commands::Sound::SOUND_CMD_SIZE;
constexpr size_t Commands::Sound::SOUND_DISABLED_SIZE;
constexpr size_t Commands::Sound::ENABLED_SIZE;

// Volume settings
constexpr uint8_t Commands::Volume::VOLUME_HIGH[];
constexpr uint8_t Commands::Volume::MEDIUM[];
constexpr uint8_t Commands::Volume::VOLUME_LOW[];

constexpr size_t Commands::Volume::VOLUME_CMD_SIZE;
constexpr size_t Commands::Volume::VOLUME_HIGH_SIZE;
constexpr size_t Commands::Volume::MEDIUM_SIZE;
constexpr size_t Commands::Volume::VOLUME_LOW_SIZE;

// Response codes
constexpr uint8_t Commands::Responses::ACK[];
constexpr uint8_t Commands::Responses::NON_HOST_ACK[];

constexpr size_t Commands::Responses::ACK_SIZE;
constexpr size_t Commands::Responses::NON_HOST_ACK_SIZE;

}  // namespace m5stack_barcode
}  // namespace esphome
