// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Computer/FlarmThermalComputer.hpp"
#include "Geo/Math.hpp"
#include "Geo/SpeedVector.hpp"
#include "MapWindow/TrafficThermalVisibility.hpp"
#include "FakeLogFile.hpp"
#include "TestUtil.hpp"

#include <chrono>
#include <cmath>

using namespace std::chrono;

static constexpr GeoPoint TEST_CENTRE = {
  Angle::Degrees(7), Angle::Degrees(45),
};
static constexpr double TEST_OWNSHIP_ALTITUDE = 900;

static GeoPoint
AdjustCoreToAltitude(GeoPoint centre, double altitude,
                     double reference_altitude,
                     double geometry_climb_rate,
                     const SpeedVector &wind)
{
  if (wind.IsZero())
    return centre;

  const double height_delta = altitude - reference_altitude;
  const Angle bearing = height_delta >= 0
    ? wind.bearing.Reciprocal()
    : wind.bearing;
  return FindLatitudeLongitude(centre, bearing,
                               wind.norm * std::abs(height_delta) /
                               geometry_climb_rate);
}

static void
TestTrafficThermalAllocation()
{
  TrafficThermalInfo info;
  info.Clear();

  auto &first = info.AllocateSource(1);
  first.first_seen = TimeStamp{seconds{1}};
  first.last_seen = first.first_seen;

  ok1(info.FindBySerial(1) == &first);
  ok1(&info.AllocateSource(1) == &first);

  for (unsigned i = 2; i <= TrafficThermalInfo::MAX_SOURCES; ++i) {
    auto &source = info.AllocateSource(i);
    source.first_seen = TimeStamp{seconds{i}};
    source.last_seen = source.first_seen;
  }

  auto &replacement = info.AllocateSource(TrafficThermalInfo::MAX_SOURCES + 1);
  ok1(info.sources.size() == TrafficThermalInfo::MAX_SOURCES);
  ok1(info.FindBySerial(1) == nullptr);
  ok1(replacement.cluster_serial == TrafficThermalInfo::MAX_SOURCES + 1);
  ok1(info.RemoveBySerial(replacement.cluster_serial));
  ok1(info.sources.size() == TrafficThermalInfo::MAX_SOURCES - 1);
}

static FlarmTraffic &
AppendTraffic(TrafficList &list, FlarmId id, TimeStamp time,
              GeoPoint centre, double climb_rate, bool circling=true,
              double radius=100)
{
  FlarmTraffic traffic{};
  traffic.Clear();
  traffic.id = id;
  traffic.id_type = FlarmTraffic::IdType::FLARM;
  traffic.source = FlarmTraffic::SourceType::FLARM;
  traffic.type = FlarmTraffic::AircraftType::GLIDER;
  traffic.valid.Update(time);
  traffic.location_available = true;
  traffic.altitude_available = true;
  traffic.speed_received = true;
  traffic.speed = 20;
  traffic.track_received = true;

  const double elapsed = time.ToDuration().count();
  const double turn = circling ? elapsed * 12 : 0;
  traffic.location = FindLatitudeLongitude(centre, Angle::Degrees(turn),
                                           radius);
  traffic.track = Angle::Degrees(circling ? turn + 90 : 90).AsBearing();
  traffic.altitude = 1000 + elapsed * climb_rate;
  traffic.relative_altitude =
    double(traffic.altitude) - TEST_OWNSHIP_ALTITUDE;
  traffic.climb_rate_avg30s = climb_rate;
  traffic.climb_rate_avg30s_available = elapsed >= 31;

  list.list.append(traffic);
  return list.list.back();
}

static FlarmTraffic &
AppendTrafficWithGeometry(TrafficList &list, FlarmId id, TimeStamp time,
                          GeoPoint centre, double reported_climb_rate,
                          double geometry_climb_rate,
                          const SpeedVector &geometry_wind,
                          double ownship_altitude,
                          bool circling=true, double radius=100,
                          double pressure_altitude_offset=0)
{
  auto &traffic = AppendTraffic(list, id, time, centre,
                                reported_climb_rate, circling, radius);
  const double elapsed = time.ToDuration().count();
  const double geometry_altitude = 1000 + elapsed * geometry_climb_rate;
  const double turn = circling ? elapsed * 12 : 0;
  const GeoPoint altitude_core = AdjustCoreToAltitude(
    centre, geometry_altitude, 1000, geometry_climb_rate, geometry_wind);

  traffic.location = FindLatitudeLongitude(altitude_core,
                                           Angle::Degrees(turn), radius);
  traffic.altitude = geometry_altitude + pressure_altitude_offset;
  traffic.relative_altitude = geometry_altitude - ownship_altitude;
  traffic.climb_rate_avg30s = reported_climb_rate;
  return traffic;
}

static void
RunSingleSequence(FlarmThermalComputer &computer, TrafficThermalInfo &info,
                  FlarmId id, double climb_rate, bool circling=true,
                  double radius=100,
                  FlarmTraffic::SourceType source=
                    FlarmTraffic::SourceType::FLARM)
{
  for (unsigned i = 1; i <= 31; ++i) {
    TrafficList list{};
    list.Clear();
    auto &traffic = AppendTraffic(list, id, TimeStamp{seconds{i}}, TEST_CENTRE,
                                  climb_rate, circling, radius);
    traffic.source = source;
    computer.Process(list, TimeStamp{seconds{i}}, TEST_OWNSHIP_ALTITUDE,
                     SpeedVector::Zero(), nullptr, info);
    if (i == 30)
      ok1(info.sources.empty());
  }
}

static void
TestQualificationAndLifecycle()
{
  TrafficThermalInfo info;
  info.Clear();
  FlarmThermalComputer computer;
  computer.Reset(info);

  RunSingleSequence(computer, info, FlarmId::FromValue(1), 1.5);
  ok1(info.sources.size() == 1);
  ok1(info.sources.front().aircraft_count == 1);
  ok1(info.sources.front().active_aircraft_count == 1);
  ok1(info.sources.front().active);
  ok1(equals(info.sources.front().thermal.lift_rate, 1.5));
  ok1(equals(info.sources.front().thermal.ground_height, 0));

  TrafficList duplicate{};
  duplicate.Clear();
  AppendTraffic(duplicate, FlarmId::FromValue(1), TimeStamp{seconds{31}},
                TEST_CENTRE, 1.5);
  computer.Process(duplicate, TimeStamp{seconds{31}},
                   TEST_OWNSHIP_ALTITUDE, SpeedVector::Zero(), nullptr,
                   info);
  ok1(info.sources.size() == 1);
  ok1(info.sources.front().aircraft_count == 1);

  TrafficList empty{};
  empty.Clear();
  computer.Process(empty, TimeStamp{seconds{42}}, TEST_OWNSHIP_ALTITUDE,
                   SpeedVector::Zero(), nullptr, info);
  ok1(!info.sources.front().active);
  ok1(info.sources.front().active_aircraft_count == 0);
  ok1(equals(info.sources.front().thermal.lift_rate, 1.5));

  computer.Process(empty, TimeStamp{seconds{20}}, TEST_OWNSHIP_ALTITUDE,
                   SpeedVector::Zero(), nullptr, info);
  ok1(info.sources.empty());
}

static void
TestRejectedTracks()
{
  TrafficThermalInfo weak_info;
  weak_info.Clear();
  FlarmThermalComputer weak;
  weak.Reset(weak_info);
  RunSingleSequence(weak, weak_info, FlarmId::FromValue(2), 0.4);
  ok1(weak_info.sources.empty());

  TrafficThermalInfo straight_info;
  straight_info.Clear();
  FlarmThermalComputer straight;
  straight.Reset(straight_info);
  RunSingleSequence(straight, straight_info, FlarmId::FromValue(3), 1.5,
                    false);
  ok1(straight_info.sources.empty());

  TrafficThermalInfo spread_info;
  spread_info.Clear();
  FlarmThermalComputer spread;
  spread.Reset(spread_info);
  RunSingleSequence(spread, spread_info, FlarmId::FromValue(4), 1.5, true,
                    600);
  ok1(spread_info.sources.empty());

  TrafficThermalInfo online_info;
  online_info.Clear();
  FlarmThermalComputer online;
  online.Reset(online_info);
  RunSingleSequence(online, online_info, FlarmId::FromValue(5), 1.5, true,
                    100, FlarmTraffic::SourceType::OGN);
  ok1(online_info.sources.empty());
}

static void
TestGrouping()
{
  TrafficThermalInfo info;
  info.Clear();
  FlarmThermalComputer computer;
  computer.Reset(info);

  const GeoPoint second_centre =
    FindLatitudeLongitude(TEST_CENTRE, Angle::Degrees(90), 200);
  for (unsigned i = 1; i <= 31; ++i) {
    TrafficList list{};
    list.Clear();
    AppendTraffic(list, FlarmId::FromValue(10), TimeStamp{seconds{i}},
                  TEST_CENTRE, 1);
    AppendTraffic(list, FlarmId::FromValue(11), TimeStamp{seconds{i}},
                  second_centre, 2);
    computer.Process(list, TimeStamp{seconds{i}}, TEST_OWNSHIP_ALTITUDE,
                     SpeedVector::Zero(), nullptr, info);
  }

  ok1(info.sources.size() == 1);
  ok1(info.sources.front().aircraft_count == 2);
  ok1(info.sources.front().active_aircraft_count == 2);
  ok1(equals(info.sources.front().thermal.lift_rate, 1.5));

  for (unsigned i = 32; i <= 42; ++i) {
    TrafficList only_second{};
    only_second.Clear();
    AppendTraffic(only_second, FlarmId::FromValue(11),
                  TimeStamp{seconds{i}}, second_centre, 2);
    computer.Process(only_second, TimeStamp{seconds{i}},
                     TEST_OWNSHIP_ALTITUDE, SpeedVector::Zero(), nullptr,
                     info);
  }
  ok1(info.sources.front().aircraft_count == 2);
  ok1(info.sources.front().active_aircraft_count == 1);
  ok1(equals(info.sources.front().thermal.lift_rate, 2));
}

static void
TestSeparateThermals()
{
  TrafficThermalInfo info;
  info.Clear();
  FlarmThermalComputer computer;
  computer.Reset(info);

  const GeoPoint distant_centre =
    FindLatitudeLongitude(TEST_CENTRE, Angle::Degrees(90), 1200);
  for (unsigned i = 1; i <= 31; ++i) {
    TrafficList list{};
    list.Clear();
    AppendTraffic(list, FlarmId::FromValue(20), TimeStamp{seconds{i}},
                  TEST_CENTRE, 1.5);
    AppendTraffic(list, FlarmId::FromValue(21), TimeStamp{seconds{i}},
                  distant_centre, 1.5);
    computer.Process(list, TimeStamp{seconds{i}}, TEST_OWNSHIP_ALTITUDE,
                     SpeedVector::Zero(), nullptr, info);
  }

  ok1(info.sources.size() == 2);
}

static void
TestStraightDepartureFreezesSource()
{
  TrafficThermalInfo info;
  info.Clear();
  FlarmThermalComputer computer;
  computer.Reset(info);

  RunSingleSequence(computer, info, FlarmId::FromValue(30), 1.5);
  ok1(info.sources.front().active);
  const GeoPoint qualified_location =
    info.sources.front().CalculateAdjustedLocation(TEST_OWNSHIP_ALTITUDE);

  for (unsigned i = 32; i <= 33; ++i) {
    TrafficList list{};
    list.Clear();
    AppendTraffic(list, FlarmId::FromValue(30), TimeStamp{seconds{i}},
                  TEST_CENTRE, 1.5, false);
    computer.Process(list, TimeStamp{seconds{i}}, TEST_OWNSHIP_ALTITUDE,
                     SpeedVector::Zero(), nullptr, info);
  }

  ok1(!info.sources.front().active);
  ok1(info.sources.front().active_aircraft_count == 0);
  ok1(qualified_location.DistanceS(
        info.sources.front().CalculateAdjustedLocation(
          TEST_OWNSHIP_ALTITUDE)) < 50);

  TrafficList straight{};
  straight.Clear();
  AppendTraffic(straight, FlarmId::FromValue(30), TimeStamp{seconds{34}},
                TEST_CENTRE, 1.5, false);
  computer.Process(straight, TimeStamp{seconds{34}},
                   TEST_OWNSHIP_ALTITUDE, SpeedVector::Zero(), nullptr,
                   info);
  ok1(info.sources.size() == 1);
}

static void
TestStableGeometryAndAltitudeDatum()
{
  TrafficThermalInfo info;
  info.Clear();
  FlarmThermalComputer computer;
  computer.Reset(info);

  const SpeedVector qualification_wind{Angle::Degrees(70), 10};
  for (unsigned i = 1; i <= 31; ++i) {
    TrafficList list{};
    list.Clear();
    AppendTrafficWithGeometry(list, FlarmId::FromValue(31),
                              TimeStamp{seconds{i}}, TEST_CENTRE,
                              1.5, 1.5, qualification_wind,
                              TEST_OWNSHIP_ALTITUDE, true, 100, 400);
    computer.Process(list, TimeStamp{seconds{i}},
                     TEST_OWNSHIP_ALTITUDE, qualification_wind, nullptr,
                     info);
  }

  ok1(info.sources.size() == 1);
  ok1(info.sources.front().mean_observed_altitude < 1200);
  ok1(equals(info.sources.front().geometry_lift_rate, 1.5));
  ok1(equals(info.sources.front().geometry_wind.norm, 10));

  const GeoPoint display_at_900 =
    info.sources.front().CalculateAdjustedLocation(900);
  const GeoPoint expected_at_900 = AdjustCoreToAltitude(
    TEST_CENTRE, 900, 1000, 1.5, qualification_wind);
  ok1(display_at_900.DistanceS(expected_at_900) < 50);

  const SpeedVector changed_wind{Angle::Degrees(250), 20};
  for (unsigned i = 32; i <= 35; ++i) {
    const double ownship_altitude = 900 + (i - 31) * 50;
    TrafficList list{};
    list.Clear();
    AppendTrafficWithGeometry(list, FlarmId::FromValue(31),
                              TimeStamp{seconds{i}}, TEST_CENTRE,
                              2.5, 1.5, qualification_wind,
                              ownship_altitude, true, 100, 400);
    computer.Process(list, TimeStamp{seconds{i}}, ownship_altitude,
                     changed_wind, nullptr, info);
  }

  ok1(equals(info.sources.front().thermal.lift_rate, 2.5));
  ok1(equals(info.sources.front().geometry_lift_rate, 1.5));
  ok1(equals(info.sources.front().geometry_wind.norm, 10));
  ok1(display_at_900.DistanceS(
        info.sources.front().CalculateAdjustedLocation(900)) < 50);

  const GeoPoint expected_at_1200 = AdjustCoreToAltitude(
    TEST_CENTRE, 1200, 1000, 1.5, qualification_wind);
  ok1(info.sources.front().CalculateAdjustedLocation(1200)
        .DistanceS(expected_at_1200) < 50);

  const GeoPoint active_location =
    info.sources.front().CalculateAdjustedLocation(1100);
  TrafficList empty{};
  empty.Clear();
  computer.Process(empty, TimeStamp{seconds{46}}, 1100, changed_wind,
                   nullptr, info);

  ok1(!info.sources.front().active);
  ok1(info.sources.front().thermal.lift_rate > 1.5 &&
      info.sources.front().thermal.lift_rate < 2.5);
  ok1(active_location.DistanceS(
        info.sources.front().CalculateAdjustedLocation(1100)) < 0.1);
  ok1(equals(info.sources.front().geometry_lift_rate, 1.5));
}

static void
TestTrafficThermalVisibility()
{
  ok1(TrafficThermalLayer::IsVisible(true, 4000));
  ok1(TrafficThermalLayer::IsVisible(true, 3999));
  ok1(!TrafficThermalLayer::IsVisible(true, 4001));
  ok1(!TrafficThermalLayer::IsVisible(false, 1000));
}

int
main()
{
  plan_tests(59);
  SetFakeLogFileQuiet(true);

  TestTrafficThermalAllocation();
  TestQualificationAndLifecycle();
  TestRejectedTracks();
  TestGrouping();
  TestSeparateThermals();
  TestStraightDepartureFreezesSource();
  TestStableGeometryAndAltitudeDatum();
  TestTrafficThermalVisibility();

  return exit_status();
}
