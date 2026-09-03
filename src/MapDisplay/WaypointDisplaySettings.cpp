// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "WaypointDisplaySettings.hpp"

#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Renderer/WaypointRendererSettings.hpp"
#include "Waypoint/MapFilterTypes.hpp"

namespace {

using namespace DisplaySettingCatalog;

static WaypointRendererSettings global_waypoint, effective_waypoint;

static bool
LoadGlobal() noexcept
{
  global_waypoint.SetDefaults();
  global_waypoint.LoadFromProfile();
  effective_waypoint = global_waypoint;
  return true;
}

static bool
GetWaypointGlobal(DisplaySettingKey key,
                  DisplaySettingValue &value) noexcept
{
  if (key == Key::WAYPOINT_LABEL_FORMAT)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_waypoint.display_text_type));
  else if (key == Key::WAYPOINT_ARRIVAL_HEIGHT)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_waypoint.arrival_height_display));
  else if (key == Key::WAYPOINT_LABEL_STYLE)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_waypoint.landable_render_mode));
  else if (key == Key::WAYPOINT_LABEL_VISIBILITY)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_waypoint.label_selection));
  else if (key == Key::WAYPOINT_LANDABLE_SYMBOLS)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_waypoint.landable_style));
  else if (key == Key::WAYPOINT_ICON_SCALE)
    value = DisplaySettingValue::Integer(
      global_waypoint.map_waypoint_icon_scale);
  else if (key == Key::WAYPOINT_DETAILED_LANDABLES)
    value = DisplaySettingValue::Boolean(
      global_waypoint.vector_landable_rendering);
  else if (key == Key::WAYPOINT_LANDABLE_SIZE)
    value = DisplaySettingValue::Integer(
      global_waypoint.landable_rendering_scale);
  else if (key == Key::WAYPOINT_SCALE_RUNWAY_LENGTH)
    value = DisplaySettingValue::Boolean(
      global_waypoint.scale_runway_length);
  else {
    for (const auto &item : GetWaypointMapFilterTypes())
      if (key.value == item.display_setting_key) {
        value = DisplaySettingValue::Boolean(
          global_waypoint.display_types[static_cast<unsigned>(item.type)]);
        return true;
      }

    return false;
  }

  return true;
}

static bool
SetWaypoint(WaypointRendererSettings &waypoint, DisplaySettingKey key,
            DisplaySettingValue value) noexcept
{
  if (key == Key::WAYPOINT_LABEL_FORMAT)
    waypoint.display_text_type =
      static_cast<WaypointRendererSettings::DisplayTextType>(value.value);
  else if (key == Key::WAYPOINT_ARRIVAL_HEIGHT)
    waypoint.arrival_height_display =
      static_cast<WaypointRendererSettings::ArrivalHeightDisplay>(
        value.value);
  else if (key == Key::WAYPOINT_LABEL_STYLE)
    waypoint.landable_render_mode = static_cast<LabelShape>(value.value);
  else if (key == Key::WAYPOINT_LABEL_VISIBILITY)
    waypoint.label_selection =
      static_cast<WaypointRendererSettings::LabelSelection>(value.value);
  else if (key == Key::WAYPOINT_LANDABLE_SYMBOLS)
    waypoint.landable_style =
      static_cast<WaypointRendererSettings::LandableStyle>(value.value);
  else if (key == Key::WAYPOINT_ICON_SCALE)
    waypoint.map_waypoint_icon_scale = value.value;
  else if (key == Key::WAYPOINT_DETAILED_LANDABLES)
    waypoint.vector_landable_rendering = value.AsBoolean();
  else if (key == Key::WAYPOINT_LANDABLE_SIZE)
    waypoint.landable_rendering_scale = value.value;
  else if (key == Key::WAYPOINT_SCALE_RUNWAY_LENGTH)
    waypoint.scale_runway_length = value.AsBoolean();
  else {
    for (const auto &item : GetWaypointMapFilterTypes())
      if (key.value == item.display_setting_key) {
        waypoint.display_types[static_cast<unsigned>(item.type)] =
          value.AsBoolean();
        return true;
      }

    return false;
  }

  return true;
}

static bool
SetWaypointGlobal(DisplaySettingKey key,
                  DisplaySettingValue value) noexcept
{
  return SetWaypoint(global_waypoint, key, value);
}

static bool
SetWaypointEffective(DisplaySettingKey key,
                     DisplaySettingValue value) noexcept
{
  return SetWaypoint(effective_waypoint, key, value);
}

static void
ApplyWaypoint(DisplaySettingEffects effects) noexcept
{
  CommonInterface::SetMapSettings().waypoint = effective_waypoint;

  if ((effects & ToDisplaySettingEffects(DisplaySettingEffect::WAYPOINT_LOOK))
      != 0 && CommonInterface::main_window != nullptr)
    CommonInterface::main_window->ReinitialiseLook();
}

} // namespace

void
RegisterWaypointDisplaySettings() noexcept
{
  DisplaySettingRuntime::Register(DisplaySettingGroup::WAYPOINTS, {
    LoadGlobal, GetWaypointGlobal, SetWaypointGlobal,
    SetWaypointEffective, ApplyWaypoint,
  });
}
