#pragma once

#include "Presets.hpp"

#include <chrono>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>

class CPed;

class Remote final {
 public:
  void Initialize();
  void Shutdown();
  void Process();

  bool SetModel(unsigned int number);
  void SetEnabled(bool enabled);
  bool ShouldSuppress(const CPed* ped) const;

 private:
  using Clock = std::chrono::steady_clock;

  struct State {
    pedildo::SampPed* ped{};
    CPed* gamePed{};
    pedildo::AccessoryBackups original{};
    std::size_t model{};
    int playerState{-1};
    bool applied{};
    Clock::time_point appliedAt{};
    Clock::time_point lastApply{};
  };

  void Disable();
  void RestoreTrackedAccessories();

  bool enabled_{};
  bool dirty_{true};
  bool modelsRequested_{};
  std::size_t selectedModel_{pedildo::kPresets.size() - 1};
  Clock::time_point lastUpdate_{};
  std::unordered_map<int, State> states_;
  std::unordered_set<const CPed*> suppressed_;
};
