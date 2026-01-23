// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <string_view>

class Path;
struct BrokenDateTime;

namespace LogbookWriter {

/**
 * Append a comment record to the flights.log file.
 *
 * The comment is associated with the flight that started at the given datetime.
 * Format: YYYY-MM-DDTHH:MM:SS comment <text>
 *
 * @param logbook_path Path to the flights.log file
 * @param flight_start_time The start datetime of the flight to comment on
 * @param comment_text The comment text to append
 * @return true on success, false on I/O error
 */
bool
WriteComment(Path logbook_path, const BrokenDateTime &flight_start_time,
             std::string_view comment_text) noexcept;

} // namespace LogbookWriter
