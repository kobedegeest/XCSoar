// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LogbookGlue.hpp"
#include "FlightParser.hpp"
#include "LogbookEntry.hpp"
#include "io/FileLineReader.hpp"
#include "LocalPath.hpp"
#include "system/Path.hpp"

#include <algorithm>

std::vector<LogbookEntry>
EnumerateFlights() noexcept
{
  std::vector<LogbookEntry> result;

  try {
    // Open the flights.log file
    AllocatedPath logbook_path = LocalPath(_T("flights.log"));
    FileLineReaderA reader(logbook_path);

    FlightParser parser(reader);
    FlightInfo flight;

    // Read all flights from the logbook
    while (parser.Read(flight)) {
      LogbookEntry entry;
      entry.flight = flight;
      entry.comment = flight.comment;
      entry.igc_file = AllocatedPath::Null();

      result.push_back(entry);
    }
  } catch (...) {
    // If file doesn't exist or can't be read, return empty list
  }

  // Sort flights by date in descending order (most recent first)
  std::sort(result.begin(), result.end(), [](const LogbookEntry &a, const LogbookEntry &b) {
    return a.flight.date > b.flight.date;
  });

  return result;
}
