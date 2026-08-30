// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapElementConfig.hpp"
#include "Keys.hpp"
#include "Map.hpp"
#include "util/StringFormat.hpp"

#include <cstddef>

static bool
MakeKey(char (&buffer)[64], unsigned index, const char *suffix) noexcept
{
  const int n = StringFormat(buffer, sizeof(buffer), "MapElementSet%u%s",
                             index, suffix);
  return n >= 0 && static_cast<std::size_t>(n) < sizeof(buffer);
}

template<typename T>
static void
LoadValue(const ProfileMap &map, unsigned index, const char *suffix,
          T &value) noexcept
{
  char key[64];
  if (MakeKey(key, index, suffix))
    map.Get(key, value);
}

template<typename T>
static void
LoadEnum(const ProfileMap &map, unsigned index, const char *suffix,
         T &value) noexcept
{
  char key[64];
  if (MakeKey(key, index, suffix))
    map.GetEnum(key, value);
}

template<typename T>
static void
SaveValue(ProfileMap &map, unsigned index, const char *suffix,
          const T &value) noexcept
{
  char key[64];
  if (MakeKey(key, index, suffix))
    map.Set(key, value);
}

template<typename T>
static void
SaveEnum(ProfileMap &map, unsigned index, const char *suffix,
         T value) noexcept
{
  char key[64];
  if (MakeKey(key, index, suffix))
    map.SetEnum(key, value);
}

static void
LoadLegacyValues(const ProfileMap &map, MapElementSet &set) noexcept
{
  map.GetEnum(ProfileKeys::FinalGlideBarDisplayMode,
              set.final_glide_bar_display_mode);
  map.Get(ProfileKeys::EnableFinalGlideBarMC0,
          set.final_glide_bar_mc0_enabled);
  map.Get(ProfileKeys::TrailDrift, set.trail.wind_drift_enabled);
  map.Get(ProfileKeys::SnailWidthScale, set.trail.scaling_enabled);
  map.GetEnum(ProfileKeys::SnailType, set.trail.type);
  map.GetEnum(ProfileKeys::SnailTrail, set.trail.length);
  map.Get(ProfileKeys::DistanceRingsEnabled, set.distance_rings_enabled);
  map.GetEnum(ProfileKeys::DisplayTrackBearing, set.display_ground_track);
  map.Get(ProfileKeys::EnableFLARMMap, set.show_flarm_on_map);
  map.Get(ProfileKeys::EnableFLARMGauge, set.flarm_gauge_enabled);
  map.GetEnum(ProfileKeys::FinalGlideTerrain, set.final_glide_terrain);
  map.Get(ProfileKeys::EnableThermalProfile, set.show_thermal_profile);
  map.Get(ProfileKeys::EnableVarioBar, set.vario_bar_enabled);
}

void
Profile::Load(const ProfileMap &map, MapElementSettings &settings)
{
  MapElementSet legacy = settings.sets[MapElementSettings::SET_CIRCLING];
  LoadLegacyValues(map, legacy);

  for (unsigned i = 0; i < MapElementSettings::MAX_SETS; ++i) {
    auto name = settings.sets[i].name;
    MapElementSet &set = settings.sets[i];
    set = legacy;
    set.name = name;

    if (i >= MapElementSettings::PREASSIGNED_SETS) {
      LoadValue(map, i, "Name", set.name);
      if (set.name.empty())
        set.name.Format("AUX-%u", i - MapElementSettings::PREASSIGNED_SETS + 1);
    }

    LoadEnum(map, i, "FinalGlideBarDisplayMode",
             set.final_glide_bar_display_mode);
    LoadValue(map, i, "FinalGlideBarMC0Enabled",
              set.final_glide_bar_mc0_enabled);
    LoadValue(map, i, "TrailWindDriftEnabled", set.trail.wind_drift_enabled);
    LoadValue(map, i, "TrailScalingEnabled", set.trail.scaling_enabled);
    LoadEnum(map, i, "TrailType", set.trail.type);
    LoadEnum(map, i, "TrailLength", set.trail.length);
    LoadValue(map, i, "DistanceRingsEnabled", set.distance_rings_enabled);
    LoadEnum(map, i, "DisplayGroundTrack", set.display_ground_track);
    LoadValue(map, i, "ShowFLARMOnMap", set.show_flarm_on_map);
    LoadValue(map, i, "FLARMGaugeEnabled", set.flarm_gauge_enabled);
    LoadEnum(map, i, "FinalGlideTerrain", set.final_glide_terrain);
    LoadValue(map, i, "ShowThermalProfile", set.show_thermal_profile);
    LoadValue(map, i, "VarioBarEnabled", set.vario_bar_enabled);
  }
}

void
Profile::Save(ProfileMap &map, const MapElementSet &set, unsigned index)
{
  if (index >= MapElementSettings::MAX_SETS)
    return;

  if (index >= MapElementSettings::PREASSIGNED_SETS)
    SaveValue(map, index, "Name", set.name.c_str());

  SaveEnum(map, index, "FinalGlideBarDisplayMode",
           set.final_glide_bar_display_mode);
  SaveValue(map, index, "FinalGlideBarMC0Enabled",
            set.final_glide_bar_mc0_enabled);
  SaveValue(map, index, "TrailWindDriftEnabled", set.trail.wind_drift_enabled);
  SaveValue(map, index, "TrailScalingEnabled", set.trail.scaling_enabled);
  SaveEnum(map, index, "TrailType", set.trail.type);
  SaveEnum(map, index, "TrailLength", set.trail.length);
  SaveValue(map, index, "DistanceRingsEnabled", set.distance_rings_enabled);
  SaveEnum(map, index, "DisplayGroundTrack", set.display_ground_track);
  SaveValue(map, index, "ShowFLARMOnMap", set.show_flarm_on_map);
  SaveValue(map, index, "FLARMGaugeEnabled", set.flarm_gauge_enabled);
  SaveEnum(map, index, "FinalGlideTerrain", set.final_glide_terrain);
  SaveValue(map, index, "ShowThermalProfile", set.show_thermal_profile);
  SaveValue(map, index, "VarioBarEnabled", set.vario_bar_enabled);
}
