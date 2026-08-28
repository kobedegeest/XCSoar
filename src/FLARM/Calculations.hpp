// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Id.hpp"
#include "Computer/ClimbAverageCalculator.hpp"

#include <map>

class TimeStamp;
struct TrafficList;

class FlarmCalculations
{
private:
  struct AverageCalculatorState {
    ClimbAverageCalculator calculator;
    TimeStamp last_time;
  };

  typedef std::map<FlarmId, AverageCalculatorState> AverageCalculatorMap;
  AverageCalculatorMap averageCalculatorMap;

public:
  using AverageResult = ClimbAverageResult;

  /**
   * Calculates the 30-second average and exposes the actual sample span.
   */
  [[nodiscard]]
  AverageResult Average30sWithSpan(FlarmId flarmId, TimeStamp curTime,
                                   double curAltitude) noexcept;

  double Average30s(FlarmId flarmId, TimeStamp curTime,
                    double curAltitude) noexcept;

  void Reset(FlarmId flarmId) noexcept;
  void ResetMissing(const TrafficList &traffic) noexcept;
  void Clear() noexcept;
  void CleanUp(TimeStamp now) noexcept;
};
