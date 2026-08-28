// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

namespace TrafficThermalLayer {

static constexpr double MAX_MAP_SCALE = 4000;

/**
 * The complete set of global visibility gates for FLARM thermal markers.
 * Flight mode and map follow/pan state deliberately do not participate.
 */
static constexpr bool
IsVisible(bool show_flarm_on_map, double map_scale) noexcept
{
  return show_flarm_on_map && map_scale <= MAX_MAP_SCALE;
}

} // namespace TrafficThermalLayer
