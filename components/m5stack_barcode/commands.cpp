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
constexpr uint8_t Commands::Sound::SOUND_ENABLED[];

// Volume settings
constexpr uint8_t Commands::Volume::VOLUME_HIGH[];
constexpr uint8_t Commands::Volume::VOLUME_MEDIUM[];
constexpr uint8_t Commands::Volume::VOLUME_LOW[];

// Decoding Success Light control commands
constexpr uint8_t Commands::DecodingSuccessLight::LIGHT_ENABLED[];
constexpr uint8_t Commands::DecodingSuccessLight::LIGHT_DISABLED[];

// Boot Sound control commands
constexpr uint8_t Commands::BootSound::BOOT_SOUND_ENABLED[];
constexpr uint8_t Commands::BootSound::BOOT_SOUND_DISABLED[];

// Decode Sound control commands
constexpr uint8_t Commands::DecodeSound::DECODE_SOUND_ENABLED[];
constexpr uint8_t Commands::DecodeSound::DECODE_SOUND_DISABLED[];

// Scan Duration commands
constexpr uint8_t Commands::ScanDuration::MS_500[];
constexpr uint8_t Commands::ScanDuration::MS_1000[];
constexpr uint8_t Commands::ScanDuration::MS_3000[];
constexpr uint8_t Commands::ScanDuration::MS_5000[];
constexpr uint8_t Commands::ScanDuration::MS_10000[];
constexpr uint8_t Commands::ScanDuration::MS_15000[];
constexpr uint8_t Commands::ScanDuration::MS_20000[];
constexpr uint8_t Commands::ScanDuration::UNLIMITED[];

// Stable Induction Time commands
constexpr uint8_t Commands::StableInductionTime::MS_0[];
constexpr uint8_t Commands::StableInductionTime::MS_100[];
constexpr uint8_t Commands::StableInductionTime::MS_300[];
constexpr uint8_t Commands::StableInductionTime::MS_500[];
constexpr uint8_t Commands::StableInductionTime::MS_1000[];

// Reading Interval commands
constexpr uint8_t Commands::ReadingInterval::MS_0[];
constexpr uint8_t Commands::ReadingInterval::MS_100[];
constexpr uint8_t Commands::ReadingInterval::MS_300[];
constexpr uint8_t Commands::ReadingInterval::MS_500[];
constexpr uint8_t Commands::ReadingInterval::MS_1000[];
constexpr uint8_t Commands::ReadingInterval::MS_1500[];
constexpr uint8_t Commands::ReadingInterval::MS_2000[];

// Same Code Interval commands
constexpr uint8_t Commands::SameCodeInterval::MS_0[];
constexpr uint8_t Commands::SameCodeInterval::MS_100[];
constexpr uint8_t Commands::SameCodeInterval::MS_300[];
constexpr uint8_t Commands::SameCodeInterval::MS_500[];
constexpr uint8_t Commands::SameCodeInterval::MS_1000[];
constexpr uint8_t Commands::SameCodeInterval::MS_1500[];
constexpr uint8_t Commands::SameCodeInterval::MS_2000[];

// Command acknowledgment sound commands
constexpr uint8_t Commands::CmdAckSound::CMD_ACK_SOUND_ENABLED[];
constexpr uint8_t Commands::CmdAckSound::CMD_ACK_SOUND_DISABLED[];

// Configuration code scan permission commands
constexpr uint8_t Commands::ConfigCodeScan::CONFIG_CODE_SCAN_ENABLED[];
constexpr uint8_t Commands::ConfigCodeScan::CONFIG_CODE_SCAN_DISABLED[];

// Factory reset command
constexpr uint8_t Commands::FactoryReset::FACTORY_RESET[];

// Response codes
constexpr uint8_t Commands::Responses::ACK[];
constexpr uint8_t Commands::Responses::NON_HOST_ACK[];

}  // namespace m5stack_barcode
}  // namespace esphome
