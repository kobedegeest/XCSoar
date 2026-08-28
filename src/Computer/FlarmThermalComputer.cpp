// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "FlarmThermalComputer.hpp"

#include "Computer/ThermalBase.hpp"
#include "Geo/Flat/FlatPoint.hpp"
#include "Geo/Flat/FlatProjection.hpp"
#include "Geo/Math.hpp"
#include "Geo/SpeedVector.hpp"

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
FlarmThermalComputer::DeactivateTarget(TargetState &target) noexcept
{
  if (auto *cluster = FindCluster(target.assigned_cluster_serial))
    for (auto &contributor : cluster->contributors)
      if (contributor.id == target.id) {
        contributor.active = false;
        break;
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
    DeactivateTarget(*target);
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
  cluster->recent = false;
  cluster->closed = false;
  return *cluster;
}

static GeoPoint
AdjustToTime(const GeoPoint location, TimeStamp sample_time,
             TimeStamp reference_time, const SpeedVector &wind) noexcept
{
  if (wind.IsZero())
    return location;

  const auto age = reference_time - sample_time;
  return FindLatitudeLongitude(location, wind.bearing.Reciprocal(),
                               wind.norm * age.count());
}

bool
FlarmThermalComputer::BuildCandidate(const TargetState &target,
                                     const FlarmTraffic &traffic,
                                     const SpeedVector &wind,
                                     const RasterTerrain *terrain,
                                     Candidate &candidate) const noexcept
{
  if (target.samples.size() < 2 ||
      !traffic.climb_rate_avg30s_available ||
      !std::isfinite(traffic.climb_rate_avg30s))
    return false;

  const auto &oldest = target.samples.front();
  const auto &newest = target.samples.back();
  if (newest.time - oldest.time < OBSERVATION_WINDOW)
    return false;

  const double climb_threshold = target.qualified
    ? EXIT_CLIMB_THRESHOLD
    : ENTER_CLIMB_THRESHOLD;
  if (traffic.climb_rate_avg30s < climb_threshold)
    return false;

  double accumulated_turn = 0;
  for (unsigned i = 1; i < target.samples.size(); ++i)
    accumulated_turn +=
      (target.samples[i].track - target.samples[i - 1].track)
      .AsDelta().Absolute().Degrees();

  if (accumulated_turn < MIN_ACCUMULATED_TURN)
    return false;

  const FlatProjection projection(newest.location);
  FlatPoint mean(0, 0);
  for (const auto &sample : target.samples)
    mean += projection.ProjectFloat(AdjustToTime(sample.location, sample.time,
                                                 newest.time, wind));

  mean = mean * (1. / target.samples.size());

  const double flat_scale = projection.GetApproximateScale();
  for (const auto &sample : target.samples) {
    const auto point = projection.ProjectFloat(
      AdjustToTime(sample.location, sample.time, newest.time, wind));
    if (point.Distance(mean) * flat_scale > MAX_DRIFT_CORRECTED_RADIUS)
      return false;
  }

  candidate.id = target.id;
  candidate.centre = projection.Unproject(mean);
  candidate.first_seen = oldest.time;
  candidate.altitude = newest.altitude;
  candidate.climb_rate = traffic.climb_rate_avg30s;
  candidate.source.lift_rate = candidate.climb_rate;
  candidate.source.time = newest.time;
  EstimateThermalBase(terrain, candidate.centre, candidate.altitude,
                      candidate.climb_rate, wind,
                      candidate.source.location,
                      candidate.source.ground_height);
  return candidate.source.location.IsValid();
}

static GeoPoint
AdjustedSourceLocation(const ThermalSource &source, double altitude,
                       const SpeedVector &wind) noexcept
{
  return wind.IsNonZero()
    ? source.CalculateAdjustedLocation(altitude, wind)
    : source.location;
}

FlarmThermalComputer::ClusterState *
FlarmThermalComputer::FindCompatibleCluster(
    const Candidate &candidate, TimeStamp now, const SpeedVector &wind,
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
        !published->thermal.location.IsValid() ||
        !(published->thermal.lift_rate > 0))
      continue;

    const double comparison_altitude = std::max({
      candidate.altitude,
      candidate.source.ground_height,
      published->thermal.ground_height,
      published->mean_observed_altitude,
    });
    const auto candidate_location =
      AdjustedSourceLocation(candidate.source, comparison_altitude, wind);
    const auto cluster_location =
      AdjustedSourceLocation(published->thermal, comparison_altitude, wind);
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
                                        TimeStamp now,
                                        const SpeedVector &wind,
                                        const RasterTerrain *terrain) noexcept
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

  contributor->centre = candidate.centre;
  contributor->last_seen = now;
  contributor->latest_climb_rate = candidate.climb_rate;
  contributor->active = true;

  contributor->source.lift_rate = contributor->encounter_average;
  contributor->source.time = now;
  EstimateThermalBase(terrain, candidate.centre, candidate.altitude,
                      contributor->encounter_average, wind,
                      contributor->source.location,
                      contributor->source.ground_height);

  target.assigned_cluster_serial = cluster.serial;
  target.qualified = true;
  cluster.last_seen = now;
  cluster.recent = false;
  cluster.closed = false;
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

  const GeoPoint projection_centre =
    cluster.contributors.front().source.location;
  const FlatProjection projection(projection_centre);
  FlatPoint ground_location(0, 0);
  double ground_height = 0;

  for (const auto &contributor : cluster.contributors) {
    if (contributor.active) {
      ++active_count;
      active_lift += contributor.latest_climb_rate;
    }

    historical_lift += contributor.encounter_average;
    mean_altitude += contributor.mean_altitude;
    ground_location += projection.ProjectFloat(contributor.source.location);
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

  published.thermal.location =
    projection.Unproject(ground_location * (1. / count));
  published.thermal.ground_height = ground_height / count;
  published.thermal.lift_rate = active_count > 0
    ? active_lift / active_count
    : historical_lift / count;
  published.thermal.time = cluster.last_seen;
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
    TimeStamp now, const SpeedVector &wind,
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
      if (a == nullptr || !(a->thermal.lift_rate > 0))
        continue;

      for (unsigned j = i + 1; j < clusters.size(); ++j) {
        if (clusters[j].closed ||
            !ClustersOverlapInTime(clusters[i].first_seen,
                                   clusters[i].last_seen,
                                   clusters[j].first_seen,
                                   clusters[j].last_seen))
          continue;

        const auto *b = output.FindBySerial(clusters[j].serial);
        if (b == nullptr || !(b->thermal.lift_rate > 0))
          continue;

        const double comparison_altitude = std::max({
          a->thermal.ground_height,
          b->thermal.ground_height,
          a->mean_observed_altitude,
          b->mean_observed_altitude,
        });
        const auto a_location =
          AdjustedSourceLocation(a->thermal, comparison_altitude, wind);
        const auto b_location =
          AdjustedSourceLocation(b->thermal, comparison_altitude, wind);
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
                              const SpeedVector &wind,
                              const RasterTerrain *terrain,
                              TrafficThermalInfo &output) noexcept
{
  if (!now.IsDefined() ||
      (last_process_time.IsDefined() && now < last_process_time)) {
    Reset(output);
    if (!now.IsDefined())
      return;
  }

  last_process_time = now;
  UpdateLifecycle(now, output);

  for (const auto &item : traffic.list) {
    TargetState *target = FindTarget(item.id);
    if (!IsEligibleTraffic(item)) {
      if (target != nullptr)
        DeactivateTarget(*target);
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
    sample.altitude = item.altitude;
    sample.track = item.track;
    sample.climb_rate = item.climb_rate_avg30s_available
      ? item.climb_rate_avg30s
      : 0;

    while (target->samples.size() > 1 &&
           target->samples.front().time < now - OBSERVATION_WINDOW)
      target->samples.remove(0);

    Candidate candidate;
    if (!BuildCandidate(*target, item, wind, terrain, candidate))
      continue;

    ClusterState *cluster = FindCluster(target->assigned_cluster_serial);
    if (cluster == nullptr || cluster->closed) {
      cluster = FindCompatibleCluster(candidate, now, wind, output);
      if (cluster == nullptr)
        cluster = &AllocateCluster(candidate.first_seen, output);
    }

    UpdateContributor(*cluster, *target, candidate, now, wind, terrain);
    RecomputeCluster(*cluster, output);
  }

  UpdateLifecycle(now, output);
  MergeCompatibleClusters(now, wind, output);

  for (unsigned i = 0; i < targets.size();) {
    if (targets[i].last_seen.IsDefined() &&
        now > targets[i].last_seen + CONTRIBUTOR_TIMEOUT) {
      DeactivateTarget(targets[i]);
      targets.quick_remove(i);
    } else
      ++i;
  }
}
