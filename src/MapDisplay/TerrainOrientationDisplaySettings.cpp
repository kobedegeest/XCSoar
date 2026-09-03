// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "TerrainOrientationDisplaySettings.hpp"

#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"
#include "Interface.hpp"
#include "MapSettings.hpp"
#include "MapWindow/GlueMapWindow.hpp"
#include "Profile/Current.hpp"
#include "Profile/MapProfile.hpp"
#include "UIGlobals.hpp"

namespace {

using namespace DisplaySettingCatalog;

struct TerrainBundle {
  TerrainRendererSettings terrain;
  bool topography_enabled;
};

struct OrientationBundle {
  MapOrientation cruise_orientation;
  MapOrientation circling_orientation;
  bool circle_zoom_enabled;
  MapShiftBias map_shift_bias;
  int glider_screen_position;
};

static TerrainBundle global_terrain, effective_terrain;
static OrientationBundle global_orientation, effective_orientation;

static constexpr int32_t
TerrainByteToPercent(short value) noexcept
{
  return (value * 200 + 100) / 510;
}

static constexpr short
TerrainPercentToByte(int32_t value) noexcept
{
  return short((value * 510 + 255) / 200);
}

static bool
LoadGlobal() noexcept
{
  MapSettings settings;
  settings.SetDefaults();
  Profile::Load(Profile::map, settings);

  global_terrain = {settings.terrain, settings.topography_enabled};
  effective_terrain = global_terrain;
  global_orientation = {
    settings.cruise_orientation,
    settings.circling_orientation,
    settings.circle_zoom_enabled,
    settings.map_shift_bias,
    settings.glider_screen_position,
  };
  effective_orientation = global_orientation;
  return true;
}

static bool
GetTerrainGlobal(DisplaySettingKey key, DisplaySettingValue &value) noexcept
{
  if (key == Key::TERRAIN_DISPLAY)
    value = DisplaySettingValue::Boolean(global_terrain.terrain.enable);
  else if (key == Key::TOPOGRAPHY_DISPLAY)
    value = DisplaySettingValue::Boolean(global_terrain.topography_enabled);
  else if (key == Key::TERRAIN_RAMP)
    value = DisplaySettingValue::Enum(global_terrain.terrain.ramp);
  else if (key == Key::TERRAIN_SLOPE_SHADING)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_terrain.terrain.slope_shading));
  else if (key == Key::TERRAIN_CONTRAST)
    value = DisplaySettingValue::Integer(
      TerrainByteToPercent(global_terrain.terrain.contrast));
  else if (key == Key::TERRAIN_BRIGHTNESS)
    value = DisplaySettingValue::Integer(
      TerrainByteToPercent(global_terrain.terrain.brightness));
  else if (key == Key::TERRAIN_CONTOURS)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_terrain.terrain.contours));
  else
    return false;

  return true;
}

static bool
SetTerrain(TerrainBundle &bundle, DisplaySettingKey key,
           DisplaySettingValue value) noexcept
{
  if (key == Key::TERRAIN_DISPLAY)
    bundle.terrain.enable = value.AsBoolean();
  else if (key == Key::TOPOGRAPHY_DISPLAY)
    bundle.topography_enabled = value.AsBoolean();
  else if (key == Key::TERRAIN_RAMP)
    bundle.terrain.ramp = static_cast<unsigned short>(value.value);
  else if (key == Key::TERRAIN_SLOPE_SHADING)
    bundle.terrain.slope_shading = static_cast<SlopeShading>(value.value);
  else if (key == Key::TERRAIN_CONTRAST)
    bundle.terrain.contrast = TerrainPercentToByte(value.value);
  else if (key == Key::TERRAIN_BRIGHTNESS)
    bundle.terrain.brightness = TerrainPercentToByte(value.value);
  else if (key == Key::TERRAIN_CONTOURS)
    bundle.terrain.contours = static_cast<Contours>(value.value);
  else
    return false;

  return true;
}

static bool
SetTerrainGlobal(DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  return SetTerrain(global_terrain, key, value);
}

static bool
SetTerrainEffective(DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  return SetTerrain(effective_terrain, key, value);
}

static void
ApplyTerrain(DisplaySettingEffects effects) noexcept
{
  auto terrain = effective_terrain.terrain;

  /* Contrast and brightness are exposed as percentages, while legacy
     profiles store bytes.  Restore the exact global byte whenever the
     effective percentage inherits it, avoiding a lossy round trip. */
  if (TerrainByteToPercent(terrain.contrast) ==
      TerrainByteToPercent(global_terrain.terrain.contrast))
    terrain.contrast = global_terrain.terrain.contrast;
  if (TerrainByteToPercent(terrain.brightness) ==
      TerrainByteToPercent(global_terrain.terrain.brightness))
    terrain.brightness = global_terrain.terrain.brightness;

  auto &settings = CommonInterface::SetMapSettings();
  settings.terrain = terrain;
  settings.topography_enabled = effective_terrain.topography_enabled;

  if ((effects & ToDisplaySettingEffects(DisplaySettingEffect::TERRAIN_CACHE)) !=
      0)
    if (auto *map = UIGlobals::GetMapIfActive(); map != nullptr)
      map->FlushCaches();
}

static bool
GetOrientationGlobal(DisplaySettingKey key,
                     DisplaySettingValue &value) noexcept
{
  if (key == Key::CRUISE_ORIENTATION)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_orientation.cruise_orientation));
  else if (key == Key::CIRCLING_ORIENTATION)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_orientation.circling_orientation));
  else if (key == Key::CIRCLING_ZOOM)
    value = DisplaySettingValue::Boolean(
      global_orientation.circle_zoom_enabled);
  else if (key == Key::MAP_SHIFT_BIAS)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_orientation.map_shift_bias));
  else if (key == Key::GLIDER_SCREEN_POSITION)
    value = DisplaySettingValue::Integer(
      global_orientation.glider_screen_position);
  else
    return false;

  return true;
}

static bool
SetOrientation(OrientationBundle &bundle, DisplaySettingKey key,
               DisplaySettingValue value) noexcept
{
  if (key == Key::CRUISE_ORIENTATION)
    bundle.cruise_orientation = static_cast<MapOrientation>(value.value);
  else if (key == Key::CIRCLING_ORIENTATION)
    bundle.circling_orientation = static_cast<MapOrientation>(value.value);
  else if (key == Key::CIRCLING_ZOOM)
    bundle.circle_zoom_enabled = value.AsBoolean();
  else if (key == Key::MAP_SHIFT_BIAS)
    bundle.map_shift_bias = static_cast<MapShiftBias>(value.value);
  else if (key == Key::GLIDER_SCREEN_POSITION)
    bundle.glider_screen_position = value.value;
  else
    return false;

  return true;
}

static bool
SetOrientationGlobal(DisplaySettingKey key,
                     DisplaySettingValue value) noexcept
{
  return SetOrientation(global_orientation, key, value);
}

static bool
SetOrientationEffective(DisplaySettingKey key,
                        DisplaySettingValue value) noexcept
{
  return SetOrientation(effective_orientation, key, value);
}

static void
ApplyOrientation(DisplaySettingEffects) noexcept
{
  auto &settings = CommonInterface::SetMapSettings();
  settings.cruise_orientation = effective_orientation.cruise_orientation;
  settings.circling_orientation = effective_orientation.circling_orientation;
  settings.circle_zoom_enabled = effective_orientation.circle_zoom_enabled;
  settings.map_shift_bias = effective_orientation.map_shift_bias;
  settings.glider_screen_position =
    effective_orientation.glider_screen_position;
}

} // namespace

void
RegisterTerrainOrientationDisplaySettings() noexcept
{
  using namespace DisplaySettingRuntime;

  Register(DisplaySettingGroup::TERRAIN, {
    LoadGlobal, GetTerrainGlobal, SetTerrainGlobal,
    SetTerrainEffective, ApplyTerrain,
  });
  Register(DisplaySettingGroup::ORIENTATION, {
    LoadGlobal, GetOrientationGlobal, SetOrientationGlobal,
    SetOrientationEffective, ApplyOrientation,
  });
}
