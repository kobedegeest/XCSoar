// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

class ProfileMap;
struct MapSettings;

namespace Profile {

/** Load both orientations through the shared legacy-compatible path. */
void LoadOrientationSettings(const ProfileMap &map,
                             MapSettings &settings) noexcept;

} // namespace Profile
