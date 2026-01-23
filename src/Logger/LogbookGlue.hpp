// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "LogbookEntry.hpp"

#include <vector>

/**
 * Enumerate all flights from the logbook.
 *
 * Reads flights.log using the extended FlightParser to include comments,
 * resolves the default XCSoar-generated IGC file path by date-based matching,
 * and returns flights sorted descending by date.
 *
 * @return vector of LogbookEntry objects sorted by date (most recent first),
 *         with total flight time calculated
 */
std::vector<LogbookEntry>
EnumerateFlights() noexcept;
