// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <cstdint>

enum class ThermalAssistantPosition : uint8_t {
  OFF,
  BOTTOM_LEFT,
  BOTTOM_LEFT_AVOID_IB,
  BOTTOM_RIGHT,
  BOTTOM_RIGHT_AVOID_IB,
  TOP_LEFT,
  TOP_RIGHT,
  CENTER_TOP,
  TOP_LEFT_AVOID_IB,
  TOP_RIGHT_AVOID_IB,
  CENTER_TOP_AVOID_IB,
};
