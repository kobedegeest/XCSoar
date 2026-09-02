// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "MapElementSettings.hpp"

#include <span>

class ProfileMap;

namespace Profile
{
  void Load(const ProfileMap &map, MapElementSettings &settings);
  void Save(ProfileMap &map, const MapElementSet &set, unsigned index);

  /** Load sparse overrides for one element set from stable profile keys. */
  void LoadElementSetDisplayOverrides(
    const ProfileMap &map, unsigned index,
    ElementSetDisplayOverrides &overrides,
    std::span<const DisplaySettingDescriptor> catalog) noexcept;

  /**
   * Save sparse overrides for one element set and remove inherited values.
   */
  void SaveElementSetDisplayOverrides(
    ProfileMap &map, unsigned index,
    const ElementSetDisplayOverrides &overrides,
    std::span<const DisplaySettingDescriptor> catalog) noexcept;
};
