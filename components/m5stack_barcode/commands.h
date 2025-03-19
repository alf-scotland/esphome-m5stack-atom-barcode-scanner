#ifndef USERS_SCOTLAND_WORKSPACE_HA_ESPHOME_M5STACK_ATOM_BARCODE_SCANNER_COMPONENTS_M5STACK_BARCODE_COMMANDS_H
#define USERS_SCOTLAND_WORKSPACE_HA_ESPHOME_M5STACK_ATOM_BARCODE_SCANNER_COMPONENTS_M5STACK_BARCODE_COMMANDS_H

#pragma once

#include <cstddef>  // For size_t
#include <cstdint>  // For uint8_t, etc.

namespace esphome::m5stack_barcode {

/**
 * All command codes for the barcode scanner.
 * These are structured as static class members to allow
 * clear organization by command type.
 */
class Commands {
public:
  /**
   * Basic scanner control commands.
   * These commands are used to control the scanning operation.
   */
  static constexpr uint8_t WAKEUP[] = {0x00};  // Wake-up command (send before any other command)
  static constexpr uint8_t START_SCAN[] = {0x04, 0xE4, 0x04, 0x00, 0xFF, 0x14};
  static constexpr uint8_t STOP_SCAN[] = {0x04, 0xE5, 0x04, 0x00, 0xFF, 0x13};
  static constexpr uint8_t GET_VERSION[] = {0x04, 0xA3, 0x04, 0x00, 0xFF, 0x55};

  // Add size constants for better safety
  static constexpr size_t WAKEUP_SIZE = 1;
  static constexpr size_t START_SCAN_SIZE = 6;
  static constexpr size_t STOP_SCAN_SIZE = 6;
  static constexpr size_t GET_VERSION_SIZE = 6;

  /**
   * Operation mode setting commands.
   * These define different scanning behavior modes.
   */
  struct Mode {
    static constexpr uint8_t HOST[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x08, 0xFE, 0x95};
    static constexpr uint8_t LEVEL[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x00, 0xFE, 0x9D};
    static constexpr uint8_t PULSE[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x02, 0xFE, 0x9B};
    static constexpr uint8_t CONTINUOUS[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x04, 0xFE, 0x99};
    static constexpr uint8_t AUTO_SENSE[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x09, 0xFE, 0x94};

    // Add size constants for better safety
    static constexpr size_t MODE_CMD_SIZE = 9;
    // Add specific size constants for each mode
    static constexpr size_t HOST_SIZE = 9;
    static constexpr size_t LEVEL_SIZE = 9;
    static constexpr size_t PULSE_SIZE = 9;
    static constexpr size_t CONTINUOUS_SIZE = 9;
    static constexpr size_t AUTO_SENSE_SIZE = 9;
  };

  /**
   * Terminator setting commands.
   * These define what characters are appended after a barcode is read.
   */
  struct Terminator {
    static constexpr uint8_t NONE[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0x05, 0x00, 0xFE, 0x2F};
    static constexpr uint8_t CRLF[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0x05, 0x01, 0xFE, 0x2E};
    static constexpr uint8_t CR[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0x05, 0x02, 0xFE, 0x2D};
    static constexpr uint8_t TAB[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0x05, 0x03, 0xFE, 0x2C};
    static constexpr uint8_t CRCR[] = {0x08, 0xC6, 0x04, 0x08, 0x00, 0xF2, 0x05, 0x04, 0xFE, 0x2B};
    static constexpr uint8_t CRLFCRLF[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                           0xF2, 0x05, 0x05, 0xFE, 0x2A};

    // Add size constants for better safety
    static constexpr size_t TERMINATOR_CMD_SIZE = 10;
    // Add specific size constants for each terminator
    static constexpr size_t NONE_SIZE = 10;
    static constexpr size_t CRLF_SIZE = 10;
    static constexpr size_t CR_SIZE = 10;
    static constexpr size_t TAB_SIZE = 10;
    static constexpr size_t CRCR_SIZE = 10;
    static constexpr size_t CRLFCRLF_SIZE = 10;
  };

  /**
   * Lighting control commands.
   * These control the main illumination light.
   */
  struct Light {
    static constexpr uint8_t ON_WHEN_READING[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                                  0xF2, 0x02, 0x00, 0xFE, 0x32};
    static constexpr uint8_t ALWAYS_ON[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                            0xF2, 0x02, 0x01, 0xFE, 0x31};
    static constexpr uint8_t ALWAYS_OFF[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                             0xF2, 0x02, 0x02, 0xFE, 0x30};

    // Add size constants for better safety
    static constexpr size_t LIGHT_CMD_SIZE = 10;
    // Add specific size constants for each light mode
    static constexpr size_t ON_WHEN_READING_SIZE = 10;
    static constexpr size_t ALWAYS_ON_SIZE = 10;
    static constexpr size_t ALWAYS_OFF_SIZE = 10;
  };

  /**
   * Locate lighting control commands.
   * These control the aiming light (red dot or pattern).
   */
  struct LocateLight {
    static constexpr uint8_t ON_WHEN_READING[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                                  0xF2, 0x03, 0x00, 0xFE, 0x31};
    static constexpr uint8_t ALWAYS_ON[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                            0xF2, 0x03, 0x01, 0xFE, 0x30};
    static constexpr uint8_t ALWAYS_OFF[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                             0xF2, 0x03, 0x02, 0xFE, 0x2F};

    // Add size constants for better safety
    static constexpr size_t LOCATE_LIGHT_CMD_SIZE = 10;
    // Add specific size constants for each locate light mode
    static constexpr size_t ON_WHEN_READING_SIZE = 10;
    static constexpr size_t ALWAYS_ON_SIZE = 10;
    static constexpr size_t ALWAYS_OFF_SIZE = 10;
  };

  /**
   * Sound control commands.
   * These control whether the scanner beeps when reading.
   */
  struct Sound {
    static constexpr uint8_t SOUND_DISABLED[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                                 0xF2, 0x0C, 0x00, 0xFE, 0x28};
    static constexpr uint8_t ENABLED[] = {0x08, 0xC6, 0x04, 0x08, 0x00,
                                          0xF2, 0x0C, 0x01, 0xFE, 0x27};

    // Add size constants for better safety
    static constexpr size_t SOUND_CMD_SIZE = 10;
    // Add specific size constants for each sound mode
    static constexpr size_t SOUND_DISABLED_SIZE = 10;
    static constexpr size_t ENABLED_SIZE = 10;
  };

  /**
   * Volume settings for the buzzer.
   * These control how loud the beep is.
   */
  struct Volume {
    // Renamed from HIGH to avoid conflicts with Arduino
    static constexpr uint8_t VOLUME_HIGH[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8C, 0x00, 0xFE, 0x9B};
    static constexpr uint8_t MEDIUM[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8C, 0x01, 0xFE, 0x9A};
    // Renamed from LOW to avoid conflicts with Arduino
    static constexpr uint8_t VOLUME_LOW[] = {0x07, 0xC6, 0x04, 0x08, 0x00, 0x8C, 0x02, 0xFE, 0x99};

    // Add size constants for better safety
    static constexpr size_t VOLUME_CMD_SIZE = 9;
    // Add specific size constants for each volume level
    static constexpr size_t VOLUME_HIGH_SIZE = 9;
    static constexpr size_t MEDIUM_SIZE = 9;
    static constexpr size_t VOLUME_LOW_SIZE = 9;
  };

  /**
   * Response codes from the scanner.
   * These are used to acknowledge commands or indicate status.
   */
  struct Responses {
    static constexpr uint8_t ACK[] = {0x04, 0xD0, 0x00, 0x00, 0xFF, 0x2C};
    // Non-host mode acknowledgment has a different format
    static constexpr uint8_t NON_HOST_ACK[] = {0x05, 0xD1, 0x00, 0x00, 0x06, 0xFF, 0x24};

    // Add size constants for better safety
    static constexpr size_t ACK_SIZE = 6;
    static constexpr size_t NON_HOST_ACK_SIZE = 7;
  };
};

}  // namespace esphome::m5stack_barcode

#endif
