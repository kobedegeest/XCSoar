// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "time/Stamp.hpp"

struct ClimbAverageResult {
  /** Average climb rate over the selected history window. */
  double average;

  /** Actual time span covered by the selected history window. */
  FloatDuration time_span;

  constexpr bool IsComplete(FloatDuration required) const noexcept {
    return time_span >= required;
  }
};

class ClimbAverageCalculator
{
  /* TODO(issue #832): size this ring from the supported update rate. */
  static constexpr int MAX_HISTORY = 40;
  struct HistoryItem
  {
    TimeStamp time;
    double altitude;

    HistoryItem() = default;

    constexpr HistoryItem(TimeStamp _time,
                          double _altitude) noexcept
      :time(_time), altitude(_altitude) {}

    bool IsDefined() const {
      return time.IsDefined();
    }

    void Reset() {
      time = TimeStamp::Undefined();
    }
  };

  HistoryItem history[MAX_HISTORY];
  int newestValIndex;

public:
  /**
   * Calculates the average and reports the actual history span used.
   *
   * This is deliberately separate from GetAverage() so existing consumers
   * can keep their current API while new consumers can reject short windows.
   */
  [[nodiscard]]
  ClimbAverageResult GetAverageWithSpan(TimeStamp time, double altitude,
                                        FloatDuration average_time) noexcept;

  double GetAverage(TimeStamp time, double altitude,
                    FloatDuration average_time) noexcept;
  void Reset();
  bool Expired(TimeStamp now,
               FloatDuration max_age) const noexcept;
};
