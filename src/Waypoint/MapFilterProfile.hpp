// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Engine/Waypoint/Waypoint.hpp"

class ProfileMap;
struct WaypointRendererSettings;

namespace WaypointMapFilterProfile {

bool LoadTypeDisplay(const ProfileMap &map, Waypoint::Type type) noexcept;
void SaveTypeDisplay(Waypoint::Type type, bool display) noexcept;
void Load(const ProfileMap &map, WaypointRendererSettings &settings) noexcept;

} // namespace WaypointMapFilterProfile
