// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "system/Path.hpp"

#include <vector>

struct FlightInfo;

/**
 * Find IGC files that match a specific flight.
 *
 * Scans the logs directory for .igc files and performs time-based matching:
 * - Reads HFDTE record for the IGC file date
 * - Reads first B-record to get IGC start time
 * - Match if IGC start time falls within [flight.start_time - 5 minutes, flight.end_time]
 * - Returns XCSoar's file first (identifiable by "XCS" manufacturer code), then others sorted by filename
 *
 * @param flight The flight to find matching IGC files for
 * @return vector of AllocatedPath objects pointing to matching IGC files,
 *         sorted with XCSoar files first, then by filename
 */
std::vector<AllocatedPath>
FindMatchingIGCFiles(const FlightInfo &flight) noexcept;
