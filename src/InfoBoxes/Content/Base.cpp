// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Base.hpp"

InfoBoxContent::~InfoBoxContent() noexcept = default;

bool
InfoBoxContent::HandleKey([[maybe_unused]] const InfoBoxKeyCodes keycode) noexcept
{
  return false;
}

bool
InfoBoxContent::HandleClick() noexcept
{
  return false;
}

// Setting HasInteraction to true for infoboxes that open a panel
bool
InfoBoxContent::HasInteraction() noexcept
{
  return GetDialogContent() != nullptr;
}

void
InfoBoxContent::OnCustomPaint([[maybe_unused]] Canvas &canvas, [[maybe_unused]] const PixelRect &rc) noexcept
{
}

const InfoBoxPanel *
InfoBoxContent::GetDialogContent() noexcept
{
  return nullptr;
}
