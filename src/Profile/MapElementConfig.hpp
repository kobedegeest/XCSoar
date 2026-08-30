// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "MapElementSettings.hpp"

class ProfileMap;

namespace Profile
{
  void Load(const ProfileMap &map, MapElementSettings &settings);
  void Save(ProfileMap &map, const MapElementSet &set, unsigned index);
};
