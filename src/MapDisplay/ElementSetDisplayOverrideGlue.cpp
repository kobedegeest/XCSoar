// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ElementSetDisplayOverrideGlue.hpp"
#include "DisplaySettingCatalog.hpp"
#include "ElementSetDisplayOverrideService.hpp"

static ElementSetDisplayOverrideService service{
  GetMapDisplaySettingCatalog(),
};

bool
InitialiseElementSetDisplayOverrides() noexcept
{
  return service.Initialise();
}

void
ApplyElementSetDisplayOverrides(
  const ElementSetDisplayOverrides &overrides) noexcept
{
  service.Apply(overrides);
}

DisplaySettingValue
GetGlobalElementSetDisplaySettingValue(
  const DisplaySettingDescriptor &descriptor) noexcept
{
  const auto *value = service.GetGlobalValue(descriptor.key);
  return value != nullptr ? *value : DisplaySettingValue{};
}

bool
SetGlobalElementSetDisplaySettingValue(
  DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  return service.SetGlobalValue(key, value);
}
