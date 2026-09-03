// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapFilterProfile.hpp"

#include "MapFilterTypes.hpp"
#include "Profile/Profile.hpp"
#include "Renderer/WaypointRendererSettings.hpp"
#include "util/StringFormat.hpp"

namespace WaypointMapFilterProfile {

bool
LoadTypeDisplay(const ProfileMap &map, Waypoint::Type type) noexcept
{
  const auto *item = FindWaypointMapFilterType(type);
  if (item == nullptr)
    return true;

  bool display = true;
  if (map.Get(item->global_profile_key, display))
    return display;

  /* Compatibility with the first experimental map-filter implementation. */
  char legacy_key[40];
  StringFormat(legacy_key, sizeof(legacy_key), "WaypointTypeDisplay%u",
               static_cast<unsigned>(type));
  map.Get(legacy_key, display);
  return display;
}

void
SaveTypeDisplay(Waypoint::Type type, bool display) noexcept
{
  const auto *item = FindWaypointMapFilterType(type);
  if (item != nullptr)
    Profile::Set(item->global_profile_key, display);
}

void
Load(const ProfileMap &map, WaypointRendererSettings &settings) noexcept
{
  for (const auto &item : GetWaypointMapFilterTypes())
    settings.display_types[static_cast<unsigned>(item.type)] =
      LoadTypeDisplay(map, item.type);
}

} // namespace WaypointMapFilterProfile
