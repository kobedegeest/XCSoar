// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "FLARM/List.hpp"
#include "Math/Angle.hpp"
#include "NMEA/TrafficThermal.hpp"
#include "time/Stamp.hpp"
#include "util/TrivialArray.hxx"

#include <cstdint>

class RasterTerrain;
struct SpeedVector;

/** Initial tunable values for the FLARM thermal detector. */
namespace FlarmThermalConstants {
static constexpr FloatDuration OBSERVATION_WINDOW{30};
static constexpr FloatDuration MAX_SAMPLE_GAP{5};
static constexpr FloatDuration CONTRIBUTOR_TIMEOUT{10};
static constexpr FloatDuration GROUPING_TIME_GAP{120};

static constexpr double ENTER_CLIMB_THRESHOLD = 0.5;
static constexpr double EXIT_CLIMB_THRESHOLD = 0.3;
static constexpr double MAX_DRIFT_CORRECTED_RADIUS = 500;
static constexpr double GROUPING_RADIUS = 500;
static constexpr double MIN_ACCUMULATED_TURN = 270;

/**
 * Temporary bounded history for the first detector skeleton.  Replace this
 * sample-count bound with a rate-aware time-based ring when FLARM update-rate
 * requirements are established.
 */
static constexpr unsigned MAX_SAMPLE_COUNT = 64;
}

/**
 * Calculation state for detecting and grouping thermal climbs reported by
 * physical FLARM traffic.
 *
 * The detector state intentionally stays outside DerivedInfo; only the
 * bounded, aggregate TrafficThermalInfo snapshot crosses the blackboard
 * boundary.
 */
class FlarmThermalComputer {
  struct Sample {
    TimeStamp time;
    GeoPoint location;
    double altitude;
    Angle track;
    double climb_rate;
  };

  struct TargetState {
    FlarmId id;
    TrivialArray<Sample, FlarmThermalConstants::MAX_SAMPLE_COUNT> samples;
    std::uint32_t assigned_cluster_serial;
    TimeStamp last_seen;
    bool qualified;
  };

  struct ContributorState {
    FlarmId id;
    GeoPoint centre;
    TimeStamp last_seen;
    double latest_climb_rate;
    double encounter_average;
    bool active;
  };

  struct ClusterState {
    std::uint32_t serial;
    TrivialArray<ContributorState, TrafficList::DEVICE_MAX_COUNT>
      contributors;
    TimeStamp first_seen;
    TimeStamp last_seen;
    bool recent;
    bool closed;
  };

  TrivialArray<TargetState, TrafficList::DEVICE_MAX_COUNT> targets;
  TrivialArray<ClusterState, TrafficThermalInfo::MAX_SOURCES> clusters;
  std::uint32_t next_cluster_serial;

public:
  /** Reset detector state and published FLARM thermal history. */
  void Reset(TrafficThermalInfo &output) noexcept;

  /**
   * Process one calculation snapshot.
   *
   * TODO(issue #832): implement target qualification, cluster assignment,
   * equal-weight aggregation, merging, and recent/closed lifecycle handling.
   */
  void Process(const TrafficList &traffic, TimeStamp now,
               const SpeedVector &wind, const RasterTerrain *terrain,
               TrafficThermalInfo &output) noexcept;
};
