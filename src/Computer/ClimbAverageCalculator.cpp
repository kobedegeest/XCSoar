// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ClimbAverageCalculator.hpp"

#include <cassert>

void
ClimbAverageCalculator::Reset()
{
  newestValIndex = -1;
  for (int i = 0; i < MAX_HISTORY; i++)
    history[i].Reset();
}

double
ClimbAverageCalculator::GetAverageWithSpan(TimeStamp time, double altitude,
                                           FloatDuration average_time) noexcept
{
  assert(average_time.count() <= MAX_HISTORY);

  if (!time.IsDefined())
    return {0, FloatDuration::zero()};

  /* A backwards timestamp starts a new observation window. */
  if (newestValIndex >= 0 && history[newestValIndex].IsDefined() &&
      time < history[newestValIndex].time)
    Reset();

  int bestHistory;

  // Don't update newestValIndex if the time didn't move forward
  if (newestValIndex < 0 ||
      !history[newestValIndex].IsDefined() ||
      time > history[newestValIndex].time)
    newestValIndex = newestValIndex < MAX_HISTORY - 1 ? newestValIndex + 1 : 0;

  // add the new sample
  history[newestValIndex] = HistoryItem(time, altitude);

  // initially bestHistory is the current...
  bestHistory = newestValIndex;

  // now run through the history and find the best sample
  // for average period within the average time period
  for (int i = 0; i < MAX_HISTORY; i++) {
    if (!history[i].IsDefined())
      continue;

    // outside the period -> skip value
    if (history[i].time + average_time < time)
      continue;

    // is the sample older (and therefore better) than the current found ?
    if (history[i].time < history[bestHistory].time)
      bestHistory = i;
  }

  // calculate the average !
  const auto time_span = time - history[bestHistory].time;
  const auto average = bestHistory != newestValIndex && time_span.count() > 0
    ? (altitude - history[bestHistory].altitude) / time_span.count()
    : 0;

  return {average, time_span};
}

double
ClimbAverageCalculator::GetAverage(TimeStamp time, double altitude,
                                   FloatDuration average_time) noexcept
{
  return GetAverageWithSpan(time, altitude, average_time).average;
}

bool
ClimbAverageCalculator::Expired(TimeStamp now,
                                FloatDuration max_age) const noexcept
{
  if (newestValIndex < 0)
    return true;

  auto item = history[newestValIndex];
  if (!item.IsDefined())
    return true;

  return now < item.time || now > item.time + max_age;
}
