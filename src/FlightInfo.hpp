// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "time/BrokenDate.hpp"
#include "time/BrokenTime.hpp"

#include <string>

struct FlightInfo {
  BrokenDate date;

  BrokenTime start_time, end_time;

  std::string comment;

  std::chrono::system_clock::duration Duration() const noexcept;
};
