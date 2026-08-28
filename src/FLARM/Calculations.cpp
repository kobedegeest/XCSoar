// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Calculations.hpp"

FlarmCalculations::AverageResult
FlarmCalculations::Average30sWithSpan(FlarmId id, TimeStamp time,
                                      double altitude) noexcept
{
  ClimbAverageCalculator &item = averageCalculatorMap[id];
  return item.GetAverageWithSpan(time, altitude, std::chrono::seconds{30});
}

double
FlarmCalculations::Average30s(FlarmId id, TimeStamp time,
                              double altitude) noexcept
{
  return Average30sWithSpan(id, time, altitude).average;
}

void
FlarmCalculations::CleanUp(TimeStamp now) noexcept
{
  constexpr FloatDuration MAX_AGE = std::chrono::minutes{1};

  // Iterate through ClimbAverageCalculators and remove expired ones
  for (auto it = averageCalculatorMap.begin(),
       it_end = averageCalculatorMap.end(); it != it_end;)
    if (it->second.Expired(now, MAX_AGE))
      it = averageCalculatorMap.erase(it);
    else
      ++it;
}
