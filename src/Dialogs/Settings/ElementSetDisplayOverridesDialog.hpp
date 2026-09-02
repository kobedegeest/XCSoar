// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "MapDisplay/ElementSetDisplayOverrides.hpp"

#include <span>

struct DialogLook;

namespace UI {
class SingleWindow;
}

using GlobalDisplaySettingValueGetter =
  DisplaySettingValue (*)(const DisplaySettingDescriptor &descriptor) noexcept;

/**
 * Edit the optional display settings owned by one map element set.
 *
 * @return true if the sparse override collection was changed
 */
bool ShowElementSetDisplayOverridesDialog(
  UI::SingleWindow &parent, const DialogLook &look,
  ElementSetDisplayOverrides &overrides,
  std::span<const DisplaySettingDescriptor> catalog,
  GlobalDisplaySettingValueGetter get_global_value);
