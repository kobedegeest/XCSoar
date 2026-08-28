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
  static constexpr FloatDuration MIN_SAMPLE_INTERVAL{0.125};

  /**
   * Keep enough samples for a 30 second window at substantially more than
   * the normal FLARM reporting rate.  Duplicate and very high-rate samples
   * are coalesced, so this is a hard memory bound rather than an assumption
   * that updates arrive at one hertz.
   */
  static constexpr int MAX_HISTORY = 256;
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
