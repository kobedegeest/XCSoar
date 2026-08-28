// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "FlarmThermalComputer.hpp"

#include "Computer/ThermalBase.hpp"
#include "Geo/Flat/FlatPoint.hpp"
#include "Geo/Flat/FlatProjection.hpp"
#include "Geo/Math.hpp"
#include "Geo/SpeedVector.hpp"
#include "LogFile.hpp"

#include <algorithm>
#include <cmath>

using namespace FlarmThermalConstants;

FlarmThermalComputer::FlarmThermalComputer() noexcept
  :next_cluster_serial(1),
   last_process_time(TimeStamp::Undefined())
{
  targets.clear();
  clusters.clear();
}

static bool
IsEligibleTraffic(const FlarmTraffic &traffic) noexcept
{
  if (!traffic.valid ||
      traffic.source != FlarmTraffic::SourceType::FLARM ||
      !traffic.id.IsDefined() ||
      traffic.id_type == FlarmTraffic::IdType::RANDOM ||
      traffic.no_track ||
      !traffic.location_available || !traffic.location.Check() ||
      !traffic.altitude_available ||
      !std::isfinite(double(traffic.altitude)) ||
      !std::isfinite(static_cast<Angle>(traffic.track).Native()) ||
      traffic.IsPassive())
    return false;

  return traffic.type == FlarmTraffic::AircraftType::GLIDER ||
    traffic.type == FlarmTraffic::AircraftType::HANG_GLIDER ||
    traffic.type == FlarmTraffic::AircraftType::PARA_GLIDER;
}

void
FlarmThermalComputer::Reset(TrafficThermalInfo &output) noexcept
{
  targets.clear();
  clusters.clear();
  next_cluster_serial = 1;
  last_process_time = TimeStamp::Undefined();
  output.Clear();
}

FlarmThermalComputer::TargetState *
FlarmThermalComputer::FindTarget(FlarmId id) noexcept
{
  for (auto &target : targets)
    if (target.id == id)
      return &target;

  return nullptr;
}

void
FlarmThermalComputer::ResetTargetWindow(TargetState &target) noexcept
{
  target.samples.clear();
  target.assigned_cluster_serial = 0;
  target.qualified = false;
}

void
FlarmThermalComputer::DeactivateTarget(TargetState &target,
                                       const char *reason) noexcept
{
  if (auto *cluster = FindCluster(target.assigned_cluster_serial)) {
    LogDebug("FLARM thermal cluster={} target={:06X} exit={}",
             cluster->serial, target.id.Value(), reason);
    for (auto &contributor : cluster->contributors)
      if (contributor.id == target.id) {
        contributor.active = false;
        break;
      }
  }

  ResetTargetWindow(target);
}

FlarmThermalComputer::TargetState &
FlarmThermalComputer::AllocateTarget(FlarmId id,
                                     TrafficThermalInfo &output) noexcept
{
  (void)output;

  TargetState *target;
  if (!targets.full())
    target = &targets.append();
  else {
    target = &*std::min_element(targets.begin(), targets.end(),
                                [](const TargetState &a,
                                   const TargetState &b) {
                                  return a.last_seen < b.last_seen;
                                });
    DeactivateTarget(*target, "target-slot-replaced");
  }

  target->id = id;
  target->samples.clear();
  target->assigned_cluster_serial = 0;
  target->last_update.Clear();
  target->last_seen = TimeStamp::Undefined();
  target->qualified = false;
  return *target;
}

FlarmThermalComputer::ClusterState *
FlarmThermalComputer::FindCluster(std::uint32_t serial) noexcept
{
  if (serial == 0)
    return nullptr;

  for (auto &cluster : clusters)
    if (cluster.serial == serial)
      return &cluster;

  return nullptr;
}

FlarmThermalComputer::ClusterState &
FlarmThermalComputer::AllocateCluster(TimeStamp first_seen,
                                      double reference_altitude,
                                      TrafficThermalInfo &output) noexcept
{
  ClusterState *cluster;
  if (!clusters.full())
    cluster = &clusters.append();
  else {
    cluster = &*std::min_element(clusters.begin(), clusters.end(),
                                [](const ClusterState &a,
                                   const ClusterState &b) {
                                  if (a.closed != b.closed)
                                    return a.closed;

                                  if (a.first_seen != b.first_seen)
                                    return a.first_seen < b.first_seen;

                                  return a.serial < b.serial;
                                });

    const auto old_serial = cluster->serial;
    for (auto &target : targets)
      if (target.assigned_cluster_serial == old_serial)
        ResetTargetWindow(target);

    output.RemoveBySerial(old_serial);
  }

  do {
    cluster->serial = next_cluster_serial++;
  } while (cluster->serial == 0);

  cluster->contributors.clear();
  cluster->first_seen = first_seen;
  cluster->last_seen = first_seen;
  cluster->reference_altitude = reference_altitude;
  cluster->recent = false;
  cluster->closed = false;
  return *cluster;
}

static SpeedVector
MakeDriftPerAltitude(const SpeedVector &wind,
                     double geometry_lift_rate) noexcept
{
  return wind.IsNonZero() && geometry_lift_rate > 0
    ? SpeedVector(wind.bearing, wind.norm / geometry_lift_rate)
    : SpeedVector::Zero();
}

static GeoPoint
AdjustToAltitude(const GeoPoint location, double source_altitude,
                 double destination_altitude,
                 const SpeedVector &drift_per_altitude) noexcept
{
  const double height_delta = destination_altitude - source_altitude;
  if (drift_per_altitude.IsZero() || height_delta == 0)
    return location;

  const Angle bearing = height_delta > 0
    ? drift_per_altitude.bearing.Reciprocal()
    : drift_per_altitude.bearing;
  return FindLatitudeLongitude(location, bearing,
                               drift_per_altitude.norm *
                               std::abs(height_delta));
}

FlarmThermalComputer::CandidateResult
FlarmThermalComputer::BuildCandidate(const TargetState &target,
                                     const FlarmTraffic &traffic,
                                     double geometry_lift_rate,
                                     const SpeedVector &geometry_wind,
                                     const RasterTerrain *terrain,
                                     Candidate &candidate) const noexcept
{
  if (target.samples.size() < 2 ||
      !traffic.climb_rate_avg30s_available ||
      !std::isfinite(traffic.climb_rate_avg30s))
    return CandidateResult::INCOMPLETE_WINDOW;

  const auto &oldest = target.samples.front();
  const auto &newest = target.samples.back();
  if (newest.time - oldest.time < OBSERVATION_WINDOW)
    return CandidateResult::INCOMPLETE_WINDOW;

  const double climb_threshold = target.qualified
    ? EXIT_CLIMB_THRESHOLD
    : ENTER_CLIMB_THRESHOLD;
  if (traffic.climb_rate_avg30s < climb_threshold)
    return CandidateResult::WEAK_LIFT;

  double accumulated_turn = 0;
  for (unsigned i = 1; i < target.samples.size(); ++i)
    accumulated_turn +=
      (target.samples[i].track - target.samples[i - 1].track)
      .AsDelta().Absolute().Degrees();

  if (accumulated_turn < MIN_ACCUMULATED_TURN)
    return CandidateResult::INSUFFICIENT_TURN;

  if (target.qualified) {
    unsigned tail_begin = target.samples.size() - 1;
    while (tail_begin > 0 &&
           target.samples[tail_begin - 1].time >=
             newest.time - EXIT_TURN_WINDOW)
      --tail_begin;

    double recent_turn = 0;
    for (unsigned i = tail_begin + 1; i < target.samples.size(); ++i)
      recent_turn +=
        (target.samples[i].track - target.samples[i - 1].track)
        .AsDelta().Absolute().Degrees();

    const double recent_duration =
      (newest.time - target.samples[tail_begin].time).count();
    const auto &previous = target.samples[target.samples.size() - 2];
    const double current_duration = (newest.time - previous.time).count();
    const double current_turn =
      (newest.track - previous.track).AsDelta().Absolute().Degrees();

    if (!(recent_duration > 0) ||
        recent_turn / recent_duration < MIN_RECENT_TURN_RATE ||
        !(current_duration > 0) ||
        current_turn / current_duration < MIN_CURRENT_TURN_RATE)
      return CandidateResult::LEFT_CIRCLE;
  }

  const auto drift_per_altitude =
    MakeDriftPerAltitude(geometry_wind, geometry_lift_rate);

  const FlatProjection projection(newest.location);
  FlatPoint mean(0, 0);
  for (const auto &sample : target.samples)
    mean += projection.ProjectFloat(
      AdjustToAltitude(sample.location, sample.altitude, newest.altitude,
                       drift_per_altitude));

  mean = mean * (1. / target.samples.size());

  const double flat_scale = projection.GetApproximateScale();
  for (const auto &sample : target.samples) {
    const auto point = projection.ProjectFloat(
      AdjustToAltitude(sample.location, sample.altitude, newest.altitude,
                       drift_per_altitude));
    if (point.Distance(mean) * flat_scale > MAX_DRIFT_CORRECTED_RADIUS)
      return CandidateResult::EXCESSIVE_RADIUS;
  }

  candidate.id = target.id;
  candidate.centre = projection.Unproject(mean);
  candidate.first_seen = oldest.time;
  candidate.altitude = newest.altitude;
  candidate.climb_rate = traffic.climb_rate_avg30s;
  candidate.geometry_lift_rate = geometry_lift_rate;
  candidate.geometry_wind = geometry_wind;
  candidate.drift_per_altitude = drift_per_altitude;
  candidate.source.lift_rate = geometry_lift_rate;
  candidate.source.time = newest.time;
  EstimateThermalBase(terrain, candidate.centre, candidate.altitude,
                      geometry_lift_rate, geometry_wind,
                      candidate.source.location,
                      candidate.source.ground_height);
  return candidate.source.location.IsValid()
    ? CandidateResult::QUALIFIED
    : CandidateResult::INVALID_SOURCE;
}

FlarmThermalComputer::ClusterState *
FlarmThermalComputer::FindCompatibleCluster(
    const Candidate &candidate, TimeStamp now,
    const TrafficThermalInfo &output) noexcept
{
  ClusterState *best = nullptr;
  double best_distance = GROUPING_RADIUS;

  for (auto &cluster : clusters) {
    if (cluster.closed || now < cluster.last_seen ||
        now > cluster.last_seen + GROUPING_TIME_GAP)
      continue;

    bool already_contributed = false;
    for (const auto &contributor : cluster.contributors)
      if (contributor.id == candidate.id) {
        already_contributed = true;
        break;
      }

    if (cluster.contributors.full() && !already_contributed)
      continue;

    const auto *published = output.FindBySerial(cluster.serial);
    if (published == nullptr ||
        !published->reference_location.IsValid() ||
        !(published->geometry_lift_rate > 0))
      continue;

    const double comparison_altitude = std::max({
      candidate.altitude,
      candidate.source.ground_height,
      published->thermal.ground_height,
      published->mean_observed_altitude,
      published->reference_altitude,
    });
    const auto candidate_location =
      AdjustToAltitude(candidate.centre, candidate.altitude,
                       comparison_altitude,
                       candidate.drift_per_altitude);
    const auto cluster_location =
      published->CalculateAdjustedLocation(comparison_altitude);
    const double distance = candidate_location.DistanceS(cluster_location);
    if (distance <= best_distance) {
      best = &cluster;
      best_distance = distance;
    }
  }

  return best;
}

void
FlarmThermalComputer::UpdateContributor(ClusterState &cluster,
                                        TargetState &target,
                                        const Candidate &candidate,
                                        TimeStamp now) noexcept
{
  ContributorState *contributor = nullptr;
  for (auto &item : cluster.contributors)
    if (item.id == target.id) {
      contributor = &item;
      break;
    }

  if (contributor == nullptr) {
    if (cluster.contributors.full())
      return;

    contributor = &cluster.contributors.append();
    contributor->id = target.id;
    contributor->first_seen = candidate.first_seen;
    contributor->last_seen = now;
    contributor->last_value_time = now;
    contributor->latest_climb_rate = candidate.climb_rate;
    contributor->last_climb_rate = candidate.climb_rate;
    contributor->climb_integral = 0;
    contributor->encounter_duration = 0;
    contributor->encounter_average = candidate.climb_rate;
    contributor->last_altitude = candidate.altitude;
    contributor->altitude_integral = 0;
    contributor->altitude_duration = 0;
    contributor->mean_altitude = candidate.altitude;
    contributor->reference_altitude = candidate.altitude;
    contributor->geometry_lift_rate = candidate.geometry_lift_rate;
    contributor->geometry_wind = candidate.geometry_wind;
    contributor->drift_per_altitude = candidate.drift_per_altitude;
  } else if (now > contributor->last_value_time) {
    const auto elapsed = now - contributor->last_value_time;
    if (elapsed <= MAX_SAMPLE_GAP) {
      const double dt = elapsed.count();
      contributor->climb_integral +=
        (contributor->last_climb_rate + candidate.climb_rate) * 0.5 * dt;
      contributor->encounter_duration += dt;
      contributor->altitude_integral +=
        (contributor->last_altitude + candidate.altitude) * 0.5 * dt;
      contributor->altitude_duration += dt;
    }

    contributor->last_value_time = now;
    contributor->last_climb_rate = candidate.climb_rate;
    contributor->last_altitude = candidate.altitude;
    if (contributor->encounter_duration > 0)
      contributor->encounter_average =
        contributor->climb_integral / contributor->encounter_duration;
    else
      contributor->encounter_average = candidate.climb_rate;

    if (contributor->altitude_duration > 0)
      contributor->mean_altitude =
        contributor->altitude_integral / contributor->altitude_duration;
    else
      contributor->mean_altitude = candidate.altitude;
  }

  contributor->last_seen = now;
  contributor->latest_climb_rate = candidate.climb_rate;
  contributor->active = true;

  contributor->source = candidate.source;
  contributor->centre = candidate.centre;
  contributor->reference_altitude = candidate.altitude;

  LogDebug("FLARM thermal cluster={} target={:06X} state={} "
           "rolling={:.6f},{:.6f} contributor_ground={:.6f},{:.6f} "
           "altitude={:.1f} reporting_lift={:.2f} geometry_lift={:.2f} "
           "wind={:.1f}@{:.1f}",
           cluster.serial, target.id.Value(),
           target.qualified ? "update" : "qualified",
           candidate.centre.latitude.Degrees(),
           candidate.centre.longitude.Degrees(),
           candidate.source.location.latitude.Degrees(),
           candidate.source.location.longitude.Degrees(),
           candidate.altitude, candidate.climb_rate,
           contributor->geometry_lift_rate,
           contributor->geometry_wind.norm,
           contributor->geometry_wind.bearing.Degrees());

  target.assigned_cluster_serial = cluster.serial;
  target.qualified = true;
  cluster.last_seen = now;
  cluster.recent = false;
  cluster.closed = false;
}

static void
AddVector(const SpeedVector &vector, double &east, double &north) noexcept
{
  east += vector.norm * vector.bearing.sin();
  north += vector.norm * vector.bearing.cos();
}

static SpeedVector
AverageVector(double east, double north, double count) noexcept
{
  return count > 0
    ? SpeedVector(east / count, north / count)
    : SpeedVector::Zero();
}

void
FlarmThermalComputer::RecomputeCluster(ClusterState &cluster,
                                       TrafficThermalInfo &output) noexcept
{
  if (cluster.contributors.empty())
    return;

  unsigned active_count = 0;
  double active_lift = 0;
  double historical_lift = 0;
  double mean_altitude = 0;
  double geometry_lift = 0;
  double wind_east = 0;
  double wind_north = 0;
  double drift_east = 0;
  double drift_north = 0;
  double ground_height = 0;

  for (const auto &contributor : cluster.contributors)
    cluster.reference_altitude =
      std::max(cluster.reference_altitude,
               contributor.source.ground_height);

  const auto &first = cluster.contributors.front();
  const GeoPoint projection_centre =
    AdjustToAltitude(first.centre, first.reference_altitude,
                     cluster.reference_altitude,
                     first.drift_per_altitude);
  const FlatProjection projection(projection_centre);
  FlatPoint reference_location(0, 0);

  for (const auto &contributor : cluster.contributors) {
    if (contributor.active) {
      ++active_count;
      active_lift += contributor.latest_climb_rate;
    }

    historical_lift += contributor.encounter_average;
    mean_altitude += contributor.mean_altitude;
    geometry_lift += contributor.geometry_lift_rate;
    AddVector(contributor.geometry_wind, wind_east, wind_north);
    AddVector(contributor.drift_per_altitude, drift_east, drift_north);
    reference_location += projection.ProjectFloat(
      AdjustToAltitude(contributor.centre, contributor.reference_altitude,
                       cluster.reference_altitude,
                       contributor.drift_per_altitude));
    ground_height += contributor.source.ground_height;
  }

  const double count = cluster.contributors.size();
  auto &published = output.AllocateSource(cluster.serial);
  published.cluster_serial = cluster.serial;
  published.aircraft_count = cluster.contributors.size();
  published.active_aircraft_count = active_count;
  published.mean_observed_altitude = mean_altitude / count;
  published.first_seen = cluster.first_seen;
  published.last_seen = cluster.last_seen;
  published.active = active_count > 0;

  published.reference_location =
    projection.Unproject(reference_location * (1. / count));
  published.reference_altitude = cluster.reference_altitude;
  published.geometry_lift_rate = geometry_lift / count;
  published.geometry_wind =
    AverageVector(wind_east, wind_north, count);
  published.drift_per_altitude =
    AverageVector(drift_east, drift_north, count);

  published.thermal.ground_height = ground_height / count;
  published.thermal.location = AdjustToAltitude(
    published.reference_location, published.reference_altitude,
    published.thermal.ground_height, published.drift_per_altitude);
  published.thermal.lift_rate = active_count > 0
    ? active_lift / active_count
    : historical_lift / count;
  published.thermal.time = cluster.last_seen;

  LogDebug("FLARM thermal cluster={} ground={:.6f},{:.6f} "
           "reference={:.6f},{:.6f}@{:.1f} reporting_lift={:.2f} "
           "geometry_lift={:.2f} wind={:.1f}@{:.1f} active={}/{}",
           cluster.serial,
           published.thermal.location.latitude.Degrees(),
           published.thermal.location.longitude.Degrees(),
           published.reference_location.latitude.Degrees(),
           published.reference_location.longitude.Degrees(),
           published.reference_altitude,
           published.thermal.lift_rate,
           published.geometry_lift_rate,
           published.geometry_wind.norm,
           published.geometry_wind.bearing.Degrees(),
           active_count, cluster.contributors.size());
}

void
FlarmThermalComputer::UpdateLifecycle(TimeStamp now,
                                      TrafficThermalInfo &output) noexcept
{
  for (auto &cluster : clusters) {
    unsigned active_count = 0;
    for (auto &contributor : cluster.contributors) {
      if (contributor.active &&
          (now < contributor.last_seen ||
           now > contributor.last_seen + CONTRIBUTOR_TIMEOUT)) {
        contributor.active = false;
        if (auto *target = FindTarget(contributor.id);
            target != nullptr &&
            target->assigned_cluster_serial == cluster.serial) {
          target->assigned_cluster_serial = 0;
          target->qualified = false;
        }
      }

      if (contributor.active)
        ++active_count;
    }

    if (active_count > 0) {
      cluster.recent = false;
      cluster.closed = false;
    } else if (now >= cluster.last_seen &&
               now <= cluster.last_seen + GROUPING_TIME_GAP) {
      cluster.recent = true;
      cluster.closed = false;
    } else {
      cluster.recent = false;
      cluster.closed = true;
    }

    RecomputeCluster(cluster, output);
  }
}

static bool
ClustersOverlapInTime(TimeStamp a_first, TimeStamp a_last,
                      TimeStamp b_first, TimeStamp b_last) noexcept
{
  if (a_last < b_first)
    return b_first - a_last <= GROUPING_TIME_GAP;

  if (b_last < a_first)
    return a_first - b_last <= GROUPING_TIME_GAP;

  return true;
}

void
FlarmThermalComputer::MergeClusters(unsigned keep_index,
                                    unsigned remove_index,
                                    TrafficThermalInfo &output) noexcept
{
  auto &keep = clusters[keep_index];
  auto &remove = clusters[remove_index];
  const auto remove_serial = remove.serial;

  for (const auto &incoming : remove.contributors) {
    ContributorState *existing = nullptr;
    for (auto &current : keep.contributors)
      if (current.id == incoming.id) {
        existing = &current;
        break;
      }

    if (existing == nullptr) {
      if (!keep.contributors.full())
        keep.contributors.append(incoming);
      continue;
    }

    const double combined_climb_duration =
      existing->encounter_duration + incoming.encounter_duration;
    if (combined_climb_duration > 0) {
      existing->climb_integral += incoming.climb_integral;
      existing->encounter_duration = combined_climb_duration;
      existing->encounter_average =
        existing->climb_integral / combined_climb_duration;
    } else {
      existing->encounter_average =
        (existing->encounter_average + incoming.encounter_average) * 0.5;
    }

    const double combined_altitude_duration =
      existing->altitude_duration + incoming.altitude_duration;
    if (combined_altitude_duration > 0) {
      existing->altitude_integral += incoming.altitude_integral;
      existing->altitude_duration = combined_altitude_duration;
      existing->mean_altitude =
        existing->altitude_integral / combined_altitude_duration;
    } else {
      existing->mean_altitude =
        (existing->mean_altitude + incoming.mean_altitude) * 0.5;
    }

    existing->first_seen = std::min(existing->first_seen,
                                    incoming.first_seen);
    if (incoming.last_seen > existing->last_seen) {
      existing->last_seen = incoming.last_seen;
      existing->last_value_time = incoming.last_value_time;
      existing->latest_climb_rate = incoming.latest_climb_rate;
      existing->last_climb_rate = incoming.last_climb_rate;
      existing->last_altitude = incoming.last_altitude;
      existing->centre = incoming.centre;
      existing->source = incoming.source;
      existing->reference_altitude = incoming.reference_altitude;
      existing->geometry_lift_rate = incoming.geometry_lift_rate;
      existing->geometry_wind = incoming.geometry_wind;
      existing->drift_per_altitude = incoming.drift_per_altitude;
    }
    existing->active = existing->active || incoming.active;
  }

  keep.first_seen = std::min(keep.first_seen, remove.first_seen);
  keep.last_seen = std::max(keep.last_seen, remove.last_seen);
  keep.closed = keep.closed && remove.closed;
  keep.recent = !keep.closed && (keep.recent || remove.recent);

  for (auto &target : targets)
    if (target.assigned_cluster_serial == remove_serial)
      target.assigned_cluster_serial = keep.serial;

  const auto keep_serial = keep.serial;
  output.RemoveBySerial(remove_serial);
  clusters.remove(remove_index);
  if (auto *kept = FindCluster(keep_serial))
    RecomputeCluster(*kept, output);
}

void
FlarmThermalComputer::MergeCompatibleClusters(
    TimeStamp now,
    TrafficThermalInfo &output) noexcept
{
  (void)now;

  bool merged;
  do {
    merged = false;
    for (unsigned i = 0; i < clusters.size() && !merged; ++i) {
      if (clusters[i].closed)
        continue;

      const auto *a = output.FindBySerial(clusters[i].serial);
      if (a == nullptr || !(a->geometry_lift_rate > 0))
        continue;

      for (unsigned j = i + 1; j < clusters.size(); ++j) {
        if (clusters[j].closed ||
            !ClustersOverlapInTime(clusters[i].first_seen,
                                   clusters[i].last_seen,
                                   clusters[j].first_seen,
                                   clusters[j].last_seen))
          continue;

        const auto *b = output.FindBySerial(clusters[j].serial);
        if (b == nullptr || !(b->geometry_lift_rate > 0))
          continue;

        const double comparison_altitude = std::max({
          a->thermal.ground_height,
          b->thermal.ground_height,
          a->mean_observed_altitude,
          b->mean_observed_altitude,
          a->reference_altitude,
          b->reference_altitude,
        });
        const auto a_location = a->CalculateAdjustedLocation(
          comparison_altitude);
        const auto b_location = b->CalculateAdjustedLocation(
          comparison_altitude);
        if (a_location.DistanceS(b_location) > GROUPING_RADIUS)
          continue;

        unsigned keep = i;
        unsigned remove = j;
        if (clusters[j].first_seen < clusters[i].first_seen ||
            (clusters[j].first_seen == clusters[i].first_seen &&
             clusters[j].serial < clusters[i].serial)) {
          keep = j;
          remove = i;
        }

        MergeClusters(keep, remove, output);
        merged = true;
        break;
      }
    }
  } while (merged);
}

void
FlarmThermalComputer::Process(const TrafficList &traffic, TimeStamp now,
                               double ownship_altitude,
                               const SpeedVector &wind,
                              const RasterTerrain *terrain,
                              TrafficThermalInfo &output) noexcept
{
  if (!now.IsDefined() || !std::isfinite(ownship_altitude) ||
      (last_process_time.IsDefined() && now < last_process_time)) {
    Reset(output);
    if (!now.IsDefined() || !std::isfinite(ownship_altitude))
      return;
  }

  last_process_time = now;
  UpdateLifecycle(now, output);

  for (const auto &item : traffic.list) {
    TargetState *target = FindTarget(item.id);
    if (!IsEligibleTraffic(item)) {
      if (target != nullptr)
        DeactivateTarget(*target, "ineligible-traffic");
      continue;
    }

    if (target == nullptr)
      target = &AllocateTarget(item.id, output);

    if (target->last_update && item.valid == target->last_update)
      continue;

    const bool traffic_time_reversal =
      target->last_update && item.valid < target->last_update;
    if (traffic_time_reversal ||
        (target->last_seen.IsDefined() &&
         now > target->last_seen + MAX_SAMPLE_GAP))
      ResetTargetWindow(*target);

    target->last_update = item.valid;
    target->last_seen = now;

    if (!target->samples.empty() &&
        now > target->samples.back().time &&
        now < target->samples.back().time + MIN_SAMPLE_INTERVAL)
      continue;

    if (target->samples.full() && target->samples.back().time != now)
      target->samples.remove(0);

    auto &sample = !target->samples.empty() &&
      target->samples.back().time == now
      ? target->samples.back()
      : target->samples.append();
    sample.time = now;
    sample.location = item.location;
    // Physical FLARM altitude is relative to ownship.  Rebuild it in the
    // navigation/QNH-or-GPS MSL datum used by terrain and map projection,
    // instead of using FlarmTraffic::altitude's pressure-first datum.
    sample.altitude = item.absolute_altitude
      ? double(item.altitude)
      : ownship_altitude + double(item.relative_altitude);
    if (!std::isfinite(sample.altitude)) {
      DeactivateTarget(*target, "invalid-navigation-altitude");
      continue;
    }
    sample.track = item.track;
    sample.climb_rate = item.climb_rate_avg30s_available
      ? item.climb_rate_avg30s
      : 0;

    while (target->samples.size() > 1 &&
           target->samples.front().time < now - OBSERVATION_WINDOW)
      target->samples.remove(0);

    double geometry_lift_rate = item.climb_rate_avg30s;
    SpeedVector geometry_wind = wind;
    if (const auto *assigned = FindCluster(
          target->assigned_cluster_serial))
      for (const auto &contributor : assigned->contributors)
        if (contributor.id == target->id) {
          geometry_lift_rate = contributor.geometry_lift_rate;
          geometry_wind = contributor.geometry_wind;
          break;
        }

    Candidate candidate;
    const auto candidate_result =
      BuildCandidate(*target, item, geometry_lift_rate, geometry_wind,
                     terrain, candidate);
    if (candidate_result != CandidateResult::QUALIFIED) {
      if (target->qualified &&
          candidate_result != CandidateResult::INCOMPLETE_WINDOW) {
        const char *reason;
        switch (candidate_result) {
        case CandidateResult::WEAK_LIFT:
          reason = "weak-lift";
          break;
        case CandidateResult::INSUFFICIENT_TURN:
          reason = "insufficient-turn";
          break;
        case CandidateResult::LEFT_CIRCLE:
          reason = "left-circle";
          break;
        case CandidateResult::EXCESSIVE_RADIUS:
          reason = "excessive-radius";
          break;
        case CandidateResult::INVALID_SOURCE:
          reason = "invalid-source";
          break;
        case CandidateResult::QUALIFIED:
        case CandidateResult::INCOMPLETE_WINDOW:
          reason = "incomplete-window";
          break;
        }

        DeactivateTarget(*target, reason);
      }

      continue;
    }

    ClusterState *cluster = FindCluster(target->assigned_cluster_serial);
    if (cluster == nullptr || cluster->closed) {
      cluster = FindCompatibleCluster(candidate, now, output);
      if (cluster == nullptr)
        cluster = &AllocateCluster(candidate.first_seen,
                                   candidate.altitude, output);
    }

    UpdateContributor(*cluster, *target, candidate, now);
    RecomputeCluster(*cluster, output);
  }

  UpdateLifecycle(now, output);
  MergeCompatibleClusters(now, output);

  for (unsigned i = 0; i < targets.size();) {
    if (targets[i].last_seen.IsDefined() &&
        now > targets[i].last_seen + CONTRIBUTOR_TIMEOUT) {
      DeactivateTarget(targets[i], "contributor-timeout");
      targets.quick_remove(i);
    } else
      ++i;
  }
}
