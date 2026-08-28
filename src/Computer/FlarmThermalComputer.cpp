// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "FlarmThermalComputer.hpp"

void
FlarmThermalComputer::Reset(TrafficThermalInfo &output) noexcept
{
  targets.clear();
  clusters.clear();
  next_cluster_serial = 1;
  output.Clear();
}

void
FlarmThermalComputer::Process(const TrafficList &traffic, TimeStamp now,
                               const SpeedVector &wind,
                               const RasterTerrain *terrain,
                               TrafficThermalInfo &output) noexcept
{
  (void)traffic;
  (void)now;
  (void)wind;
  (void)terrain;
  (void)output;

  /*
   * Skeleton only: keep this entry point in the lifecycle now so the full
   * detector can be added without moving work into MapWindow later.
   *
   * TODO(issue #832): perform the bounded processing loop described in
   * ISSUE_832_IMPLEMENTATION_PLAN.md.
   */
}
