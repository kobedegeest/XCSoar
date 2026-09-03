// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "WaypointRendererSettings.hpp"
#include "Profile/Current.hpp"
#include "Profile/Map.hpp"
#include "Profile/Keys.hpp"
#include "Waypoint/MapFilterProfile.hpp"

#include <string_view>

static bool
IsValidDisplayText(unsigned value) noexcept
{
  using Type = WaypointRendererSettings::DisplayTextType;
  switch (static_cast<Type>(value)) {
  case Type::NAME:
  case Type::FIRST_FIVE:
  case Type::NONE:
  case Type::FIRST_THREE:
  case Type::FIRST_WORD:
  case Type::SHORT_NAME:
    return true;

  case Type::OBSOLETE_DONT_USE_NUMBER:
  case Type::OBSOLETE_DONT_USE_NAMEIFINTASK:
    break;
  }

  return false;
}

template<typename T>
static void
LoadEnumInRange(const ProfileMap &map, std::string_view key,
                T &value, unsigned count) noexcept
{
  unsigned candidate = static_cast<unsigned>(value);
  if (map.Get(key, candidate) && candidate < count)
    value = static_cast<T>(candidate);
}

static void
LoadIntegerInRange(const ProfileMap &map, std::string_view key,
                   int &value, int minimum, int maximum) noexcept
{
  int candidate = value;
  if (map.Get(key, candidate) &&
      candidate >= minimum && candidate <= maximum)
    value = candidate;
}

void
WaypointRendererSettings::LoadFromProfile() noexcept
{
  LoadFromProfile(Profile::map);
}

void
WaypointRendererSettings::LoadFromProfile(const ProfileMap &map) noexcept
{
  unsigned display_text = static_cast<unsigned>(display_text_type);
  if (map.Get(ProfileKeys::DisplayText, display_text)) {
    const auto candidate = static_cast<DisplayTextType>(display_text);

    if (candidate == DisplayTextType::OBSOLETE_DONT_USE_NAMEIFINTASK) {
      // pref migration. The migrated values will not be written unless the
      // user explicitly changes the corresponding setting manually.
      display_text_type = DisplayTextType::NAME;
      label_selection = LabelSelection::TASK;
    } else if (candidate == DisplayTextType::OBSOLETE_DONT_USE_NUMBER) {
      display_text_type = DisplayTextType::NAME;
    } else if (IsValidDisplayText(display_text)) {
      display_text_type = candidate;
    }
  }

  // DisplayText must be loaded first due to the migration above.
  LoadEnumInRange(map, ProfileKeys::WaypointLabelSelection,
                  label_selection, 5);
  LoadEnumInRange(map, ProfileKeys::WaypointArrivalHeightDisplay,
                  arrival_height_display, 6);

  unsigned label_style = static_cast<unsigned>(landable_render_mode);
  if (map.Get(ProfileKeys::WaypointLabelStyle, label_style) &&
      (label_style == static_cast<unsigned>(LabelShape::ROUNDED_BLACK) ||
       label_style == static_cast<unsigned>(LabelShape::OUTLINED_INVERTED)))
    landable_render_mode = static_cast<LabelShape>(label_style);

  LoadEnumInRange(map, ProfileKeys::AppIndLandable, landable_style, 3);
  map.Get(ProfileKeys::AppUseSWLandablesRendering,
          vector_landable_rendering);
  map.Get(ProfileKeys::AppScaleRunwayLength, scale_runway_length);
  LoadIntegerInRange(map, ProfileKeys::AppLandableRenderingScale,
                     landable_rendering_scale, 50, 200);
  LoadIntegerInRange(map, ProfileKeys::MapWaypointIconScale,
                     map_waypoint_icon_scale, 50, 200);

  WaypointMapFilterProfile::Load(map, *this);
}
