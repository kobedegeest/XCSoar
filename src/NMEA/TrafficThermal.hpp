// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "NMEA/ThermalLocator.hpp"
#include "util/TrivialArray.hxx"

#include <cstdint>
#include <type_traits>

/** A calculated thermal marker contributed by physical FLARM traffic. */
struct TrafficThermalSource {
  /** Wind-adjustable ground source shared with the own-thermal renderer. */
  ThermalSource thermal;

  /** Stable identity used by the calculation state to update this marker. */
  std::uint32_t cluster_serial;

  /** Number of unique aircraft contributing to this encounter. */
  unsigned aircraft_count;

  /** Number of contributors that are currently qualified and active. */
  unsigned active_aircraft_count;

  /** Mean observed altitude across the contributing aircraft. */
  double mean_observed_altitude;

  /** First and most recent observation in the cluster encounter. */
  TimeStamp first_seen;
  TimeStamp last_seen;

  /** True while at least one contributor is current. */
  bool active;

  void Clear() noexcept;
};

static_assert(std::is_trivially_copyable_v<TrafficThermalSource>,
              "type is not trivially copyable");

/** Bounded blackboard output for aggregated FLARM thermal markers. */
struct TrafficThermalInfo {
  static constexpr unsigned MAX_SOURCES = 20;

  TrivialArray<TrafficThermalSource, MAX_SOURCES> sources;

  void Clear() noexcept;

  /** Find a published source by its stable calculation serial. */
  [[nodiscard]]
  TrafficThermalSource *FindBySerial(std::uint32_t serial) noexcept;

  [[nodiscard]]
  const TrafficThermalSource *FindBySerial(std::uint32_t serial) const noexcept;

  /**
   * Find an existing source or allocate a slot, replacing the oldest source
   * when the bounded history is full.
   */
  [[nodiscard]]
  TrafficThermalSource &AllocateSource(std::uint32_t serial) noexcept;
};

static_assert(std::is_trivially_copyable_v<TrafficThermalInfo>,
              "type is not trivially copyable");
