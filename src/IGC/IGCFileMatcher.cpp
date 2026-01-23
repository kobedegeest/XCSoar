// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "IGCFileMatcher.hpp"
#include "IGCParser.hpp"
#include "FlightInfo.hpp"
#include "time/BrokenDateTime.hpp"
#include "io/FileLineReader.hpp"
#include "LocalPath.hpp"
#include "system/FileUtil.hpp"
#include "system/Path.hpp"

#include <algorithm>
#include <cstring>

/**
 * Visitor for collecting IGC files during directory traversal
 */
class IGCFileFinder : public File::Visitor {
public:
  std::vector<AllocatedPath> files;

  void Visit(Path path, Path filename) override {
    files.push_back(AllocatedPath(path));
  }
};

/**
 * Check if an IGC file matches a flight based on time criteria
 */
static bool
IGCFileMatches(const AllocatedPath &igc_path,
               const FlightInfo &flight,
               bool &is_xcsoar_file)
{
  is_xcsoar_file = false;

  try {
    FileLineReaderA reader(igc_path);

    BrokenDate igc_date = BrokenDate::Invalid();
    BrokenTime igc_start_time = BrokenTime::Invalid();
    char *line;
    bool found_date = false;
    bool found_fix = false;

    // First pass: find HFDTE record and first B-record
    while ((line = reader.ReadLine()) != nullptr) {
      // Check for HFDTE (date) record
      if (!found_date && std::memcmp(line, "HFDTE", 5) == 0) {
        if (IGCParseDateRecord(line, igc_date)) {
          found_date = true;
        }
      }

      // Check for A record to detect XCSoar files (manufacturer code "XCS")
      if (std::memcmp(line, "AXCS", 4) == 0) {
        is_xcsoar_file = true;
      }

      // Check for first B record (fix)
      if (!found_fix && line[0] == 'B' && std::strlen(line) >= 7) {
        IGCExtensions extensions;
        extensions.clear();
        IGCFix fix;

        if (IGCParseFix(line, extensions, fix)) {
          igc_start_time = fix.time;
          found_fix = true;
          break; // Found what we needed
        }
      }
    }

    // If we found both date and start time, check if they match the flight
    if (!found_date || !found_fix || !igc_start_time.IsPlausible())
      return false;

    // Create a datetime from the IGC date and first fix time
    BrokenDateTime igc_start(igc_date, igc_start_time);
    BrokenDateTime flight_start(flight.date, flight.start_time);
    BrokenDateTime flight_end(flight.date, flight.end_time);

    // Allow 5 minute tolerance before flight start
    auto min_time = flight_start - std::chrono::minutes{5};

    // Check if IGC start time falls within the acceptable range
    return igc_start >= min_time && igc_start <= flight_end;
  } catch (...) {
    return false;
  }
}

std::vector<AllocatedPath>
FindMatchingIGCFiles(const FlightInfo &flight) noexcept
{
  std::vector<AllocatedPath> matching_files;

  try {
    AllocatedPath logs_path = LocalPath(_T("logs"));

    IGCFileFinder visitor;
    VisitDataFiles(_T("*.igc"), visitor);

    // Separate XCSoar and other files
    std::vector<AllocatedPath> xcsoar_files;
    std::vector<AllocatedPath> other_files;

    for (const auto &igc_file : visitor.files) {
      bool is_xcsoar;
      if (IGCFileMatches(igc_file, flight, is_xcsoar)) {
        if (is_xcsoar) {
          xcsoar_files.push_back(igc_file);
        } else {
          other_files.push_back(igc_file);
        }
      }
    }

    // Sort XCSoar files by filename
    std::sort(xcsoar_files.begin(), xcsoar_files.end());

    // Sort other files by filename
    std::sort(other_files.begin(), other_files.end());

    // Combine: XCSoar files first, then others
    matching_files.insert(matching_files.end(),
                          xcsoar_files.begin(), xcsoar_files.end());
    matching_files.insert(matching_files.end(),
                          other_files.begin(), other_files.end());
  } catch (...) {
    // Return empty list on error
  }

  return matching_files;
}
