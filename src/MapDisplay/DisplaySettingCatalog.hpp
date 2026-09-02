// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ElementSetDisplayOverrides.hpp"

#include <span>

/**
 * Map-display settings known to the element-set override system.
 *
 * The catalog is intentionally empty until the deny-by-default catalog is
 * introduced.  Merely adding a setting elsewhere must never expose it here.
 */
constexpr std::span<const DisplaySettingDescriptor>
GetMapDisplaySettingCatalog() noexcept
{
  return {};
}
