// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Engine/Airspace/AirspaceClass.hpp"

/** Register the renderer/computer/UI Airspace group-bundle accessors. */
void RegisterAirspaceDisplaySettings() noexcept;

/** Global value shown by the class-filter configuration dialog. */
bool GetGlobalAirspaceClassDisplay(AirspaceClass airspace_class) noexcept;
