#include "Remote.hpp"

#include "Samp.hpp"

#include <CStreaming.h>
#include <sampapi/0.3.7-R1/CRemotePlayer.h>

namespace {

constexpr auto kUpdateInterval = std::chrono::milliseconds{100};
constexpr auto kRetryInterval = std::chrono::milliseconds{300};
constexpr auto kAttachmentSettleTime = std::chrono::milliseconds{500};

} 

void Remote::Initialize() {
  enabled_ = false;
  selectedModel_ = pedildo::kPresets.size() - 1;
  dirty_ = true;
  lastUpdate_ = {};
  states_.clear();
  suppressed_.clear();

  for (const auto& preset : pedildo::kPresets) {
    CStreaming::RequestModel(preset.model, GAME_REQUIRED);
  }
  CStreaming::LoadAllRequestedModels(false);
  modelsRequested_ = true;
}

void Remote::Shutdown() {
  Disable();

  if (!modelsRequested_) {
    return;
  }

  for (const auto& preset : pedildo::kPresets) {
    CStreaming::SetModelIsDeletable(preset.model);
  }
  modelsRequested_ = false;
}

bool Remote::SetModel(unsigned int number) {
  if (number == 0 || number > pedildo::kPresets.size()) {
    return false;
  }

  selectedModel_ = number - 1;
  dirty_ = true;
  return true;
}

void Remote::SetEnabled(bool enabled) {
  if (enabled) {
    enabled_ = true;
    dirty_ = true;
  } else {
    Disable();
  }
}

bool Remote::ShouldSuppress(const CPed* ped) const {
  return enabled_ && ped && suppressed_.find(ped) != suppressed_.end();
}

void Remote::Process() {
  if (!enabled_) {
    return;
  }

  const auto now = Clock::now();
  if (lastUpdate_ != Clock::time_point{} &&
      now - lastUpdate_ < kUpdateInterval) {
    return;
  }
  lastUpdate_ = now;

  auto* playerPool = pedildo::GetPlayerPool();
  if (!playerPool) {
    states_.clear();
    suppressed_.clear();
    return;
  }

  std::unordered_set<int> visiblePlayers;
  std::unordered_set<const CPed*> nextSuppressed;

  for (int id = 0; id < pedildo::SampPlayerPool::MAX_PLAYERS; ++id) {
    auto* remote = playerPool->GetPlayer(id);
    if (!remote || !remote->m_pPed || !remote->m_pPed->m_pGamePed) {
      continue;
    }

    visiblePlayers.insert(id);
    auto& state = states_[id];

    if (state.ped != remote->m_pPed ||
        state.gamePed != remote->m_pPed->m_pGamePed) {
      state = {};
      state.ped = remote->m_pPed;
      state.gamePed = remote->m_pPed->m_pGamePed;
      state.original = pedildo::CaptureAccessories(*state.ped);
      state.model = selectedModel_;
      state.playerState = remote->m_nState;
    }

    const int playerState = remote->m_nState;
    const bool playerStateChanged = state.playerState != playerState;
    if (playerStateChanged) {
      state.playerState = playerState;
      state.applied = false;
      state.lastApply = {};
    }

    const auto& preset = pedildo::kPresets[selectedModel_];
    const bool needsApply =
        !state.applied || state.model != selectedModel_ || dirty_ ||
        playerStateChanged || !pedildo::PresetMatches(*state.ped, preset);

    if (needsApply &&
        (state.lastApply == Clock::time_point{} ||
         now - state.lastApply >= kRetryInterval)) {
      pedildo::ApplyPreset(*state.ped, preset);
      state.model = selectedModel_;
      state.applied = true;
      state.appliedAt = now;
      state.lastApply = now;
    }

    if (state.applied && now - state.appliedAt >= kAttachmentSettleTime &&
        pedildo::PresetReady(*state.ped,
                             pedildo::kPresets[state.model])) {
      nextSuppressed.insert(state.gamePed);
    }
  }

  for (auto iterator = states_.begin(); iterator != states_.end();) {
    if (visiblePlayers.find(iterator->first) == visiblePlayers.end()) {
      iterator = states_.erase(iterator);
    } else {
      ++iterator;
    }
  }

  suppressed_.swap(nextSuppressed);
  dirty_ = false;
}

void Remote::Disable() {
  if (!states_.empty()) {
    RestoreTrackedAccessories();
  }

  enabled_ = false;
  dirty_ = true;
  states_.clear();
  suppressed_.clear();
}

void Remote::RestoreTrackedAccessories() {
  auto* playerPool = pedildo::GetPlayerPool();
  if (!playerPool) {
    return;
  }

  for (auto& [id, state] : states_) {
    auto* remote = playerPool->GetPlayer(id);
    if (!remote || remote->m_pPed != state.ped ||
        remote->m_pPed->m_pGamePed != state.gamePed) {
      continue;
    }

    pedildo::RestoreAccessories(*state.ped, state.original);
  }
}
