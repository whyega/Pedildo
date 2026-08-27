#pragma once

#include <sampapi/0.3.7-R1/CPed.h>

#include <array>
#include <cstddef>

namespace pedildo {

using SampPed = sampapi::v037r1::CPed;

struct Vector3 {
  float x;
  float y;
  float z;
};

struct Attachment {
  int slot;
  int bone;
  Vector3 position;
  Vector3 rotation;
  Vector3 scale;
};

struct Preset {
  int model;
  std::array<Attachment, 5> attachments;
};

inline constexpr std::array<Preset, 3> kPresets{{
    {321,
     {{{1, 1, {0.00F, 0.30F, 0.15F}, {270.0F, 120.0F, 0.0F},
        {4.0F, 4.0F, 2.0F}},
       {3, 3, {0.40F, -0.15F, 0.00F}, {0.0F, 270.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {4, 4, {0.00F, -0.15F, 0.10F}, {0.0F, 130.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {7, 7, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}},
       {8, 8, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}}}}},
    {322,
     {{{1, 1, {0.34F, 0.14F, 0.09F}, {270.0F, 120.0F, 0.0F},
        {4.0F, 4.0F, 2.0F}},
       {3, 3, {0.14F, -0.05F, -0.24F}, {0.0F, 270.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {4, 4, {0.11F, 0.06F, 0.10F}, {0.0F, 130.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {7, 7, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}},
       {8, 8, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}}}}},
    {323,
     {{{1, 1, {0.30F, 0.13F, 0.11F}, {270.0F, 120.0F, 0.0F},
        {4.0F, 4.0F, 2.0F}},
       {3, 3, {0.40F, 0.09F, -0.20F}, {0.0F, 270.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {4, 4, {0.03F, 0.09F, 0.10F}, {0.0F, 130.0F, 0.0F},
        {1.5F, 1.5F, 1.5F}},
       {7, 7, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}},
       {8, 8, {0.00F, 0.10F, 0.10F}, {180.0F, 90.0F, 90.0F},
        {2.0F, 2.0F, 2.5F}}}}},
}};

struct AccessoryBackup {
  bool occupied{};
  SampPed::Accessory accessory{};
};

using AccessoryBackups = std::array<AccessoryBackup, 5>;

inline SampPed::Accessory MakeAccessory(int model,
                                        const Attachment& attachment) {
  SampPed::Accessory accessory{};
  accessory.m_nModel = model;
  accessory.m_nBone = attachment.bone;
  accessory.m_offset.Set(attachment.position.x, attachment.position.y,
                         attachment.position.z);
  accessory.m_rotation.Set(attachment.rotation.x, attachment.rotation.y,
                           attachment.rotation.z);
  accessory.m_scale.Set(attachment.scale.x, attachment.scale.y,
                        attachment.scale.z);
  accessory.m_firstMaterialColor = 0xFFFFFFFFu;
  accessory.m_secondMaterialColor = 0xFFFFFFFFu;
  return accessory;
}

inline void ApplyPreset(SampPed& ped, const Preset& preset) {
  for (const auto& attachment : preset.attachments) {
    if (ped.GetAccessoryState(attachment.slot)) {
      ped.DeleteAccessory(attachment.slot);
    }

    const auto accessory = MakeAccessory(preset.model, attachment);
    ped.AddAccessory(attachment.slot, &accessory);
  }
}

inline bool PresetMatches(SampPed& ped, const Preset& preset) {
  for (const auto& attachment : preset.attachments) {
    if (!ped.GetAccessoryState(attachment.slot)) {
      return false;
    }

    const auto& accessory = ped.m_accessories.m_info[attachment.slot];
    if (accessory.m_nModel != preset.model ||
        accessory.m_nBone != attachment.bone) {
      return false;
    }
  }
  return true;
}

inline bool PresetReady(SampPed& ped, const Preset& preset) {
  if (!PresetMatches(ped, preset)) {
    return false;
  }

  for (const auto& attachment : preset.attachments) {
    if (!ped.GetAccessory(attachment.slot)) {
      return false;
    }
  }
  return true;
}

inline AccessoryBackups CaptureAccessories(SampPed& ped) {
  AccessoryBackups backups{};
  const auto& attachments = kPresets.front().attachments;

  for (std::size_t i = 0; i < attachments.size(); ++i) {
    const auto slot = attachments[i].slot;
    backups[i].occupied = ped.GetAccessoryState(slot) != 0;
    if (backups[i].occupied) {
      backups[i].accessory = ped.m_accessories.m_info[slot];
    }
  }
  return backups;
}

inline void RestoreAccessories(SampPed& ped,
                               const AccessoryBackups& backups) {
  const auto& attachments = kPresets.front().attachments;

  for (std::size_t i = 0; i < attachments.size(); ++i) {
    const auto slot = attachments[i].slot;
    if (ped.GetAccessoryState(slot)) {
      ped.DeleteAccessory(slot);
    }
    if (backups[i].occupied) {
      ped.AddAccessory(slot, &backups[i].accessory);
    }
  }
}

}  
