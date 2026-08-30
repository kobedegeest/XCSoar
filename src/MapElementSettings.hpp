// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "MapSettings.hpp"
#include "Computer/Settings.hpp"
#include "util/StaticString.hxx"

#include <type_traits>

struct MapElementSet {
  StaticString<32u> name;

  FinalGlideBarDisplayMode final_glide_bar_display_mode;
  bool final_glide_bar_mc0_enabled;

  TrailSettings trail;
  bool distance_rings_enabled;
  DisplayGroundTrack display_ground_track;
  bool show_flarm_on_map;
  bool flarm_gauge_enabled;
  FeaturesSettings::FinalGlideTerrain final_glide_terrain;
  bool show_thermal_profile;
  bool vario_bar_enabled;
};

static_assert(std::is_trivial<MapElementSet>::value, "type is not trivial");

struct MapElementSettings {
  enum SetIndex {
    SET_CIRCLING,
    SET_CRUISE,
    SET_FINAL_GLIDE,
  };

  static constexpr unsigned MAX_SETS = 8;
  static constexpr unsigned PREASSIGNED_SETS = 3;

  MapElementSet sets[MAX_SETS];

  void SetDefaults() noexcept;
};

static_assert(std::is_trivial<MapElementSettings>::value,
              "type is not trivial");
