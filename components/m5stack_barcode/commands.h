#pragma once

// UART protocol of the M5Stack Atom QR code scanner, from ATOM_QRCODE_CMD_EN.pdf (docs/).
//
// Every command is a frame: a length byte L, L - 1 payload bytes and a big-endian 16-bit
// checksum (the two's complement of the sum of the preceding bytes), L + 2 bytes in total.
// Setting commands are "08 C6 04 08 00 ..." / "07 C6 04 08 00 ...": opcode C6 writes a
// parameter, and the scanner keeps it across power cycles.

#include <cstddef>
#include <cstdint>
#include <string>

namespace esphome {
namespace m5stack_barcode {

/// Frame length (including the checksum) given its first byte.
constexpr size_t frame_length(const uint8_t *frame) { return frame[0] + 2; }

/// Whether a frame of `size` bytes has a consistent length byte and checksum.
constexpr bool is_valid_frame(const uint8_t *frame, size_t size) {
  if (size < 3 || frame_length(frame) != size)
    return false;
  uint16_t sum = 0;
  for (size_t i = 0; i + 2 < size; i++)
    sum += frame[i];
  return static_cast<uint16_t>(-sum) == ((frame[size - 2] << 8) | frame[size - 1]);
}
template<size_t N> constexpr bool is_valid_frame(const uint8_t (&frame)[N]) { return is_valid_frame(frame, N); }

namespace frames {

/// Sent before every command: wakes the scanner from sleep (not a frame).
inline constexpr uint8_t WAKEUP = 0x00;
inline constexpr uint8_t START_SCAN[] = {0x04, 0xE4, 0x04, 0x00, 0xFF, 0x14};
inline constexpr uint8_t STOP_SCAN[] = {0x04, 0xE5, 0x04, 0x00, 0xFF, 0x13};
/// Answered by an unframed product information string, not by an ACK.
inline constexpr uint8_t GET_VERSION[] = {0x04, 0xA3, 0x04, 0x00, 0xFF, 0x55};
/// PDF item 1, "Set default parameters".
inline constexpr uint8_t FACTORY_RESET[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0xFF, 0x00, 0xFD, 0x35};

/// Reply to every setting command and to start/stop decoding in host mode.
inline constexpr uint8_t ACK[] = {0x04, 0xD0, 0x00, 0x00, 0xFF, 0x2C};
/// A rejected command is answered with 05 D1 00 00 <cause> <checksum>, e.g. start/stop
/// decoding outside host mode (PDF item 3).  Barcode output itself is unframed.
inline constexpr uint8_t NAK_PREFIX[] = {0x05, 0xD1, 0x00, 0x00};
inline constexpr size_t NAK_LENGTH = 7;
inline constexpr size_t NAK_CAUSE_INDEX = 4;

static_assert(is_valid_frame(START_SCAN) && is_valid_frame(STOP_SCAN) && is_valid_frame(GET_VERSION) &&
                  is_valid_frame(FACTORY_RESET) && is_valid_frame(ACK),
              "bad frame");

}  // namespace frames

/// The scanner settings.  The order is persisted to NVS (ScannerPreferences): append only.
enum class SettingId : uint8_t {
  OPERATION_MODE,
  TERMINATOR,
  LIGHT_MODE,
  LOCATE_LIGHT_MODE,
  SOUND_MODE,
  BUZZER_VOLUME,
  DECODING_SUCCESS_LIGHT_MODE,
  BOOT_SOUND_MODE,
  DECODE_SOUND_MODE,
  SCAN_DURATION,
  STABLE_INDUCTION_TIME,
  READING_INTERVAL,
  SAME_CODE_INTERVAL,
  CMD_ACK_SOUND_MODE,
  CONFIG_CODE_SCAN_MODE,
};
inline constexpr size_t NUM_SETTINGS = 15;

/// Values of the operation_mode setting (the order of its option keys).
enum class OperationMode : uint8_t {
  HOST,        // Scans on start/stop decoding commands
  LEVEL,       // Scans while the trigger input is held
  PULSE,       // Scans once per trigger pulse
  CONTINUOUS,  // Scans all the time
  AUTO_SENSE,  // Scans when an object is detected
};

/// A setting's values are indices 0 … num_values - 1 into its option keys and command frames.
/// The keys are the YAML / HA select / action values and must match SETTINGS in __init__.py
/// (tests/test_setting_tables.py checks this).  Two-state settings use "disabled", "enabled".
struct SettingInfo {
  const char *key;                  ///< YAML key, also used in logs
  const char *const *values;        ///< option key per value
  const uint8_t *frames;            ///< num_values frames of frame_size bytes, one per value
  const char *const *config_codes;  ///< configuration barcode per value (after the prefix), or nullptr
  uint8_t num_values;
  uint8_t frame_size;

  const uint8_t *frame(uint8_t value) const { return this->frames + value * this->frame_size; }
  /// Look up an option key; returns false if `key` is not an option of this setting.
  bool parse(const std::string &key, uint8_t &value) const;
};

const SettingInfo &get_setting_info(SettingId id);

/// Configuration barcodes (AtomicQR_Reader_EN.pdf) are QR codes "^#SC^<code>".  With the
/// scanner's config_code_scan_mode enabled it applies them itself, without telling the host;
/// with it disabled it outputs them like any barcode.
inline constexpr const char *CONFIG_CODE_PREFIX = "^#SC^";
/// The configuration barcode that restores the scanner's factory defaults (PDF section 1.1).
inline constexpr const char *FACTORY_RESET_CONFIG_CODE = "303FFF0";
/// Find the setting and value a configuration barcode (without the prefix) sets.
bool find_config_code(const std::string &code, SettingId &id, uint8_t &value);

/// Bytes the scanner appends to each barcode for a terminator value ("" for none).
const char *terminator_bytes(uint8_t terminator);
/// Scan duration in milliseconds for a scan_duration value; 0 for unlimited.
uint32_t scan_duration_ms(uint8_t scan_duration);

}  // namespace m5stack_barcode
}  // namespace esphome
