// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapFilterTypes.hpp"

#include "Language/Language.hpp"

#include <array>

static constexpr std::array<WaypointMapFilterType,
                            WAYPOINT_MAP_FILTER_TYPE_COUNT> types{{
  {Waypoint::Type::NORMAL, 0x3101, "WaypointTypeDisplayNormal",
   "WaypointTypeNormal", N_("Turnpoint")},
  {Waypoint::Type::AIRFIELD, 0x3102, "WaypointTypeDisplayAirfield",
   "WaypointTypeAirfield", N_("Airport")},
  {Waypoint::Type::OUTLANDING, 0x3103, "WaypointTypeDisplayOutlanding",
   "WaypointTypeOutlanding", N_("Landable")},
  {Waypoint::Type::MOUNTAIN_PASS, 0x3104, "WaypointTypeDisplayMountainPass",
   "WaypointTypeMountainPass", N_("Mountain Pass")},
  {Waypoint::Type::MOUNTAIN_TOP, 0x3105, "WaypointTypeDisplayMountainTop",
   "WaypointTypeMountainTop", N_("Mountain Top")},
  {Waypoint::Type::OBSTACLE, 0x3106, "WaypointTypeDisplayObstacle",
   "WaypointTypeObstacle", N_("Transmitter Mast")},
  {Waypoint::Type::VOR, 0x3107, "WaypointTypeDisplayVOR",
   "WaypointTypeVOR", N_("VOR")},
  {Waypoint::Type::NDB, 0x3108, "WaypointTypeDisplayNDB",
   "WaypointTypeNDB", N_("NDB")},
  {Waypoint::Type::TOWER, 0x3109, "WaypointTypeDisplayTower",
   "WaypointTypeTower", N_("Tower")},
  {Waypoint::Type::DAM, 0x310a, "WaypointTypeDisplayDam",
   "WaypointTypeDam", N_("Dam")},
  {Waypoint::Type::TUNNEL, 0x310b, "WaypointTypeDisplayTunnel",
   "WaypointTypeTunnel", N_("Tunnel")},
  {Waypoint::Type::BRIDGE, 0x310c, "WaypointTypeDisplayBridge",
   "WaypointTypeBridge", N_("Bridge")},
  {Waypoint::Type::POWERPLANT, 0x310d, "WaypointTypeDisplayPowerPlant",
   "WaypointTypePowerPlant", N_("Power Plant")},
  {Waypoint::Type::CASTLE, 0x310e, "WaypointTypeDisplayCastle",
   "WaypointTypeCastle", N_("Castle")},
  {Waypoint::Type::INTERSECTION, 0x310f,
   "WaypointTypeDisplayIntersection", "WaypointTypeIntersection",
   N_("Intersection")},
  {Waypoint::Type::MARKER, 0x3110, "WaypointTypeDisplayMarker",
   "WaypointTypeMarker", N_("Marker")},
  {Waypoint::Type::REPORTING_POINT, 0x3111,
   "WaypointTypeDisplayReportingPoint", "WaypointTypeReportingPoint",
   N_("Control Point")},
  {Waypoint::Type::PGTAKEOFF, 0x3112, "WaypointTypeDisplayPGTakeoff",
   "WaypointTypePGTakeoff", N_("PG Take Off")},
  {Waypoint::Type::PGLANDING, 0x3113, "WaypointTypeDisplayPGLanding",
   "WaypointTypePGLanding", N_("PG Landing Zone")},
}};

std::span<const WaypointMapFilterType>
GetWaypointMapFilterTypes() noexcept
{
  return types;
}

const WaypointMapFilterType *
FindWaypointMapFilterType(Waypoint::Type type) noexcept
{
  for (const auto &item : types)
    if (item.type == type)
      return &item;

  return nullptr;
}
