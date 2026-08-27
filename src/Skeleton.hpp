#pragma once

#include "Presets.hpp"

#include <cstddef>

class CPed;

class Skeleton final {
 public:
  Skeleton();
  ~Skeleton();

  Skeleton(const Skeleton&) = delete;
  Skeleton& operator=(const Skeleton&) = delete;

  bool Initialize();
  void Shutdown();
  bool SelectModel(unsigned int number);
  bool Render(CPed& ped);

 private:
  std::size_t selectedModel_{};
  pedildo::SampPed* attachedPed_{};
  bool initialized_{};
  bool dirty_{true};
};
