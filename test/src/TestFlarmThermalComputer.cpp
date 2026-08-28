// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Computer/FlarmThermalComputer.hpp"
#include "TestUtil.hpp"

#include <chrono>

static void
TestTrafficThermalAllocation()
{
  TrafficThermalInfo info;
  info.Clear();

  auto &first = info.AllocateSource(1);
  first.first_seen = TimeStamp{std::chrono::seconds{1}};
  first.last_seen = first.first_seen;

  ok1(info.FindBySerial(1) == &first);
  ok1(&info.AllocateSource(1) == &first);

  for (unsigned i = 2; i <= TrafficThermalInfo::MAX_SOURCES; ++i) {
    auto &source = info.AllocateSource(i);
    source.first_seen = TimeStamp{std::chrono::seconds{i}};
    source.last_seen = source.first_seen;
  }

  auto &replacement = info.AllocateSource(TrafficThermalInfo::MAX_SOURCES + 1);
  ok1(info.sources.size() == TrafficThermalInfo::MAX_SOURCES);
  ok1(info.FindBySerial(1) == nullptr);
  ok1(replacement.cluster_serial == TrafficThermalInfo::MAX_SOURCES + 1);
}

static void
TestComputerReset()
{
  TrafficThermalInfo info;
  info.Clear();
  info.AllocateSource(42);

  FlarmThermalComputer computer;
  computer.Reset(info);

  ok1(info.sources.empty());

  /* TODO(issue #832): add detector qualification and clustering cases. */
}

int
main()
{
  plan_tests(6);

  TestTrafficThermalAllocation();
  TestComputerReset();

  return exit_status();
}
