#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

class BarcodeScanner;

// Declare logging tag in UPPER_SNAKE_CASE for constants
extern const char *const TAG_ACTION;

// Basic control actions
template<typename... Ts> class StartAction : public Action<Ts...> {
 public:
  explicit StartAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class StopAction : public Action<Ts...> {
 public:
  explicit StopAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Setting actions
template<typename... Ts> class SetModeAction : public Action<Ts...> {
 public:
  /**
   * Constructor for SetModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)

  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class SetTerminatorAction : public Action<Ts...> {
 public:
  /**
   * Constructor for SetTerminatorAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetTerminatorAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, terminator)

  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Light control actions
template<typename... Ts> class SetLightModeAction : public Action<Ts...> {
 public:
  /**
   * Constructor for SetLightModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetLightModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)

  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class SetLocateLightModeAction : public Action<Ts...> {
 public:
  explicit SetLocateLightModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Sound control actions
template<typename... Ts> class SetSoundModeAction : public Action<Ts...> {
 public:
  /**
   * Constructor for SetSoundModeAction.
   * @param scanner Reference to the barcode scanner instance
   */
  explicit SetSoundModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)

  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class SetBuzzerVolumeAction : public Action<Ts...> {
 public:
  explicit SetBuzzerVolumeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, volume)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// New actions for additional settings

// Decoding success light actions
template<typename... Ts> class SetDecodingSuccessLightModeAction : public Action<Ts...> {
 public:
  explicit SetDecodingSuccessLightModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Boot sound actions
template<typename... Ts> class SetBootSoundModeAction : public Action<Ts...> {
 public:
  explicit SetBootSoundModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Decode sound actions
template<typename... Ts> class SetDecodeSoundModeAction : public Action<Ts...> {
 public:
  explicit SetDecodeSoundModeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Scan duration actions
template<typename... Ts> class SetScanDurationAction : public Action<Ts...> {
 public:
  explicit SetScanDurationAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, duration)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Stable induction time actions
template<typename... Ts> class SetStableInductionTimeAction : public Action<Ts...> {
 public:
  explicit SetStableInductionTimeAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, time)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Reading interval actions
template<typename... Ts> class SetReadingIntervalAction : public Action<Ts...> {
 public:
  explicit SetReadingIntervalAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, interval)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

// Same code interval actions
template<typename... Ts> class SetSameCodeIntervalAction : public Action<Ts...> {
 public:
  explicit SetSameCodeIntervalAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, interval)
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class ProcessCurrentBufferAction : public Action<Ts...> {
 public:
  explicit ProcessCurrentBufferAction(BarcodeScanner *scanner) : scanner_(scanner) {}
  void play(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

template<typename... Ts> class IsContinuousModeCondition : public Condition<Ts...> {
 public:
  explicit IsContinuousModeCondition(BarcodeScanner *scanner) : scanner_(scanner) {}
  bool check(Ts... x) override;

 protected:
  BarcodeScanner *scanner_;
};

}  // namespace m5stack_barcode
}  // namespace esphome
