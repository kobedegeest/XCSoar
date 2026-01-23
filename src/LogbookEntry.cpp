// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LogbookEntry.hpp"

#include <cstdio>

std::string
LogbookEntry::GetIGCFilename() const noexcept
{
  char filename[32];
  snprintf(filename, sizeof(filename), "%04u%02u%02uT%02u%02u.igc",
           flight.date.year, flight.date.month, flight.date.day,
           flight.start_time.hour, flight.start_time.minute);
  return std::string(filename);
}
