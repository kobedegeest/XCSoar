// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ElementSetDisplayOverrides.hpp"

#include <cstddef>
#include <span>

/**
 * Map-display settings known to the element-set override system.
 *
 * The catalog is deliberately deny-by-default: a descriptor is visible in
 * the element-set override editor only after its element_set_overwritable
 * flag has been enabled explicitly.
 */
std::span<const DisplaySettingDescriptor>
GetMapDisplaySettingCatalog() noexcept;

namespace DisplaySettingCatalog {

static constexpr std::size_t TERRAIN_COUNT = 7;
static constexpr std::size_t ORIENTATION_COUNT = 5;
static constexpr std::size_t WAYPOINT_GENERAL_COUNT = 9;
static constexpr std::size_t WAYPOINT_TYPE_COUNT = 19;
static constexpr std::size_t WAYPOINT_COUNT =
  WAYPOINT_GENERAL_COUNT + WAYPOINT_TYPE_COUNT;
static constexpr std::size_t AIRSPACE_GENERAL_COUNT = 13;
static constexpr std::size_t AIRSPACE_CLASS_COUNT = 52;
static constexpr std::size_t AIRSPACE_COUNT =
  AIRSPACE_GENERAL_COUNT + AIRSPACE_CLASS_COUNT;
static constexpr std::size_t COUNT =
  TERRAIN_COUNT + ORIENTATION_COUNT + WAYPOINT_COUNT + AIRSPACE_COUNT;

namespace Key {

static constexpr DisplaySettingKey TERRAIN_DISPLAY{0x1001};
static constexpr DisplaySettingKey TOPOGRAPHY_DISPLAY{0x1002};
static constexpr DisplaySettingKey TERRAIN_RAMP{0x1003};
static constexpr DisplaySettingKey TERRAIN_SLOPE_SHADING{0x1004};
static constexpr DisplaySettingKey TERRAIN_CONTRAST{0x1005};
static constexpr DisplaySettingKey TERRAIN_BRIGHTNESS{0x1006};
static constexpr DisplaySettingKey TERRAIN_CONTOURS{0x1007};

static constexpr DisplaySettingKey CRUISE_ORIENTATION{0x2001};
static constexpr DisplaySettingKey CIRCLING_ORIENTATION{0x2002};
static constexpr DisplaySettingKey CIRCLING_ZOOM{0x2003};
static constexpr DisplaySettingKey MAP_SHIFT_BIAS{0x2004};
static constexpr DisplaySettingKey GLIDER_SCREEN_POSITION{0x2005};

static constexpr DisplaySettingKey WAYPOINT_LABEL_FORMAT{0x3001};
static constexpr DisplaySettingKey WAYPOINT_ARRIVAL_HEIGHT{0x3002};
static constexpr DisplaySettingKey WAYPOINT_LABEL_STYLE{0x3003};
static constexpr DisplaySettingKey WAYPOINT_LABEL_VISIBILITY{0x3004};
static constexpr DisplaySettingKey WAYPOINT_LANDABLE_SYMBOLS{0x3005};
static constexpr DisplaySettingKey WAYPOINT_ICON_SCALE{0x3006};
static constexpr DisplaySettingKey WAYPOINT_DETAILED_LANDABLES{0x3007};
static constexpr DisplaySettingKey WAYPOINT_LANDABLE_SIZE{0x3008};
static constexpr DisplaySettingKey WAYPOINT_SCALE_RUNWAY_LENGTH{0x3009};

static constexpr DisplaySettingKey AIRSPACE_DISPLAY{0x4001};
static constexpr DisplaySettingKey AIRSPACE_LABEL_VISIBILITY{0x4002};
static constexpr DisplaySettingKey AIRSPACE_SHOW_NOTAM_LABELS{0x4003};
static constexpr DisplaySettingKey AIRSPACE_CLIP_ALTITUDE{0x4004};
static constexpr DisplaySettingKey AIRSPACE_MARGIN{0x4005};
static constexpr DisplaySettingKey AIRSPACE_WARNINGS{0x4006};
static constexpr DisplaySettingKey AIRSPACE_WARNING_DIALOG{0x4007};
static constexpr DisplaySettingKey AIRSPACE_WARNING_TIME{0x4008};
static constexpr DisplaySettingKey AIRSPACE_REPETITIVE_SOUND{0x4009};
static constexpr DisplaySettingKey AIRSPACE_ACKNOWLEDGE_TIME{0x400a};
static constexpr DisplaySettingKey AIRSPACE_BLACK_OUTLINE{0x400b};
static constexpr DisplaySettingKey AIRSPACE_FILL_MODE{0x400c};
static constexpr DisplaySettingKey AIRSPACE_TRANSPARENCY{0x400d};

} // namespace Key

} // namespace DisplaySettingCatalog
