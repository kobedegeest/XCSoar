// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "FlightInfo.hpp"
#include "system/Path.hpp"

#include <chrono>
#include <string>

/**
 * Represents an entry in the flight logbook, combining flight information
 * with associated metadata like IGC file path and comments.
 */
struct LogbookEntry {
  FlightInfo flight;
  AllocatedPath igc_file;
  std::string comment;

  /**
   * Calculate the flight duration from start and end times.
   * 
   * @return duration as std::chrono::system_clock::duration
   */
  std::chrono::system_clock::duration GetDuration() const noexcept {
    return flight.Duration();
  }

  /**
   * Generate an IGC filename based on the flight's date and time.
   * Uses the format: YYYYMMDDTHHMM.igc (ISO 8601 compact format with T separator)
   * 
   * @return formatted IGC filename string
   */
  std::string GetIGCFilename() const noexcept;

  /**
   * Check if this logbook entry has an associated IGC file.
   *
   * @return true if igc_file is valid (non-empty)
   */
  bool HasIGCFile() const noexcept {
    return igc_file != nullptr;
  }
};
