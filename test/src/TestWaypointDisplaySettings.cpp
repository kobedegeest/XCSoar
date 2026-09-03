// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Profile/Keys.hpp"
#include "Profile/Map.hpp"
#include "Renderer/WaypointRendererSettings.hpp"
#include "Waypoint/MapFilterTypes.hpp"
#include "TestUtil.hpp"

#include <cstring>

static void
TestMapFilterCatalog()
{
  const auto types = GetWaypointMapFilterTypes();
  ok1(types.size() == WAYPOINT_MAP_FILTER_TYPE_COUNT);

  for (std::size_t i = 0; i < types.size(); ++i) {
    const auto &item = types[i];
    ok1(static_cast<unsigned>(item.type) < WAYPOINT_TYPE_SLOT_COUNT);
    ok1(item.type != Waypoint::Type::THERMAL_HOTSPOT);
    ok1(item.global_profile_key[0] != '\0' &&
        item.override_profile_suffix[0] != '\0');

    bool unique = true;
    for (std::size_t j = 0; j < i; ++j)
      unique &= item.type != types[j].type &&
        item.display_setting_key != types[j].display_setting_key &&
        std::strcmp(item.global_profile_key,
                    types[j].global_profile_key) != 0 &&
        std::strcmp(item.override_profile_suffix,
                    types[j].override_profile_suffix) != 0;
    ok1(unique);
  }
}

static void
TestDefaultsAndTypeProfiles()
{
  WaypointRendererSettings settings;
  settings.SetDefaults();
  for (bool display : settings.display_types)
    ok1(display);

  const auto types = GetWaypointMapFilterTypes();
  const auto &normal = types.front();
  const auto &airfield = types[1];

  {
    ProfileMap map;
    map.Set(normal.global_profile_key, false);
    settings.SetDefaults();
    settings.LoadFromProfile(map);
    ok1(!settings.IsTypeDisplayed(normal.type));
    ok1(settings.IsTypeDisplayed(airfield.type));
  }

  {
    ProfileMap map;
    map.Set("WaypointTypeDisplay1", false);
    settings.SetDefaults();
    settings.LoadFromProfile(map);
    ok1(!settings.IsTypeDisplayed(Waypoint::Type::AIRFIELD));
  }

  {
    ProfileMap map;
    map.Set("WaypointTypeDisplay1", false);
    map.Set(airfield.global_profile_key, true);
    settings.SetDefaults();
    settings.LoadFromProfile(map);
    ok1(settings.IsTypeDisplayed(Waypoint::Type::AIRFIELD));
  }

  ok1(settings.IsTypeDisplayed(Waypoint::Type::THERMAL_HOTSPOT));
}

static void
TestValidatedGeneralProfile()
{
  ProfileMap map;
  map.Set(ProfileKeys::DisplayText, 99);
  map.Set(ProfileKeys::WaypointArrivalHeightDisplay, 99);
  map.Set(ProfileKeys::WaypointLabelSelection, 99);
  map.Set(ProfileKeys::WaypointLabelStyle, 0);
  map.Set(ProfileKeys::AppIndLandable, 99);
  map.Set(ProfileKeys::AppLandableRenderingScale, 1);
  map.Set(ProfileKeys::MapWaypointIconScale, 300);

  WaypointRendererSettings settings;
  settings.SetDefaults();
  settings.LoadFromProfile(map);
  ok1(settings.display_text_type ==
      WaypointRendererSettings::DisplayTextType::SHORT_NAME);
  ok1(settings.arrival_height_display ==
      WaypointRendererSettings::ArrivalHeightDisplay::GLIDE);
  ok1(settings.label_selection ==
      WaypointRendererSettings::LabelSelection::ALL);
  ok1(settings.landable_render_mode == LabelShape::ROUNDED_BLACK);
  ok1(settings.landable_style ==
      WaypointRendererSettings::LandableStyle::PURPLE_CIRCLE);
  ok1(settings.landable_rendering_scale == 100);
  ok1(settings.map_waypoint_icon_scale == 100);
}

static void
TestLegacyLabelMigration()
{
  {
    ProfileMap map;
    map.Set(ProfileKeys::DisplayText, static_cast<unsigned>(
      WaypointRendererSettings::DisplayTextType::
        OBSOLETE_DONT_USE_NAMEIFINTASK));

    WaypointRendererSettings settings;
    settings.SetDefaults();
    settings.LoadFromProfile(map);
    ok1(settings.display_text_type ==
        WaypointRendererSettings::DisplayTextType::NAME);
    ok1(settings.label_selection ==
        WaypointRendererSettings::LabelSelection::TASK);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::DisplayText, static_cast<unsigned>(
      WaypointRendererSettings::DisplayTextType::
        OBSOLETE_DONT_USE_NAMEIFINTASK));
    map.SetEnum(ProfileKeys::WaypointLabelSelection,
                WaypointRendererSettings::LabelSelection::ALL);

    WaypointRendererSettings settings;
    settings.SetDefaults();
    settings.LoadFromProfile(map);
    ok1(settings.display_text_type ==
        WaypointRendererSettings::DisplayTextType::NAME);
    ok1(settings.label_selection ==
        WaypointRendererSettings::LabelSelection::ALL);
  }
}

int
main()
{
  plan_tests(17 + 4 * WAYPOINT_MAP_FILTER_TYPE_COUNT +
             WAYPOINT_TYPE_SLOT_COUNT);
  TestMapFilterCatalog();
  TestDefaultsAndTypeProfiles();
  TestValidatedGeneralProfile();
  TestLegacyLabelMigration();
  return exit_status();
}
