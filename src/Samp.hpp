#pragma once

#include "Presets.hpp"

#include <sampapi/0.3.7-R1/CNetGame.h>
#include <sampapi/0.3.7-R1/CPlayerPool.h>

class CPed;

namespace pedildo {

using SampPlayerPool = sampapi::v037r1::CPlayerPool;

inline SampPlayerPool* GetPlayerPool() {
  if (sampapi::GetBase() == 0) {
    return nullptr;
  }

  auto* netGame = sampapi::v037r1::RefNetGame();
  return netGame ? netGame->GetPlayerPool() : nullptr;
}

inline SampPed* GetLocalPed(CPed& gamePed) {
  auto* playerPool = GetPlayerPool();
  if (!playerPool) {
    return nullptr;
  }

  auto* localPlayer = playerPool->GetLocalPlayer();
  if (!localPlayer) {
    return nullptr;
  }

  auto* ped = localPlayer->GetPed();
  return ped && ped->m_pGamePed == &gamePed ? ped : nullptr;
}

}  
