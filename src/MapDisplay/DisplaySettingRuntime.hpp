// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ElementSetDisplayOverrides.hpp"

namespace DisplaySettingRuntime {

struct GroupHandler {
  bool (*load_global)() noexcept;
  bool (*get_global)(DisplaySettingKey key,
                     DisplaySettingValue &value) noexcept;
  bool (*set_global)(DisplaySettingKey key,
                     DisplaySettingValue value) noexcept;
  bool (*set_effective)(DisplaySettingKey key,
                        DisplaySettingValue value) noexcept;
  void (*apply_effective)(DisplaySettingEffects effects) noexcept;
};

void Register(DisplaySettingGroup group, GroupHandler handler) noexcept;

/** Load each registered global bundle through its normal profile loader. */
bool LoadGlobalValues() noexcept;

DisplaySettingValue GetGlobalValue(DisplaySettingKey key) noexcept;
bool SetGlobalValue(DisplaySettingKey key,
                    DisplaySettingValue value) noexcept;
bool SetEffectiveValue(DisplaySettingKey key,
                       DisplaySettingValue value) noexcept;

/** Apply the staged effective bundle once for the selected group. */
void ApplyGroup(DisplaySettingGroup group,
                DisplaySettingEffects effects) noexcept;

} // namespace DisplaySettingRuntime
