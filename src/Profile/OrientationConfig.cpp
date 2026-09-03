// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "OrientationConfig.hpp"

#include "Keys.hpp"
#include "Map.hpp"
#include "MapSettings.hpp"

static bool
IsValidMapOrientation(unsigned value) noexcept
{
  switch (value) {
  case (unsigned)MapOrientation::TRACK_UP:
  case (unsigned)MapOrientation::NORTH_UP:
  case (unsigned)MapOrientation::TARGET_UP:
  case (unsigned)MapOrientation::HEADING_UP:
  case (unsigned)MapOrientation::WIND_UP:
    return true;
  }

  return false;
}

void
Profile::LoadOrientationSettings(const ProfileMap &map,
                                 MapSettings &settings) noexcept
{
  bool orientation_found = false;

  if (unsigned value = (unsigned)MapOrientation::NORTH_UP;
      map.Get(ProfileKeys::OrientationCircling, value)) {
    orientation_found = true;

    if (IsValidMapOrientation(value))
      settings.circling_orientation = (MapOrientation)value;
  }

  if (unsigned value = (unsigned)MapOrientation::NORTH_UP;
      map.Get(ProfileKeys::OrientationCruise, value)) {
    orientation_found = true;

    if (IsValidMapOrientation(value))
      settings.cruise_orientation = (MapOrientation)value;
  }

  if (orientation_found)
    return;

  unsigned value = 1;
  map.Get(ProfileKeys::DisplayUpValue, value);
  switch (value) {
  case 0:
    settings.cruise_orientation = MapOrientation::TRACK_UP;
    settings.circling_orientation = MapOrientation::TRACK_UP;
    break;
  case 1:
    settings.cruise_orientation = MapOrientation::NORTH_UP;
    settings.circling_orientation = MapOrientation::NORTH_UP;
    break;
  case 2:
    settings.cruise_orientation = MapOrientation::TRACK_UP;
    settings.circling_orientation = MapOrientation::NORTH_UP;
    break;
  case 3:
    settings.cruise_orientation = MapOrientation::TRACK_UP;
    settings.circling_orientation = MapOrientation::TARGET_UP;
    break;
  case 4:
    settings.cruise_orientation = MapOrientation::NORTH_UP;
    settings.circling_orientation = MapOrientation::TRACK_UP;
    break;
  }
}
