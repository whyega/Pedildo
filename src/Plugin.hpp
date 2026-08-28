#pragma once

#include "Commands.hpp"
#include "Remote.hpp"
#include "Skeleton.hpp"

#include <kthook/kthook.hpp>
#include <plugintmplt/plugintmplt.hpp>

#include <array>
#include <cstdint>

class CPed;

class Plugin final : public plugintmplt::AbstractPlugin<Plugin> {
 public:
  void OnAttach(void* module) override;
  void OnDetach() override;

 private:
  using RenderPedFunction = void(__thiscall*)(CPed*);
  using RenderPedHook = kthook::kthook_signal<RenderPedFunction>;

  static void __cdecl SelectDildo(const char* arguments);

  Skeleton skeleton_;
  Remote remote_;
  Commands commands_;
  std::array<RenderPedHook, 2> renderPedHooks_;
};
