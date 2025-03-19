#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

class BarcodeScanner;

// Declare logging tag in UPPER_SNAKE_CASE for constants
extern const char* const TAG_ACTION;

// Basic control actions
template <typename... Ts>
class StartAction : public Action<Ts...> {
public:
  explicit StartAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

template <typename... Ts>
class StopAction : public Action<Ts...> {
public:
  explicit StopAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

// Setting actions
template <typename... Ts>
class SetModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  TEMPLATABLE_VALUE(std::string, mode)

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

template <typename... Ts>
class SetTerminatorAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetTerminatorAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetTerminatorAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  TEMPLATABLE_VALUE(std::string, terminator)

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

// Light control actions
template <typename... Ts>
class SetLightModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetLightModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetLightModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  TEMPLATABLE_VALUE(std::string, mode)

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

template <typename... Ts>
class SetLocateLightModeAction : public Action<Ts...> {
public:
  explicit SetLocateLightModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

// Sound control actions
template <typename... Ts>
class SetSoundModeAction : public Action<Ts...> {
public:
  /**
   * Constructor for SetSoundModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetSoundModeAction(BarcodeScanner* scanner) : scanner_(scanner) {}

  TEMPLATABLE_VALUE(std::string, mode)

  /**
   * Execute the action
   */
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

template <typename... Ts>
class SetBuzzerVolumeAction : public Action<Ts...> {
public:
  explicit SetBuzzerVolumeAction(BarcodeScanner* scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, volume)
  void play(Ts... x) override;

protected:
  BarcodeScanner* scanner_;
};

}  // namespace m5stack_barcode
}  // namespace esphome
