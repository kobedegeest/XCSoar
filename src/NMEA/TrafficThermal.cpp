// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "TrafficThermal.hpp"

#include <algorithm>
#include <cassert>

void
TrafficThermalSource::Clear() noexcept
{
  thermal.location = GeoPoint::Invalid();
  thermal.ground_height = 0;
  thermal.lift_rate = 0;
  thermal.time = TimeStamp::Undefined();

  cluster_serial = 0;
  aircraft_count = 0;
  active_aircraft_count = 0;
  mean_observed_altitude = 0;
  first_seen = TimeStamp::Undefined();
  last_seen = TimeStamp::Undefined();
  active = false;
}

void
TrafficThermalInfo::Clear() noexcept
{
  sources.clear();
}

TrafficThermalSource *
TrafficThermalInfo::FindBySerial(std::uint32_t serial) noexcept
{
  for (auto &source : sources)
    if (source.cluster_serial == serial)
      return &source;

  return nullptr;
}

const TrafficThermalSource *
TrafficThermalInfo::FindBySerial(std::uint32_t serial) const noexcept
{
  for (const auto &source : sources)
    if (source.cluster_serial == serial)
      return &source;

  return nullptr;
}

bool
TrafficThermalInfo::RemoveBySerial(std::uint32_t serial) noexcept
{
  for (unsigned i = 0; i < sources.size(); ++i)
    if (sources[i].cluster_serial == serial) {
      sources.remove(i);
      return true;
    }

  return false;
}

static bool
IsOlder(const TrafficThermalSource &a,
        const TrafficThermalSource &b) noexcept
{
  if (a.first_seen != b.first_seen)
    return a.first_seen < b.first_seen;

  return a.cluster_serial < b.cluster_serial;
}

TrafficThermalSource &
TrafficThermalInfo::AllocateSource(std::uint32_t serial) noexcept
{
  if (auto *source = FindBySerial(serial))
    return *source;

  TrafficThermalSource *source;
  if (!sources.full())
    source = &sources.append();
  else {
    auto oldest = std::min_element(sources.begin(), sources.end(), IsOlder);
    assert(oldest != sources.end());
    source = &*oldest;
  }

  source->Clear();
  source->cluster_serial = serial;
  return *source;
}
