// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapElementSettings.hpp"
#include "Gauge/TrafficSettings.hpp"
#include "Language/Language.hpp"

void
MapElementSettings::SetDefaults() noexcept
{
  MapSettings map_defaults;
  map_defaults.SetDefaults();

  TrafficSettings traffic_defaults;
  traffic_defaults.SetDefaults();

  for (auto &set : sets) {
    set.name.clear();
    set.final_glide_bar_display_mode =
      map_defaults.final_glide_bar_display_mode;
    set.final_glide_bar_mc0_enabled =
      map_defaults.final_glide_bar_mc0_enabled;
    set.trail = map_defaults.trail;
    set.distance_rings_enabled = map_defaults.distance_rings_enabled;
    set.display_ground_track = map_defaults.display_ground_track;
    set.show_flarm_on_map = map_defaults.show_flarm_on_map;
    set.flarm_gauge_enabled = traffic_defaults.enable_gauge;
    set.final_glide_terrain =
      FeaturesSettings::FinalGlideTerrain::TERRAIN_LINE;
    set.show_thermal_profile = map_defaults.show_thermal_profile;
    set.vario_bar_enabled = map_defaults.vario_bar_enabled;
  }

  sets[SET_CIRCLING].name = N_("Circling");
  sets[SET_CRUISE].name = N_("Cruise");
  sets[SET_FINAL_GLIDE].name = N_("FinalGlide");

  for (unsigned i = PREASSIGNED_SETS; i < MAX_SETS; ++i)
    sets[i].name.Format("AUX-%u", i - PREASSIGNED_SETS + 1);
}
