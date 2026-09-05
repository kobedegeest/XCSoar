// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ElementSetDisplayOverrideGlue.hpp"
#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"
#include "AirspaceDisplaySettings.hpp"
#include "ElementSetDisplayOverrideService.hpp"
#include "TerrainOrientationDisplaySettings.hpp"
#include "WaypointDisplaySettings.hpp"

static ElementSetDisplayOverrideService service{
  GetMapDisplaySettingCatalog(),
  DisplaySettingRuntime::ApplyGroup,
};

bool
InitialiseElementSetDisplayOverrides() noexcept
{
  RegisterTerrainOrientationDisplaySettings();
  RegisterWaypointDisplaySettings();
  RegisterAirspaceDisplaySettings();
  if (!DisplaySettingRuntime::LoadGlobalValues())
    return false;

  return service.Initialise();
}

bool
ReloadGlobalElementSetDisplaySettings() noexcept
{
  if (!DisplaySettingRuntime::LoadGlobalValues())
    return false;

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
  return GetGlobalElementSetDisplaySettingValueByKey(descriptor.key);
}

DisplaySettingValue
GetGlobalElementSetDisplaySettingValueByKey(DisplaySettingKey key) noexcept
{
  const auto *value = service.GetGlobalValue(key);
  return value != nullptr ? *value : DisplaySettingValue{};
}
