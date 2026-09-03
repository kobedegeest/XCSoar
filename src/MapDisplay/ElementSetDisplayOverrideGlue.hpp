// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ElementSetDisplayOverrides.hpp"

/** Capture global values after the normal profile loaders have run. */
bool InitialiseElementSetDisplayOverrides() noexcept;

/** Reload validated global bundles and reapply the active element set. */
bool ReloadGlobalElementSetDisplaySettings() noexcept;

/** Resolve and apply the selected map element set's sparse overrides. */
void ApplyElementSetDisplayOverrides(
  const ElementSetDisplayOverrides &overrides) noexcept;

/** Value getter used when a new override is added by the generic editor. */
DisplaySettingValue GetGlobalElementSetDisplaySettingValue(
  const DisplaySettingDescriptor &descriptor) noexcept;

DisplaySettingValue GetGlobalElementSetDisplaySettingValueByKey(
  DisplaySettingKey key) noexcept;

/** Writer entry point for global configuration panels and quick actions. */
bool SetGlobalElementSetDisplaySettingValue(
  DisplaySettingKey key, DisplaySettingValue value) noexcept;
