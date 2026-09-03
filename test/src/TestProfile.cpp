// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Profile/Profile.hpp"
#include "Profile/Keys.hpp"
#include "Profile/Current.hpp"
#include "Profile/PageProfile.hpp"
#include "Profile/MapElementConfig.hpp"
#include "MapDisplay/DisplaySettingCatalog.hpp"
#include "PageSettings.hpp"
#include "Profile/Map.hpp"
#include "Profile/WeatherProfile.hpp"
#include "Weather/Settings.hpp"
#include "io/FileLineReader.hpp"
#include "system/Path.hpp"
#include "TestUtil.hpp"
#include "util/StringAPI.hxx"
#include "util/StaticString.hxx"
#include "util/PrintException.hxx"

#include <stdlib.h>

static constexpr DisplaySettingDescriptor display_override_descriptors[] = {
  {
    DisplaySettingKey{1}, DisplaySettingGroup::TERRAIN,
    "TerrainDisplay", "Terrain display", nullptr,
    DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr, true,
  },
  {
    DisplaySettingKey{2}, DisplaySettingGroup::TERRAIN,
    "TerrainBrightness", "Terrain brightness", nullptr,
    DisplaySettingValueType::INTEGER,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(100),
    nullptr, true,
  },
  {
    DisplaySettingKey{3}, DisplaySettingGroup::ORIENTATION,
    "DisabledSetting", "Disabled setting", nullptr,
    DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr,
  },
};

static void
TestMap()
{
  Profile::Clear();

  {
    int value;
    ok1(!Profile::Exists("key1"));
    ok1(!Profile::Get("key1", value));
    Profile::Set("key1", 4);
    ok1(Profile::Exists("key1"));
    ok1(Profile::Get("key1", value));
    ok1(value == 4);
  }

  {
    short value;
    ok1(!Profile::Get("key2", value));
    Profile::Set("key2", 123);
    ok1(Profile::Get("key2", value));
    ok1(value == 123);
  }

  {
    unsigned value;
    ok1(!Profile::Get("key3", value));
    Profile::Set("key3", -42);
    ok1(Profile::Get("key3", value));
    ok1(value == -42u);
  }

  {
    bool value;
    ok1(!Profile::Get("key4", value));
    Profile::Set("key4", true);
    ok1(Profile::Get("key4", value));
    ok1(value);
    Profile::Set("key4", false);
    ok1(Profile::Get("key4", value));
    ok1(!value);
  }

  {
    double value;
    ok1(!Profile::Get("key5", value));
    Profile::Set("key5", 1.337);
    ok1(Profile::Get("key5", value));
    ok1(equals(value, 1.337));
  }
}

static void
TestWriter()
{
  Profile::Clear();
  Profile::Set("key1", 4);
  Profile::Set("key2", "value2");

  Profile::SaveFile(Path("output/TestProfileWriter.prf"));

  FileLineReaderA reader(Path("output/TestProfileWriter.prf"));

  unsigned count = 0;
  bool found1 = false, found2 = false;

  char *line;
  while ((line = reader.ReadLine()) != NULL) {
    if (StringIsEqual(line, "key1=\"4\""))
      found1 = true;
    if (StringIsEqual(line, "key2=\"value2\""))
      found2 = true;

    count++;
  }

  ok1(count == 2);
  ok1(found1);
  ok1(found2);
}

static void
TestReader()
{
  Profile::Clear();
  Profile::LoadFile(Path("test/data/TestProfileReader.prf"));

  {
    int value;
    ok1(Profile::Exists("key1"));
    ok1(Profile::Get("key1", value));
    ok1(value == 1);
  }

  {
    StaticString<32> value;
    ok1(Profile::Exists("key2"));
    ok1(Profile::Get("key2", value));
    ok1(value == "value");
  }

  {
    int value;
    ok1(Profile::Exists("key3"));
    ok1(Profile::Get("key3", value));
    ok1(value == 5);
  }
}

static void
TestMigration()
{
  Profile::Clear();
  Profile::LoadFile(Path("test/data/TestProfileMigration.prf"));

  /* verify old keys are not present (they are consumed by migration) */
  ok1(!Profile::Exists("WPFile"));
  ok1(!Profile::Exists("AdditionalWPFile"));
  ok1(!Profile::Exists("AirspaceFile"));
  ok1(!Profile::Exists("AdditionalAirspaceFile"));
  ok1(!Profile::Exists("WatchedWPFile"));

  /* verify migrated waypoint list has primary file first */
  {
    StaticString<256> value;
    ok1(Profile::Get(ProfileKeys::WaypointFileList, value));
    ok1(value == "/path/to/main_waypoints.cup|/path/to/extra_waypoints.cup");
  }

  /* verify migrated airspace list has primary file first */
  {
    StaticString<256> value;
    ok1(Profile::Get(ProfileKeys::AirspaceFileList, value));
    ok1(value == "/path/to/main_airspace.txt|/path/to/extra_airspace.txt");
  }

  /* verify migrated watched waypoint list */
  {
    StaticString<256> value;
    ok1(Profile::Get(ProfileKeys::WatchedWaypointFileList, value));
    ok1(value == "/path/to/watched.cup");
  }

  /* verify non-migrated keys are preserved */
  {
    int value;
    ok1(Profile::Get(ProfileKeys::HomeWaypoint, value));
    ok1(value == 500);
  }
}

static void
TestWeatherPageCursorRoundTrip()
{
  PageSettings settings;
  settings.SetDefaults();

  auto &edl = settings.pages[0];
  edl.map_element_config.auto_switch = false;
  edl.map_element_config.set = 5;
  edl.overlay = PageLayout::Overlay::EDL;
  edl.edl_time = 500000;
  edl.edl_isobar = 70000;
  edl.Normalise();

  auto &xctherm = settings.pages[1];
  xctherm.map_element_config.auto_switch = true;
  xctherm.map_element_config.set = 7;
  xctherm.overlay = PageLayout::Overlay::XCTHERM;
  xctherm.xctherm_layer = 3;
  xctherm.xctherm_time = 15;
  xctherm.Normalise();

  settings.n_pages = 3;
  auto &skysight = settings.pages[2];
  skysight = PageLayout::Default();
  skysight.overlay = PageLayout::Overlay::SKYSIGHT;
  skysight.skysight_overlay = "wind_925";
  skysight.skysight_time = 1785542400;
  skysight.Normalise();

  Profile::Clear();
  Profile::Save(Profile::map, settings);

  PageSettings loaded;
  loaded.SetDefaults();
  Profile::Load(Profile::map, loaded);

  ok1(loaded.pages[0].edl_time == edl.edl_time);
  ok1(loaded.pages[0].edl_isobar == edl.edl_isobar);
  ok1(!loaded.pages[0].map_element_config.auto_switch);
  ok1(loaded.pages[0].map_element_config.set == 5);
  ok1(loaded.pages[1].xctherm_layer == xctherm.xctherm_layer);
  ok1(loaded.pages[1].xctherm_time == xctherm.xctherm_time);
  ok1(loaded.pages[1].map_element_config.auto_switch);
  ok1(loaded.pages[1].map_element_config.set ==
      MapElementSettings::SET_CIRCLING);
  ok1(loaded.pages[2].skysight_overlay == skysight.skysight_overlay);
  ok1(loaded.pages[2].skysight_time == skysight.skysight_time);
}

static void
TestMapElementSetRoundTrip()
{
  MapElementSet source{};
  source.name = "Weather";
  source.final_glide_bar_display_mode = FinalGlideBarDisplayMode::AUTO;
  source.final_glide_bar_mc0_enabled = false;
  source.trail.wind_drift_enabled = false;
  source.trail.scaling_enabled = true;
  source.trail.type = TrailSettings::Type::VARIO_EINK;
  source.trail.length = TrailSettings::Length::SHORT;
  source.distance_rings_enabled = true;
  source.display_ground_track = DisplayGroundTrack::ON;
  source.show_flarm_on_map = false;
  source.flarm_gauge_enabled = false;
  source.final_glide_terrain =
    FeaturesSettings::FinalGlideTerrain::WORKING_TERRAIN_SHADE;
  source.show_thermal_profile = false;
  source.vario_bar_enabled = true;
  source.detour_cost_markers_enabled = true;
  source.wind_arrow_style = WindArrowStyle::FULL_ARROW;
  source.online_traffic_map_mode =
    DisplayOnlineTrafficMapMode::SYMBOL_NAME;
  source.thermal_assistant_position = ThermalAssistantPosition::TOP_RIGHT;
  source.turn_back_marker_enabled = true;
  source.show_menu_button = true;
  source.show_zoom_button = false;
  source.show_quickmenu_button = true;
#ifdef HAVE_TRACKING
  source.cloud_show_thermals = false;
#endif
#ifdef HAVE_HTTP
  source.enable_thermal_information_map = true;
#endif

  ProfileMap map;
  Profile::Save(map, source, 3);

  MapElementSettings loaded{};
  Profile::Load(map, loaded);
  const auto &result = loaded.sets[3];

  ok1(StringIsEqual(result.name.c_str(), source.name.c_str()));
  ok1(result.final_glide_bar_display_mode ==
      source.final_glide_bar_display_mode);
  ok1(result.final_glide_bar_mc0_enabled ==
      source.final_glide_bar_mc0_enabled);
  ok1(result.trail.wind_drift_enabled == source.trail.wind_drift_enabled);
  ok1(result.trail.scaling_enabled == source.trail.scaling_enabled);
  ok1(result.trail.type == source.trail.type);
  ok1(result.trail.length == source.trail.length);
  ok1(result.distance_rings_enabled == source.distance_rings_enabled);
  ok1(result.display_ground_track == source.display_ground_track);
  ok1(result.show_flarm_on_map == source.show_flarm_on_map);
  ok1(result.flarm_gauge_enabled == source.flarm_gauge_enabled);
  ok1(result.final_glide_terrain == source.final_glide_terrain);
  ok1(result.show_thermal_profile == source.show_thermal_profile);
  ok1(result.vario_bar_enabled == source.vario_bar_enabled);
  ok1(result.detour_cost_markers_enabled ==
      source.detour_cost_markers_enabled);
  ok1(result.wind_arrow_style == source.wind_arrow_style);
  ok1(result.online_traffic_map_mode == source.online_traffic_map_mode);
  ok1(result.thermal_assistant_position ==
      source.thermal_assistant_position);
  ok1(result.turn_back_marker_enabled == source.turn_back_marker_enabled);
  ok1(result.show_menu_button == source.show_menu_button);
  ok1(result.show_zoom_button == source.show_zoom_button);
  ok1(result.show_quickmenu_button == source.show_quickmenu_button);
#ifdef HAVE_TRACKING
  ok1(result.cloud_show_thermals == source.cloud_show_thermals);
#endif
#ifdef HAVE_HTTP
  ok1(result.enable_thermal_information_map ==
      source.enable_thermal_information_map);
#endif
}

static void
TestMapElementSetLegacyMigration()
{
  ProfileMap map;
  map.Set(ProfileKeys::DetourCostMarker, true);
  map.SetEnum(ProfileKeys::WindArrowStyle, WindArrowStyle::NO_ARROW);
  map.SetEnum(ProfileKeys::OnlineTrafficMapMode,
              DisplayOnlineTrafficMapMode::SYMBOL_NAME);
  map.SetEnum(ProfileKeys::TAPosition,
              ThermalAssistantPosition::CENTER_TOP);
  map.Set(ProfileKeys::TurnBackMarkerEnabled, true);
  map.Set(ProfileKeys::ShowMenuButton, true);
  map.Set(ProfileKeys::ShowZoomButton, false);
  map.Set(ProfileKeys::ShowQuickMenuButton, true);
#ifdef HAVE_TRACKING
  map.Set(ProfileKeys::CloudShowThermals, false);
#endif
#ifdef HAVE_HTTP
  map.Set(ProfileKeys::EnableThermalInformationMap, true);
#endif

  MapElementSettings loaded{};
  Profile::Load(map, loaded);
  const auto &result = loaded.sets[MapElementSettings::SET_CRUISE];

  ok1(result.detour_cost_markers_enabled);
  ok1(result.wind_arrow_style == WindArrowStyle::NO_ARROW);
  ok1(result.online_traffic_map_mode ==
      DisplayOnlineTrafficMapMode::SYMBOL_NAME);
  ok1(result.thermal_assistant_position ==
      ThermalAssistantPosition::CENTER_TOP);
  ok1(result.turn_back_marker_enabled);
  ok1(result.show_menu_button);
  ok1(!result.show_zoom_button);
  ok1(result.show_quickmenu_button);
#ifdef HAVE_TRACKING
  ok1(!result.cloud_show_thermals);
#endif
#ifdef HAVE_HTTP
  ok1(result.enable_thermal_information_map);
#endif
}

static void
TestElementSetDisplayOverridePersistence()
{
  static constexpr unsigned set_index = 3;
  static constexpr char terrain_display_key[] =
    "MapElementSet3DisplayOverrideTerrainDisplay";
  static constexpr char terrain_brightness_key[] =
    "MapElementSet3DisplayOverrideTerrainBrightness";
  static constexpr char disabled_setting_key[] =
    "MapElementSet3DisplayOverrideDisabledSetting";

  ElementSetDisplayOverrides source{};
  source.Clear();
  ok1(source.Set(display_override_descriptors[0],
                 DisplaySettingValue::Boolean(true)) ==
      SetDisplayOverrideResult::ADDED);
  ok1(source.Set(display_override_descriptors[1],
                 DisplaySettingValue::Integer(63)) ==
      SetDisplayOverrideResult::ADDED);

  ProfileMap map;
  Profile::SaveElementSetDisplayOverrides(
    map, set_index, source, display_override_descriptors);
  ok1(map.Exists(terrain_display_key));
  ok1(map.Exists(terrain_brightness_key));
  ok1(!map.Exists(disabled_setting_key));

  ElementSetDisplayOverrides loaded{};
  loaded.Clear();
  Profile::LoadElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(loaded == source);
  ok1(loaded.Get(display_override_descriptors[0].key)->AsBoolean());
  ok1(loaded.Get(display_override_descriptors[1].key)->value == 63);

  map.Clear();
  Profile::LoadElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(loaded.IsEmpty());

  map.Set(terrain_display_key, 2);
  map.Set(terrain_brightness_key, 101);
  Profile::LoadElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(loaded.IsEmpty());

  map.Set(terrain_display_key, true);
  Profile::LoadElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(loaded.Size() == 1);
  ok1(loaded.Get(display_override_descriptors[0].key)->AsBoolean());

  map.Clear();
  Profile::SaveElementSetDisplayOverrides(
    map, set_index, source, display_override_descriptors);
  ok1(map.Exists(terrain_display_key));
  ok1(map.Exists(terrain_brightness_key));
  ok1(source.Remove(display_override_descriptors[0].key));
  Profile::SaveElementSetDisplayOverrides(
    map, set_index, source, display_override_descriptors);
  ok1(!map.Exists(terrain_display_key));
  ok1(map.Exists(terrain_brightness_key));
  source.Clear();
  Profile::SaveElementSetDisplayOverrides(
    map, set_index, source, display_override_descriptors);
  ok1(!map.Exists(terrain_brightness_key));

  map.Set(disabled_setting_key, true);
  Profile::LoadElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(loaded.IsEmpty());
  Profile::SaveElementSetDisplayOverrides(
    map, set_index, loaded, display_override_descriptors);
  ok1(!map.Exists(disabled_setting_key));
}

static void
TestFullDisplayOverrideCatalogRoundTrip()
{
  const auto catalog = GetMapDisplaySettingCatalog();
  ElementSetDisplayOverrides source{};
  source.Clear();

  for (const auto &descriptor : catalog) {
    DisplaySettingValue value = descriptor.minimum;
    if (descriptor.value_type == DisplaySettingValueType::ENUM &&
        descriptor.enum_choice_count > 0)
      value = DisplaySettingValue::Enum(descriptor.enum_choices[0].value);

    ok1(source.Set(descriptor, value) == SetDisplayOverrideResult::ADDED);
  }

  ok1(source.Size() == DisplaySettingCatalog::COUNT);

  ProfileMap map;
  Profile::SaveElementSetDisplayOverrides(map, 0, source, catalog);

  ElementSetDisplayOverrides loaded{};
  loaded.Clear();
  Profile::LoadElementSetDisplayOverrides(map, 0, loaded, catalog);
  ok1(loaded == source);
}

#ifdef HAVE_HTTP

static void
TestSkySightProfileCompatibility()
{
  {
    ProfileMap map;
    WeatherSettings settings;
    settings.SetDefaults();
    ok1(settings.skysight.auto_update);

    map.Set(ProfileKeys::SkySightAutoUpdate, false);
    Profile::Load(map, settings);
    ok1(!settings.skysight.auto_update);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::LegacySkySightEmail, "legacy@example.com");
    map.Set(ProfileKeys::LegacySkySightPassword, "legacy-password");
    map.Set(ProfileKeys::LegacySkySightRegion, "EUROPE");

    WeatherSettings settings{};
    Profile::Load(map, settings);
    ok1(settings.skysight.email == "legacy@example.com");
    ok1(settings.skysight.password == "legacy-password");
    ok1(settings.skysight.region == "EUROPE");
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::LegacySkySightEmail, "legacy@example.com");
    map.Set(ProfileKeys::LegacySkySightPassword, "legacy-password");
    map.Set(ProfileKeys::LegacySkySightRegion, "EUROPE");
    map.Set(ProfileKeys::SkySightEmail, "canonical@example.com");
    map.Set(ProfileKeys::SkySightPassword, "canonical-password");
    map.Set(ProfileKeys::SkySightRegion, "NZ");

    WeatherSettings settings{};
    Profile::Load(map, settings);
    ok1(settings.skysight.email == "canonical@example.com");
    ok1(settings.skysight.password == "canonical-password");
    ok1(settings.skysight.region == "NZ");
  }
}

#endif

int main()
try {
  plan_tests(106 + DisplaySettingCatalog::COUNT
#ifdef HAVE_TRACKING
             + 2
#endif
#ifdef HAVE_HTTP
             + 10
#endif
             );

  TestMap();
  TestWriter();
  TestReader();
  TestMigration();
  TestWeatherPageCursorRoundTrip();
  TestMapElementSetRoundTrip();
  TestMapElementSetLegacyMigration();
  TestElementSetDisplayOverridePersistence();
  TestFullDisplayOverrideCatalogRoundTrip();
#ifdef HAVE_HTTP
  TestSkySightProfileCompatibility();
#endif

  return exit_status();
} catch (...) {
  PrintException(std::current_exception());
  return EXIT_FAILURE;
}
