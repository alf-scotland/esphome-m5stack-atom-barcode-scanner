#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "types.h"

namespace esphome {
namespace m5stack_barcode {

class BarcodeScannerBase;

extern const char *const TAG_ACTION;

// Basic control actions
template<typename... Ts> class StartAction : public Action<Ts...> {
 public:
  explicit StartAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class StopAction : public Action<Ts...> {
 public:
  explicit StopAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Setting actions
template<typename... Ts> class SetModeAction : public Action<Ts...> {
 public:
  explicit SetModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, mode)

  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class SetTerminatorAction : public Action<Ts...> {
 public:
  explicit SetTerminatorAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, terminator)

  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Light control actions
template<typename... Ts> class SetLightModeAction : public Action<Ts...> {
 public:
  explicit SetLightModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, light_mode)

  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class SetLocateLightModeAction : public Action<Ts...> {
 public:
  explicit SetLocateLightModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, locate_light_mode)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Sound control actions
template<typename... Ts> class SetSoundModeAction : public Action<Ts...> {
 public:
  explicit SetSoundModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, sound_mode)

  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class SetBuzzerVolumeAction : public Action<Ts...> {
 public:
  explicit SetBuzzerVolumeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, volume)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Decoding success light actions
template<typename... Ts> class SetDecodingSuccessLightModeAction : public Action<Ts...> {
 public:
  explicit SetDecodingSuccessLightModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, decoding_success_light_mode)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Boot sound actions
template<typename... Ts> class SetBootSoundModeAction : public Action<Ts...> {
 public:
  explicit SetBootSoundModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, boot_sound_mode)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Decode sound actions
template<typename... Ts> class SetDecodeSoundModeAction : public Action<Ts...> {
 public:
  explicit SetDecodeSoundModeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, decode_sound_mode)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Scan duration actions
template<typename... Ts> class SetScanDurationAction : public Action<Ts...> {
 public:
  explicit SetScanDurationAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, duration)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Stable induction time actions
template<typename... Ts> class SetStableInductionTimeAction : public Action<Ts...> {
 public:
  explicit SetStableInductionTimeAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, time)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Reading interval actions
template<typename... Ts> class SetReadingIntervalAction : public Action<Ts...> {
 public:
  explicit SetReadingIntervalAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, interval)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

// Same code interval actions
template<typename... Ts> class SetSameCodeIntervalAction : public Action<Ts...> {
 public:
  explicit SetSameCodeIntervalAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  TEMPLATABLE_VALUE(std::string, interval)
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class ProcessCurrentBufferAction : public Action<Ts...> {
 public:
  explicit ProcessCurrentBufferAction(BarcodeScannerBase *scanner) : scanner_(scanner) {}
  void play(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class IsManualScanningCondition : public Condition<Ts...> {
 public:
  explicit IsManualScanningCondition(BarcodeScannerBase *scanner) : scanner_(scanner) {}

  bool check(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class IsIdleCondition : public Condition<Ts...> {
 public:
  explicit IsIdleCondition(BarcodeScannerBase *scanner) : scanner_(scanner) {}

  bool check(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

template<typename... Ts> class IsContinuousModeCondition : public Condition<Ts...> {
 public:
  explicit IsContinuousModeCondition(BarcodeScannerBase *scanner) : scanner_(scanner) {}

  bool check(const Ts &...x) override;

 protected:
  BarcodeScannerBase *scanner_;
};

}  // namespace m5stack_barcode
}  // namespace esphome
