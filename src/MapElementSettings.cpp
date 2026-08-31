// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapElementSettings.hpp"
#include "Asset.hpp"
#include "Gauge/TrafficSettings.hpp"
#include "Language/Language.hpp"

void
MapElementSettings::SetDefaults() noexcept
{
  MapSettings map_defaults;
  map_defaults.SetDefaults();

  TrafficSettings traffic_defaults;
  traffic_defaults.SetDefaults();

#ifdef KOBO
  static constexpr bool default_show_menu_button = true;
#else
  static constexpr bool default_show_menu_button = false;
#endif
  const bool default_show_quickmenu_button = HasTouchScreen();

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
    set.detour_cost_markers_enabled =
      map_defaults.detour_cost_markers_enabled;
    set.wind_arrow_style = map_defaults.wind_arrow_style;
    set.online_traffic_map_mode = map_defaults.online_traffic_map_mode;
    set.thermal_assistant_position =
      ThermalAssistantPosition::BOTTOM_LEFT_AVOID_IB;
    set.turn_back_marker_enabled = false;
    set.show_menu_button = default_show_menu_button;
    set.show_zoom_button = set.show_menu_button;
    set.show_quickmenu_button = default_show_quickmenu_button;
#ifdef HAVE_TRACKING
    set.cloud_show_thermals = true;
#endif
#ifdef HAVE_HTTP
    set.enable_thermal_information_map = false;
#endif
  }

  sets[SET_CIRCLING].name = N_("Circling");
  sets[SET_CRUISE].name = N_("Cruise");
  sets[SET_FINAL_GLIDE].name = N_("FinalGlide");

  for (unsigned i = PREASSIGNED_SETS; i < MAX_SETS; ++i)
    sets[i].name.Format("AUX-%u", i - PREASSIGNED_SETS + 1);
}
