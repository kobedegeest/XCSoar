// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Engine/Waypoint/Waypoint.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

/** Number of values in the contiguous #Waypoint::Type enum. */
static constexpr std::size_t WAYPOINT_TYPE_SLOT_COUNT =
  static_cast<std::size_t>(Waypoint::Type::PGLANDING) + 1;

/**
 * One user-configurable waypoint type.  The explicit strings and setting key
 * stay stable if the enum or presentation order changes.
 */
struct WaypointMapFilterType {
  Waypoint::Type type;
  uint16_t display_setting_key;
  const char *global_profile_key;
  const char *override_profile_suffix;
  const char *label;
};

static constexpr std::size_t WAYPOINT_MAP_FILTER_TYPE_COUNT = 19;

std::span<const WaypointMapFilterType>
GetWaypointMapFilterTypes() noexcept;

const WaypointMapFilterType *
FindWaypointMapFilterType(Waypoint::Type type) noexcept;
