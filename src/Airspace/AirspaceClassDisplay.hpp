// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Engine/Airspace/AirspaceClass.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

struct AirspaceClassDisplaySetting {
  AirspaceClass airspace_class;
  uint16_t display_setting_key;
  const char *override_profile_suffix;
  const char *label;
};

static constexpr std::size_t AIRSPACE_CLASS_DISPLAY_SETTING_COUNT =
  AIRSPACECLASSCOUNT - 1;

std::span<const AirspaceClassDisplaySetting>
GetAirspaceClassDisplaySettings() noexcept;

const AirspaceClassDisplaySetting *
FindAirspaceClassDisplaySetting(AirspaceClass airspace_class) noexcept;
