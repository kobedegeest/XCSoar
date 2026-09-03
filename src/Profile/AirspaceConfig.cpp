// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceConfig.hpp"
#include "Map.hpp"
#include "Keys.hpp"
#include "ui/canvas/Features.hpp"
#include "Look/AirspaceLook.hpp"
#include "Renderer/AirspaceRendererSettings.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "util/Macros.hpp"
#include "util/StringFormat.hpp"

#include <cassert>
#include <chrono>

#ifdef HAVE_HTTP
#include "NotamConfig.hpp"
#endif

template <size_t N>
static const char *
MakeAirspaceSettingName(char (&buffer)[N], const char *prefix, unsigned n)
{
  if (prefix == nullptr || prefix[0] == '\0')
    prefix = "";

  const int written = StringFormat(buffer, N, "%s%u", prefix, n);
  assert(written > 0 && written < (int)N);
  if (written <= 0 || static_cast<size_t>(written) >= N)
    buffer[0] = '\0';

  return buffer;
}

/**
 * This function and the "ColourXX" profile keys are deprecated and
 * are only used as a fallback for old profiles.
 */
static bool
GetAirspaceColor(const ProfileMap &map, unsigned i, RGB8Color &color)
{
  char name[64];
  MakeAirspaceSettingName(name, "Colour", i);

  // Try to load the hex color directly
  if (map.GetColor(name, color))
    return true;

  // Try to load an indexed preset color (legacy, < 6.3)
  unsigned index;
  if (!map.Get(name, index))
    return false;

  // Adjust index if the user has configured a preset color out of range
  if (index >= ARRAY_SIZE(AirspaceLook::preset_colors))
    index = 0;

  // Assign configured preset color
  color = AirspaceLook::preset_colors[index];
  return true;
}

void
Profile::Load(const ProfileMap &map, AirspaceRendererSettings &settings)
{
  unsigned value = static_cast<unsigned>(settings.label_selection);
  if (map.Get(ProfileKeys::AirspaceLabelSelection, value) && value < 2)
    settings.label_selection =
      static_cast<AirspaceRendererSettings::LabelSelection>(value);

  map.Get(ProfileKeys::AirspaceShowNOTAMLabels, settings.show_notam_labels);
  map.Get(ProfileKeys::AirspaceBlackOutline, settings.black_outline);

  value = static_cast<unsigned>(settings.altitude_mode);
  if (map.Get(ProfileKeys::AltMode, value) &&
      (value <= static_cast<unsigned>(AirspaceDisplayMode::ALLBELOW) ||
       value == static_cast<unsigned>(AirspaceDisplayMode::ALLOFF)))
    settings.altitude_mode = static_cast<AirspaceDisplayMode>(value);

  unsigned clip_altitude = settings.clip_altitude;
  if (map.Get(ProfileKeys::ClipAlt, clip_altitude) &&
      clip_altitude <= 20000)
    settings.clip_altitude = clip_altitude;

#if defined(HAVE_HATCHED_BRUSH) && defined(HAVE_ALPHA_BLEND)
  map.Get(ProfileKeys::AirspaceTransparency, settings.transparency);
#endif

  value = static_cast<unsigned>(settings.fill_mode);
  if (map.Get(ProfileKeys::AirspaceFillMode, value) && value < 4)
    settings.fill_mode =
      static_cast<AirspaceRendererSettings::FillMode>(value);

  for (unsigned i = 0; i < AIRSPACECLASSCOUNT; i++)
    Load(map, i, settings.classes[i]);
}

void
Profile::Load(const ProfileMap &map,
              unsigned i, AirspaceClassRendererSettings &settings)
{
  char name[64];

  MakeAirspaceSettingName(name, "AirspaceDisplay", i);
  if (!map.Get(name, settings.display)) {
    // Load setting from legacy key-value pair
    MakeAirspaceSettingName(name, "AirspaceMode", i);

    unsigned value;
    if (map.Get(name, value))
      settings.display = (value & 0x1) != 0;
  }

#ifdef HAVE_HATCHED_BRUSH
  MakeAirspaceSettingName(name, "Brush", i);
  map.Get(name, settings.brush);
  if (settings.brush >= ARRAY_SIZE(AirspaceLook::brushes))
    settings.brush = 0;
#endif

  MakeAirspaceSettingName(name, "AirspaceBorderColor", i);
  if (!map.GetColor(name, settings.border_color))
    GetAirspaceColor(map, i, settings.border_color);

  MakeAirspaceSettingName(name, "AirspaceFillColor", i);
  if (!map.GetColor(name, settings.fill_color))
    GetAirspaceColor(map, i, settings.fill_color);

  MakeAirspaceSettingName(name, "AirspaceBorderWidth", i);
  map.Get(name, settings.border_width);

  MakeAirspaceSettingName(name, "AirspaceFillMode", i);
  unsigned fill_mode = static_cast<unsigned>(settings.fill_mode);
  if (map.Get(name, fill_mode) && fill_mode < 3)
    settings.fill_mode =
      static_cast<AirspaceClassRendererSettings::FillMode>(fill_mode);
}

void
Profile::Load(const ProfileMap &map, AirspaceComputerSettings &settings)
{
  map.Get(ProfileKeys::AirspaceWarning, settings.enable_warnings);

  unsigned margin = settings.warnings.altitude_warning_margin;
  if (map.Get(ProfileKeys::AltMargin, margin) && margin <= 10000)
    settings.warnings.altitude_warning_margin = margin;

  using namespace std::chrono;
  auto warning_time = settings.warnings.warning_time;
  if (map.Get(ProfileKeys::WarningTime, warning_time) &&
      warning_time >= seconds{10} && warning_time <= seconds{1000})
    settings.warnings.warning_time = warning_time;

  auto acknowledgement_time = settings.warnings.acknowledgement_time;
  if (map.Get(ProfileKeys::AcknowledgementTime, acknowledgement_time) &&
      acknowledgement_time >= seconds{10} &&
      acknowledgement_time <= seconds{1000})
    settings.warnings.acknowledgement_time = acknowledgement_time;

  map.Get(ProfileKeys::RepetitiveSound, settings.warnings.repetitive_sound);

  char name[64];
  unsigned value;
  for (unsigned i = 0; i < AIRSPACECLASSCOUNT; i++) {
    MakeAirspaceSettingName(name, "AirspaceWarning", i);
    if (!map.Get(name, settings.warnings.class_warnings[i])) {
      // Load setting from legacy key-value pair
      MakeAirspaceSettingName(name, "AirspaceMode", i);
      if (map.Get(name, value))
        settings.warnings.class_warnings[i] = (value & 0x2) != 0;
    }
  }

#ifdef HAVE_HTTP
  // Load NOTAM settings via NotamConfig
  Profile::LoadNOTAMSettings(map, settings.notam);
#endif
}

void
Profile::LoadAirspaceWarningDialog(const ProfileMap &map,
                                   bool &enabled) noexcept
{
  map.Get(ProfileKeys::AirspaceWarningDialog, enabled);
}

void
Profile::SetAirspaceMode(ProfileMap &map,
                         unsigned i, bool display, bool warning)
{
  char name[64];

  MakeAirspaceSettingName(name, "AirspaceDisplay", i);
  map.Set(name, display);

  MakeAirspaceSettingName(name, "AirspaceWarning", i);
  map.Set(name, warning);
}

void
Profile::SetAirspaceBorderWidth(ProfileMap &map,
                                unsigned i, unsigned border_width)
{
  char name[64];
  MakeAirspaceSettingName(name, "AirspaceBorderWidth", i);
  map.Set(name, border_width);
}

void
Profile::SetAirspaceBorderColor(ProfileMap &map,
                                unsigned i, const RGB8Color &color)
{
  char name[64];
  MakeAirspaceSettingName(name, "AirspaceBorderColor", i);
  map.SetColor(name, color);
}

void
Profile::SetAirspaceFillColor(ProfileMap &map,
                              unsigned i, const RGB8Color &color)
{
  char name[64];
  MakeAirspaceSettingName(name, "AirspaceFillColor", i);
  map.SetColor(name, color);
}

void
Profile::SetAirspaceFillMode(ProfileMap &map, unsigned i, uint8_t mode)
{
  char name[64];
  MakeAirspaceSettingName(name, "AirspaceFillMode", i);
  map.SetEnum(name, (AirspaceClassRendererSettings::FillMode)mode);
}

void
Profile::SetAirspaceBrush(ProfileMap &map, unsigned i, int brush_index)
{
  char name[64];
  MakeAirspaceSettingName(name, "Brush", i);
  map.Set(name, brush_index);
}
