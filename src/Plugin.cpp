#include "plugin.hpp"

#include <CStreaming.h>
#include <CVisibilityPlugins.h>
#include <extensions/ScriptCommands.h>
#include <plugin.h>

#include <unordered_map>

constexpr auto DILDO_MODEL_ID = 321;
constexpr auto SCALE = 2.f;
constexpr auto DOWN_OFFSET = 0.5f;

std::unordered_map<CEntity*, std::vector<std::pair<std::uint32_t, CObject*>>>
    gPool;

static const std::unordered_map<int, int> gBonesPair = {
    {53, 52}, {51, 52}, {43, 42}, {41, 42}, {51, 2},  {41, 2},
    {3, 2},   {4, 3},   {4, 22},  {4, 32},  {32, 33}, {22, 23},
    {23, 24}, {33, 34}, {4, 8},   {8, 7},   {8, 6},   {6, 7}};

using namespace plugin;

static RpAtomic* ScaleAtomicCallback(RpAtomic* atomic, void* data) {
  RwMatrix* matrix = (RwMatrix*)data;
  RwFrame* frame = (RwFrame*)atomic->object.object
                       .parent;  // Получаем фрейм через RwObjectHasFrame
  if (frame) {
    RwFrameTransform(frame, matrix, rwCOMBINEPRECONCAT);
  }
  return atomic;
}

void RenderEntity(CEntity* entity) {
  auto rwObject = entity->m_pRwObject;
  auto rwClump = entity->m_pRwClump;
  if (!rwObject || !rwClump) return;

  RwV3d scaleVec = {SCALE, SCALE, SCALE};
  RwMatrix scaleMatrix;
  RwMatrixScale(&scaleMatrix, &scaleVec, rwCOMBINEREPLACE);
  RpClumpForAllAtomics(rwClump, ScaleAtomicCallback, &scaleMatrix);

  entity->Add();
  entity->PreRender();
  CVisibilityPlugins::RenderEntity(entity, false, 999.f);
  entity->Remove();
}

void RenderObject(std::vector<std::pair<std::uint32_t, CObject*>> objects) {
  for (auto& objectData : objects) {
    auto [handle, object] = objectData;

    RenderEntity(object);
  }
}

static void __fastcall CEntity__Render(CEntity* entity, void* edx) {
  if (!entity) return;

  auto it = gPool.find(entity);
  if (it == gPool.end()) {
    bool exist = Command<Commands::HAS_MODEL_LOADED>(DILDO_MODEL_ID);
    if (!exist) {
      Command<Commands::REQUEST_MODEL>(DILDO_MODEL_ID);
      Command<Commands::LOAD_ALL_MODELS_NOW>();
      if (!Command<Commands::HAS_MODEL_LOADED>(DILDO_MODEL_ID)) return;
    }

    std::vector<std::pair<std::uint32_t, CObject*>> bones;
    for (auto i = 0; i < gBonesPair.size(); i++) {
      std::uint32_t handle{};
      Command<Commands::CREATE_OBJECT>(DILDO_MODEL_ID, 0.0f, 0.0f, 50.0f,
                                       &handle);
      auto object = CPools::GetObject(handle);
      object->CreateRwObject();
      if (!object) return;
      bones.push_back(std::pair(handle, object));
    }
    gPool.insert_or_assign(entity, bones);
  }

  auto objects = it->second;

  // auto AnimHierarchyFromSkinClump =
  //     GetAnimHierarchyFromSkinClump(entity->m_pRwClump);

  // if (AnimHierarchyFromSkinClump) {
  //   int i = 0;
  //   for (auto&& [startBoneId, endBoneId] : gBonesPair) {
  //     auto id = RpHAnimIDGetIndex(AnimHierarchyFromSkinClump, startBoneId);
  //     auto startBoneMat =
  //         &RpHAnimHierarchyGetMatrixArray(AnimHierarchyFromSkinClump)[id];

  //     objects[i].second->SetMatrix({startBoneMat, true});
  //     // auto start_bone_pos = world_to_screen_pos(
  //     //     misc::get_bone_pos(remote_player.gta_ped(), start_bone_id));
  //     // if (start_bone_pos.z < 1.f) return;

  //     // auto end_bone_pos = misc::world_to_screen_pos(
  //     //     misc::get_bone_pos(remote_player.gta_ped(), end_bone_id));
  //     // if (end_bone_pos.z < 1.f) return;

  //     // draw_list->AddLine({start_bone_pos.x, start_bone_pos.y},
  //     //                    {end_bone_pos.x, end_bone_pos.y}, color);
  //     i++;
  //   }
  // }

  // RenderObject(it->second);
}

void Plugin::OnAttach(void* handle) {
  patch::RedirectCall(0x5E77FC, &CEntity__Render);
  patch::RedirectCall(0x5E780A, &CEntity__Render);

  Events::pedDtorEvent += [](CPed* ped) {
    auto it = gPool.find(ped);
    if (it != gPool.end()) {
      for (auto& objectData : it->second) {
        auto [handle, object] = objectData;
        Command<Commands::DELETE_OBJECT>(handle);
      }
    }
  };
}