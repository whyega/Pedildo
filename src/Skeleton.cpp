#include "Skeleton.hpp"

#include "Samp.hpp"

#include <CPed.h>

Skeleton::Skeleton() = default;

Skeleton::~Skeleton() {
  Shutdown();
}

bool Skeleton::Initialize() {
  initialized_ = true;
  dirty_ = true;
  return true;
}

void Skeleton::Shutdown() {
  attachedPed_ = nullptr;
  initialized_ = false;
  dirty_ = true;
}

bool Skeleton::SelectModel(unsigned int number) {
  if (number == 0 || number > pedildo::kPresets.size()) {
    return false;
  }

  selectedModel_ = number - 1;
  dirty_ = true;
  return true;
}

bool Skeleton::Render(CPed& ped) {
  if (!initialized_) {
    return false;
  }

  auto* sampPed = pedildo::GetLocalPed(ped);
  if (!sampPed) {
    return false;
  }

  if (attachedPed_ != sampPed) {
    attachedPed_ = sampPed;
    dirty_ = true;
  }

  const auto& preset = pedildo::kPresets[selectedModel_];
  if (dirty_ || !pedildo::PresetMatches(*sampPed, preset)) {
    pedildo::ApplyPreset(*sampPed, preset);
    dirty_ = false;
  }

  return pedildo::PresetReady(*sampPed, preset);
}
