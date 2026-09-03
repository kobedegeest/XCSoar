// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Profile/Keys.hpp"
#include "Profile/Map.hpp"
#include "Profile/OrientationConfig.hpp"
#include "Profile/TerrainConfig.hpp"
#include "MapSettings.hpp"
#include "Terrain/TerrainSettings.hpp"
#include "TestUtil.hpp"

static MapSettings
MakeOrientationDefaults() noexcept
{
  MapSettings settings{};
  settings.cruise_orientation = MapOrientation::NORTH_UP;
  settings.circling_orientation = MapOrientation::NORTH_UP;
  return settings;
}

static void
TestOrientationCompatibility()
{
  {
    ProfileMap map;
    auto settings = MakeOrientationDefaults();
    Profile::LoadOrientationSettings(map, settings);
    ok1(settings.cruise_orientation == MapOrientation::NORTH_UP);
    ok1(settings.circling_orientation == MapOrientation::NORTH_UP);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::DisplayUpValue, 0);
    auto settings = MakeOrientationDefaults();
    Profile::LoadOrientationSettings(map, settings);
    ok1(settings.cruise_orientation == MapOrientation::TRACK_UP);
    ok1(settings.circling_orientation == MapOrientation::TRACK_UP);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::DisplayUpValue, 3);
    auto settings = MakeOrientationDefaults();
    Profile::LoadOrientationSettings(map, settings);
    ok1(settings.cruise_orientation == MapOrientation::TRACK_UP);
    ok1(settings.circling_orientation == MapOrientation::TARGET_UP);
  }

  {
    ProfileMap map;
    map.SetEnum(ProfileKeys::OrientationCruise, MapOrientation::WIND_UP);
    map.SetEnum(ProfileKeys::OrientationCircling, MapOrientation::HEADING_UP);
    auto settings = MakeOrientationDefaults();
    Profile::LoadOrientationSettings(map, settings);
    ok1(settings.cruise_orientation == MapOrientation::WIND_UP);
    ok1(settings.circling_orientation == MapOrientation::HEADING_UP);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::DisplayUpValue, 0);
    map.Set(ProfileKeys::OrientationCruise, 99);
    auto settings = MakeOrientationDefaults();
    Profile::LoadOrientationSettings(map, settings);
    ok1(settings.cruise_orientation == MapOrientation::NORTH_UP);
    ok1(settings.circling_orientation == MapOrientation::NORTH_UP);
  }
}

static void
TestTerrainCompatibility()
{
  {
    ProfileMap map;
    map.Set(ProfileKeys::SlopeShading, false);
    TerrainRendererSettings settings;
    settings.SetDefaults();
    Profile::LoadTerrainRendererSettings(map, settings);
    ok1(settings.slope_shading == SlopeShading::OFF);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::SlopeShading, true);
    TerrainRendererSettings settings;
    settings.SetDefaults();
    Profile::LoadTerrainRendererSettings(map, settings);
    ok1(settings.slope_shading == SlopeShading::WIND);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::SlopeShading, false);
    map.SetEnum(ProfileKeys::SlopeShadingType, SlopeShading::FIXED);
    TerrainRendererSettings settings;
    settings.SetDefaults();
    Profile::LoadTerrainRendererSettings(map, settings);
    ok1(settings.slope_shading == SlopeShading::FIXED);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::SlopeShadingType, 99);
    TerrainRendererSettings settings;
    settings.SetDefaults();
    Profile::LoadTerrainRendererSettings(map, settings);
    ok1(settings.slope_shading == SlopeShading::WIND);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::TerrainRamp, 99);
    map.Set(ProfileKeys::TerrainContours, 99);
    TerrainRendererSettings settings;
    settings.SetDefaults();
    Profile::LoadTerrainRendererSettings(map, settings);
    ok1(settings.ramp == 0);
    ok1(settings.contours == Contours::OFF);
  }
}

int
main()
{
  plan_tests(16);
  TestOrientationCompatibility();
  TestTerrainCompatibility();
  return exit_status();
}
