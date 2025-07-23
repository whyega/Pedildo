#include "plugin.hpp"

#include <CStreaming.h>
#include <CVisibilityPlugins.h>
#include <extensions/ScriptCommands.h>
#include <plugin.h>

#include <unordered_map>

constexpr auto DILDO_MODEL_ID = 321;
constexpr auto SCALE = 2.f;
constexpr auto DOWN_OFFSET = 0.5f;

std::unordered_map<CEntity*, std::pair<std::uint32_t, CObject*>> gPool;

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

void RenderObject(CEntity* entity) {
  auto [handle, object] = gPool[entity];

  object->CreateRwObject();
  auto pos = entity->GetPosition();
  pos.z -= DOWN_OFFSET;
  object->Teleport(pos, false);
  object->SetHeading(entity->GetHeading());

  RenderEntity(object);
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

    std::uint32_t handle{};
    Command<Commands::CREATE_OBJECT>(DILDO_MODEL_ID, 0.0f, 0.0f, 50.0f,
                                     &handle);
    auto object = CPools::GetObject(handle);
    if (!object) return;
    gPool.insert_or_assign(entity, std::pair(handle, object));
  }

  RenderObject(entity);
}

void Plugin::OnAttach(void* handle) {
  patch::RedirectCall(0x5E77FC, &CEntity__Render);
  patch::RedirectCall(0x5E780A, &CEntity__Render);

  Events::pedDtorEvent += [](CPed* ped) {
    auto it = gPool.find(ped);
    if (it != gPool.end()) {
      auto [handle, object] = it->second;
      Command<Commands::DELETE_OBJECT>(handle);
    }
  };
}