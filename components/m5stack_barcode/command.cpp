#include "command.h"

#include <algorithm>
#include <cstdio>

#include "commands.h"
#include "esphome/core/log.h"
#include "m5stack_barcode.h"

namespace esphome {
namespace m5stack_barcode {

static const char *const TAG_CMD = "m5stack_barcode.cmd";

void Command::log_command_data(const char *tag, const char *prefix) const {
  constexpr size_t MAX_LOG_BYTES = 16;
  char hex_buffer[MAX_LOG_BYTES * 3 + 4] = {0};  // "XX " per byte + "..." + NUL
  const size_t log_len = std::min(this->length_, MAX_LOG_BYTES);
  for (size_t i = 0; i < log_len; i++) {
    snprintf(hex_buffer + (i * 3), 4, "%02X ", this->data_[i]);
  }
  if (log_len < this->length_) {
    snprintf(hex_buffer + (log_len * 3), 4, "...");
  }
  ESP_LOGD(tag, "%s command [%s %s]: %s", prefix, this->name_, this->value_, hex_buffer);
}

// Every setting command table below is indexed by the setting's enum value, so each table
// must list the command arrays in enum declaration order (see types.h).
template<typename E, size_t N>
static std::unique_ptr<Command> make_setting_command(const char *name, const char *value, E setting,
                                                     const uint8_t *const (&table)[N], size_t length,
                                                     void (BarcodeScanner::*apply)(E)) {
  const auto index = static_cast<size_t>(setting);
  if (index >= N) {
    ESP_LOGW(TAG_CMD, "Invalid %s value: %zu", name, index);
    return nullptr;
  }
  return std::make_unique<Command>(
      table[index], length, name, value, [apply, setting](BarcodeScanner *s) { (s->*apply)(setting); }, nullptr,
      ResponseType::NONE, true);
}

// ── Basic commands ────────────────────────────────────────────────────────────

std::unique_ptr<Command> CommandFactory::create_start_command() {
  return std::make_unique<Command>(
      Commands::START_SCAN, Commands::START_SCAN_SIZE, "Start scanning", "",
      [](BarcodeScanner *s) { s->on_scan_started_(); }, [](BarcodeScanner *s) { s->set_scan_state(ScanState::IDLE); });
}

std::unique_ptr<Command> CommandFactory::create_stop_command() {
  return std::make_unique<Command>(Commands::STOP_SCAN, Commands::STOP_SCAN_SIZE, "Stop scanning", "",
                                   [](BarcodeScanner *s) { s->set_scan_state(ScanState::IDLE); });
}

std::unique_ptr<Command> CommandFactory::create_version_command() {
  return std::make_unique<Command>(Commands::GET_VERSION, Commands::GET_VERSION_SIZE, "Get version", "", nullptr,
                                   nullptr, ResponseType::VERSION);
}

std::unique_ptr<Command> CommandFactory::create_factory_reset_command() {
  return std::make_unique<Command>(Commands::FactoryReset::FACTORY_RESET, Commands::FactoryReset::SIZE, "Factory reset",
                                   "", [](BarcodeScanner *s) { s->do_factory_reset_(); });
}

// ── Setting commands ──────────────────────────────────────────────────────────

std::unique_ptr<Command> CommandFactory::create_mode_command(OperationMode mode) {
  static const uint8_t *const TABLE[] = {Commands::Mode::HOST, Commands::Mode::LEVEL, Commands::Mode::PULSE,
                                         Commands::Mode::CONTINUOUS, Commands::Mode::AUTO_SENSE};
  return make_setting_command("Operation mode", operation_mode_to_string(mode), mode, TABLE, Commands::Mode::SIZE,
                              &BarcodeScanner::set_operation_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_terminator_command(Terminator term) {
  static const uint8_t *const TABLE[] = {Commands::Terminator::NONE, Commands::Terminator::CRLF,
                                         Commands::Terminator::CR,   Commands::Terminator::TAB,
                                         Commands::Terminator::CRCR, Commands::Terminator::CRLFCRLF};
  return make_setting_command("Terminator", terminator_to_string(term), term, TABLE, Commands::Terminator::SIZE,
                              &BarcodeScanner::set_terminator_state);
}

std::unique_ptr<Command> CommandFactory::create_light_command(LightMode mode) {
  static const uint8_t *const TABLE[] = {Commands::Light::ON_WHEN_READING, Commands::Light::ALWAYS_ON,
                                         Commands::Light::ALWAYS_OFF};
  return make_setting_command("Light mode", light_mode_to_string(mode), mode, TABLE, Commands::Light::SIZE,
                              &BarcodeScanner::set_light_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_locate_light_command(LocateLightMode mode) {
  static const uint8_t *const TABLE[] = {Commands::LocateLight::ON_WHEN_READING, Commands::LocateLight::ALWAYS_ON,
                                         Commands::LocateLight::ALWAYS_OFF};
  return make_setting_command("Locate light mode", light_mode_to_string(static_cast<LightMode>(mode)), mode, TABLE,
                              Commands::LocateLight::SIZE, &BarcodeScanner::set_locate_light_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_sound_command(SoundMode mode) {
  static const uint8_t *const TABLE[] = {Commands::Sound::SOUND_DISABLED, Commands::Sound::SOUND_ENABLED};
  return make_setting_command("Sound", enabled_to_string(mode == SoundMode::SOUND_ENABLED), mode, TABLE,
                              Commands::Sound::SIZE, &BarcodeScanner::set_sound_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_volume_command(BuzzerVolume volume) {
  static const uint8_t *const TABLE[] = {Commands::Volume::VOLUME_HIGH, Commands::Volume::VOLUME_MEDIUM,
                                         Commands::Volume::VOLUME_LOW};
  return make_setting_command("Buzzer volume", buzzer_volume_to_string(volume), volume, TABLE, Commands::Volume::SIZE,
                              &BarcodeScanner::set_buzzer_volume_state);
}

std::unique_ptr<Command> CommandFactory::create_decoding_success_light_command(DecodingSuccessLightMode mode) {
  static const uint8_t *const TABLE[] = {Commands::DecodingSuccessLight::LIGHT_DISABLED,
                                         Commands::DecodingSuccessLight::LIGHT_ENABLED};
  return make_setting_command(
      "Decoding success light", enabled_to_string(mode == DecodingSuccessLightMode::DECODING_LIGHT_ENABLED), mode,
      TABLE, Commands::DecodingSuccessLight::SIZE, &BarcodeScanner::set_decoding_success_light_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_boot_sound_command(BootSoundMode mode) {
  static const uint8_t *const TABLE[] = {Commands::BootSound::BOOT_SOUND_DISABLED,
                                         Commands::BootSound::BOOT_SOUND_ENABLED};
  return make_setting_command("Boot sound", enabled_to_string(mode == BootSoundMode::BOOT_SOUND_ENABLED), mode, TABLE,
                              Commands::BootSound::SIZE, &BarcodeScanner::set_boot_sound_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_decode_sound_command(DecodeSoundMode mode) {
  static const uint8_t *const TABLE[] = {Commands::DecodeSound::DECODE_SOUND_DISABLED,
                                         Commands::DecodeSound::DECODE_SOUND_ENABLED};
  return make_setting_command("Decode sound", enabled_to_string(mode == DecodeSoundMode::DECODE_SOUND_ENABLED), mode,
                              TABLE, Commands::DecodeSound::SIZE, &BarcodeScanner::set_decode_sound_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_scan_duration_command(ScanDuration duration) {
  static const uint8_t *const TABLE[] = {Commands::ScanDuration::MS_500,   Commands::ScanDuration::MS_1000,
                                         Commands::ScanDuration::MS_3000,  Commands::ScanDuration::MS_5000,
                                         Commands::ScanDuration::MS_10000, Commands::ScanDuration::MS_15000,
                                         Commands::ScanDuration::MS_20000, Commands::ScanDuration::UNLIMITED};
  return make_setting_command("Scan duration", scan_duration_to_string(duration), duration, TABLE,
                              Commands::ScanDuration::SIZE, &BarcodeScanner::set_scan_duration_state);
}

std::unique_ptr<Command> CommandFactory::create_stable_induction_time_command(StableInductionTime time) {
  static const uint8_t *const TABLE[] = {Commands::StableInductionTime::MS_0, Commands::StableInductionTime::MS_100,
                                         Commands::StableInductionTime::MS_300, Commands::StableInductionTime::MS_500,
                                         Commands::StableInductionTime::MS_1000};
  return make_setting_command("Stable induction time", interval_to_string(static_cast<uint8_t>(time)), time, TABLE,
                              Commands::StableInductionTime::SIZE, &BarcodeScanner::set_stable_induction_time_state);
}

std::unique_ptr<Command> CommandFactory::create_reading_interval_command(ReadingInterval interval) {
  static const uint8_t *const TABLE[] = {Commands::ReadingInterval::MS_0,    Commands::ReadingInterval::MS_100,
                                         Commands::ReadingInterval::MS_300,  Commands::ReadingInterval::MS_500,
                                         Commands::ReadingInterval::MS_1000, Commands::ReadingInterval::MS_1500,
                                         Commands::ReadingInterval::MS_2000};
  return make_setting_command("Reading interval", interval_to_string(static_cast<uint8_t>(interval)), interval, TABLE,
                              Commands::ReadingInterval::SIZE, &BarcodeScanner::set_reading_interval_state);
}

std::unique_ptr<Command> CommandFactory::create_same_code_interval_command(SameCodeInterval interval) {
  static const uint8_t *const TABLE[] = {Commands::SameCodeInterval::MS_0,    Commands::SameCodeInterval::MS_100,
                                         Commands::SameCodeInterval::MS_300,  Commands::SameCodeInterval::MS_500,
                                         Commands::SameCodeInterval::MS_1000, Commands::SameCodeInterval::MS_1500,
                                         Commands::SameCodeInterval::MS_2000};
  return make_setting_command("Same code interval", interval_to_string(static_cast<uint8_t>(interval)), interval, TABLE,
                              Commands::SameCodeInterval::SIZE, &BarcodeScanner::set_same_code_interval_state);
}

std::unique_ptr<Command> CommandFactory::create_cmd_ack_sound_command(CmdAckSoundMode mode) {
  static const uint8_t *const TABLE[] = {Commands::CmdAckSound::CMD_ACK_SOUND_DISABLED,
                                         Commands::CmdAckSound::CMD_ACK_SOUND_ENABLED};
  return make_setting_command("Command ACK sound", enabled_to_string(mode == CmdAckSoundMode::CMD_ACK_SOUND_ENABLED),
                              mode, TABLE, Commands::CmdAckSound::SIZE, &BarcodeScanner::set_cmd_ack_sound_mode_state);
}

std::unique_ptr<Command> CommandFactory::create_config_code_scan_command(ConfigCodeScanMode mode) {
  static const uint8_t *const TABLE[] = {Commands::ConfigCodeScan::CONFIG_CODE_SCAN_DISABLED,
                                         Commands::ConfigCodeScan::CONFIG_CODE_SCAN_ENABLED};
  return make_setting_command("Config code scanning",
                              enabled_to_string(mode == ConfigCodeScanMode::CONFIG_CODE_SCAN_ENABLED), mode, TABLE,
                              Commands::ConfigCodeScan::SIZE, &BarcodeScanner::set_config_code_scan_mode_state);
}

}  // namespace m5stack_barcode
}  // namespace esphome
