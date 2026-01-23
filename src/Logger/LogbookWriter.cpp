// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LogbookWriter.hpp"
#include "time/BrokenDateTime.hpp"
#include "io/FileOutputStream.hxx"
#include "system/Path.hpp"

#include <cstdio>

bool
LogbookWriter::WriteComment(Path logbook_path,
                            const BrokenDateTime &flight_start_time,
                            std::string_view comment_text) noexcept
{
  try {
    FileOutputStream file(logbook_path);

    // Format: YYYY-MM-DDTHH:MM:SS comment <text>
    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer),
                       "%04u-%02u-%02uT%02u:%02u:%02u comment %.*s\n",
                       flight_start_time.year, flight_start_time.month,
                       flight_start_time.day, flight_start_time.hour,
                       flight_start_time.minute, flight_start_time.second,
                       (int)comment_text.size(), comment_text.data());

    if (len < 0 || len >= (int)sizeof(buffer))
      return false;

    file.Write(buffer);
    file.Commit();
    return true;
  } catch (...) {
    return false;
  }
}
