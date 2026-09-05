// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"

#include "Airspace/AirspaceClassDisplay.hpp"
#include "Language/Language.hpp"
#include "Waypoint/MapFilterTypes.hpp"

#include <array>
#include <cassert>

namespace {

using namespace DisplaySettingCatalog;
using NumericFormat = DisplaySettingNumericFormat;

static_assert(::WAYPOINT_MAP_FILTER_TYPE_COUNT ==
              DisplaySettingCatalog::WAYPOINT_TYPE_COUNT);
static_assert(::AIRSPACE_CLASS_DISPLAY_SETTING_COUNT ==
              DisplaySettingCatalog::AIRSPACE_CLASS_COUNT);

constexpr DisplaySettingDescriptor
Boolean(DisplaySettingKey key, DisplaySettingGroup group,
        const char *profile_suffix, const char *label,
        const char *help=nullptr) noexcept
{
  return {
    key, group, profile_suffix, label, help,
    DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr,
  };
}

constexpr DisplaySettingDescriptor
Enumeration(DisplaySettingKey key, DisplaySettingGroup group,
            const char *profile_suffix, const char *label,
            int32_t minimum, int32_t maximum,
            const char *help=nullptr) noexcept
{
  return {
    key, group, profile_suffix, label, help,
    DisplaySettingValueType::ENUM,
    DisplaySettingValue::Integer(minimum),
    DisplaySettingValue::Integer(maximum), nullptr,
  };
}

constexpr DisplaySettingDescriptor
Integer(DisplaySettingKey key, DisplaySettingGroup group,
        const char *profile_suffix, const char *label,
        int32_t minimum, int32_t maximum, int32_t step,
        NumericFormat format,
        const char *help=nullptr) noexcept
{
  DisplaySettingDescriptor descriptor{
    key, group, profile_suffix, label, help,
    DisplaySettingValueType::INTEGER,
    DisplaySettingValue::Integer(minimum),
    DisplaySettingValue::Integer(maximum), nullptr,
  };
  descriptor.integer_step = step;
  descriptor.numeric_format = format;
  return descriptor;
}

template<std::size_t N>
constexpr DisplaySettingDescriptor
Choices(DisplaySettingDescriptor descriptor,
        const DisplaySettingEnumChoice (&choices)[N]) noexcept
{
  descriptor.enum_choices = choices;
  descriptor.enum_choice_count = static_cast<uint16_t>(N);
  return descriptor;
}

constexpr DisplaySettingDescriptor
Overwritable(DisplaySettingDescriptor descriptor,
             DisplaySettingEffects effects) noexcept
{
  descriptor.element_set_overwritable = true;
  descriptor.accessor = {
    DisplaySettingRuntime::GetGlobalValue,
    DisplaySettingRuntime::SetGlobalValue,
    DisplaySettingRuntime::SetEffectiveValue,
  };
  descriptor.effects = effects;
  return descriptor;
}

static constexpr DisplaySettingEffects TERRAIN_EFFECTS =
  ToDisplaySettingEffects(DisplaySettingEffect::REDRAW) |
  ToDisplaySettingEffects(DisplaySettingEffect::TERRAIN_CACHE);

static constexpr DisplaySettingEffects ORIENTATION_EFFECTS =
  ToDisplaySettingEffects(DisplaySettingEffect::REDRAW) |
  ToDisplaySettingEffects(DisplaySettingEffect::PROJECTION);

static constexpr DisplaySettingEffects WAYPOINT_EFFECTS =
  ToDisplaySettingEffects(DisplaySettingEffect::REDRAW);

static constexpr DisplaySettingEffects WAYPOINT_LOOK_EFFECTS =
  WAYPOINT_EFFECTS |
  ToDisplaySettingEffects(DisplaySettingEffect::WAYPOINT_LOOK);

static constexpr DisplaySettingEffects AIRSPACE_EFFECTS =
  ToDisplaySettingEffects(DisplaySettingEffect::REDRAW);

static constexpr DisplaySettingEffects AIRSPACE_COMPUTER_EFFECTS =
  AIRSPACE_EFFECTS |
  ToDisplaySettingEffects(DisplaySettingEffect::AIRSPACE_COMPUTER);

static constexpr DisplaySettingEnumChoice terrain_ramp_choices[] = {
  {0, N_("Low lands"), nullptr},
  {1, N_("Mountainous"), nullptr},
  {2, N_("Imhof 7"), nullptr},
  {3, N_("Imhof 4"), nullptr},
  {4, N_("Imhof 12"), nullptr},
  {5, N_("Imhof Atlas"), nullptr},
  {6, N_("ICAO"), nullptr},
  {9, N_("Vibrant"), nullptr},
  {7, N_("Grey"), nullptr},
  {8, N_("White"), nullptr},
  {10, N_("Sandstone"), nullptr},
  {11, N_("Pastel"), nullptr},
  {12, N_("Italian Avioportolano VFR Chart"), nullptr},
  {13, N_("German DFS VFR Chart"), nullptr},
  {14, N_("French SIA VFR Chart"), nullptr},
  {15, N_("High Contrast"), nullptr},
  {16, N_("High Contrast low lands"), nullptr},
  {17, N_("Very low lands"), nullptr},
};

static constexpr DisplaySettingEnumChoice slope_shading_choices[] = {
  {0, N_("Off"), nullptr},
  {1, N_("Fixed (North-West)"), nullptr},
  {2, N_("Sun"), nullptr},
  {3, N_("Wind"), nullptr},
  {4, N_("Fixed (Top Left)"), nullptr},
};

static constexpr DisplaySettingEnumChoice contour_choices[] = {
  {0, N_("Off"), N_("No contour lines")},
  {1, N_("Mountains"), N_("For steep mountain terrain, 256m minimum spacing")},
  {2, N_("Highlands"), N_("Medium density, with 64m minimum spacing")},
  {3, N_("Lowlands"), N_("More line density for gentler slopes. 16m minimum spacing")},
  {4, N_("Superfine"), N_("Maximum density contour lines down to 8m spacing")},
  {5, N_("Fixed 256m"), N_("Fixed 256m spacing, no zoom dependence")},
  {6, N_("Fixed 128m"), N_("Fixed 128m spacing, no zoom dependence")},
  {7, N_("Fixed 64m"), N_("Fixed 64m spacing, no zoom dependence")},
};

static constexpr DisplaySettingEnumChoice orientation_choices[] = {
  {0, N_("Track up"), N_("The moving map display will be rotated so the glider's track is oriented up.")},
  {3, N_("Heading up"), N_("The moving map display will be rotated so the glider's heading is oriented up.")},
  {1, N_("North up"), N_("The moving map display will always be orientated north to south and the glider icon will be rotated to show its course.")},
  {2, N_("Target up"), N_("The moving map display will be rotated so the navigation target is oriented up.")},
  {4, N_("Wind up"), N_("The moving map display will be rotated so the wind is always oriented up to down.")},
};

static constexpr DisplaySettingEnumChoice map_shift_bias_choices[] = {
  {0, N_("None"), N_("Disable adjustments.")},
  {1, N_("Track"), N_("Use a recent average of the ground track as basis.")},
  {2, N_("Target"), N_("Use the current target waypoint as basis.")},
};

static constexpr DisplaySettingEnumChoice waypoint_label_format_choices[] = {
  {0, N_("Full name"),
   N_("The full name of each waypoint is displayed.")},
  {6, N_("First word of name"),
   N_("The first word of the waypoint name is displayed.")},
  {4, N_("First 3 letters"),
   N_("The first 3 letters of the waypoint name are displayed.")},
  {2, N_("First 5 letters"),
   N_("The first 5 letters of the waypoint name are displayed.")},
  {3, N_("None"), N_("No waypoint name is displayed.")},
  {7, N_("Short Name"),
   N_("The short name of each waypoint is displayed. If unavailable, "
      "the first five letters of the full name are displayed.")},
};

static constexpr DisplaySettingEnumChoice waypoint_arrival_choices[] = {
  {0, N_("None"), N_("No arrival height is displayed.")},
  {1, N_("Straight glide"),
   N_("Straight glide arrival height (no terrain is considered).")},
  {2, N_("Terrain avoidance glide"),
   N_("Arrival height considering terrain avoidance.")},
  {3, N_("Straight & terrain glide"),
   N_("Both arrival heights are displayed.")},
  {4, N_("Required glide ratio"), nullptr},
  {5, N_("Required GR & terrain glide"),
   N_("Both required glide ratio and terrain avoidance height are displayed.")},
};

static constexpr DisplaySettingEnumChoice waypoint_label_style_choices[] = {
  {5, N_("Rounded rectangle"), nullptr},
  {3, N_("Outlined"), nullptr},
};

static constexpr DisplaySettingEnumChoice waypoint_visibility_choices[] = {
  {0, N_("All"), N_("All labels will be displayed.")},
  {4, N_("Task waypoints & airfields"),
   N_("All task waypoints and airfields will be displayed.")},
  {1, N_("Task waypoints & landables"),
   N_("All task waypoints and landables will be displayed.")},
  {2, N_("Task waypoints"),
   N_("All waypoints in the task will be displayed.")},
  {3, N_("None"), N_("No labels will be displayed.")},
};

static constexpr DisplaySettingEnumChoice waypoint_landable_choices[] = {
  {0, N_("Purple circle"), nullptr},
  {1, N_("B/W"), nullptr},
  {2, N_("Traffic lights"), nullptr},
};

static constexpr DisplaySettingEnumChoice airspace_display_choices[] = {
  {0, N_("All on"), N_("All airspaces are displayed.")},
  {1, N_("Clip"), N_("Display airspaces below the clip altitude.")},
  {2, NC_("Setting", "Auto"),
   N_("Display airspaces within a margin of the glider.")},
  {3, N_("All below"),
   N_("Display airspaces below the glider or within a margin.")},
  {5, N_("All off"), N_("No airspaces are displayed.")},
};

static constexpr DisplaySettingEnumChoice airspace_label_choices[] = {
  {0, N_("None"), N_("No labels will be displayed.")},
  {1, N_("All"), N_("All labels will be displayed.")},
};

static constexpr DisplaySettingEnumChoice airspace_fill_choices[] = {
  {0, N_("Default"),
   N_("Select the best performing option for this hardware.")},
  {1, N_("Fill all"),
   N_("Transparently fill the whole airspace area.")},
  {2, N_("Fill padding"),
   N_("Draw a solid outline with a partly transparent border.")},
  {3, N_("No fill"), N_("Do not fill the airspace area.")},
};

static const std::array<DisplaySettingDescriptor, COUNT> catalog = [] {
  std::array<DisplaySettingDescriptor, COUNT> result{};
  std::size_t n = 0;

  const auto add = [&result, &n](DisplaySettingDescriptor descriptor) {
    assert(n < result.size());
    result[n++] = descriptor;
  };

  add(Overwritable(
    Boolean(Key::TERRAIN_DISPLAY, DisplaySettingGroup::TERRAIN,
            "TerrainDisplay", N_("Terrain Display"),
            N_("Draw a digital elevation terrain on the map.")),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Boolean(Key::TOPOGRAPHY_DISPLAY, DisplaySettingGroup::TERRAIN,
            "TopographyDisplay", N_("Topography display"),
            N_("Draw topographical features (roads, rivers, lakes etc.) on the map.")),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::TERRAIN_RAMP, DisplaySettingGroup::TERRAIN,
                        "TerrainRamp", N_("Terrain colors"), 0, 17,
                        N_("Defines the color ramp used in terrain rendering.")),
            terrain_ramp_choices),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::TERRAIN_SLOPE_SHADING,
                        DisplaySettingGroup::TERRAIN,
                        "TerrainSlopeShading", N_("Slope shading"), 0, 4),
            slope_shading_choices),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Integer(Key::TERRAIN_CONTRAST, DisplaySettingGroup::TERRAIN,
            "TerrainContrast", N_("Terrain contrast"), 0, 100, 5,
            NumericFormat::PERCENT),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Integer(Key::TERRAIN_BRIGHTNESS, DisplaySettingGroup::TERRAIN,
            "TerrainBrightness", N_("Terrain brightness"), 0, 100, 5,
            NumericFormat::PERCENT),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::TERRAIN_CONTOURS,
                        DisplaySettingGroup::TERRAIN,
                        "TerrainContours", N_("Contours"), 0, 7),
            contour_choices),
    TERRAIN_EFFECTS));

  add(Overwritable(
    Choices(Enumeration(Key::CRUISE_ORIENTATION,
                        DisplaySettingGroup::ORIENTATION,
                        "CruiseOrientation", N_("Cruise orientation"), 0, 4),
            orientation_choices),
    ORIENTATION_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::CIRCLING_ORIENTATION,
                        DisplaySettingGroup::ORIENTATION,
                        "CirclingOrientation", N_("Circling orientation"),
                        0, 4),
            orientation_choices),
    ORIENTATION_EFFECTS));
  add(Overwritable(
    Boolean(Key::CIRCLING_ZOOM, DisplaySettingGroup::ORIENTATION,
            "CirclingZoom", N_("Circling zoom")),
    ORIENTATION_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::MAP_SHIFT_BIAS,
                        DisplaySettingGroup::ORIENTATION,
                        "MapShiftBias", N_("Map shift reference"), 0, 2),
            map_shift_bias_choices),
    ORIENTATION_EFFECTS));
  add(Overwritable(
    Integer(Key::GLIDER_SCREEN_POSITION,
            DisplaySettingGroup::ORIENTATION,
            "GliderScreenPosition", N_("Glider position offset"),
            10, 50, 5, NumericFormat::PERCENT),
    ORIENTATION_EFFECTS));

  add(Overwritable(
    Choices(Enumeration(Key::WAYPOINT_LABEL_FORMAT,
                        DisplaySettingGroup::WAYPOINTS,
                        "WaypointLabelFormat", N_("Label format"), 0, 7),
            waypoint_label_format_choices),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::WAYPOINT_ARRIVAL_HEIGHT,
                        DisplaySettingGroup::WAYPOINTS,
                        "WaypointArrivalHeight", N_("Arrival height"), 0, 5),
            waypoint_arrival_choices),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::WAYPOINT_LABEL_STYLE,
                        DisplaySettingGroup::WAYPOINTS,
                        "WaypointLabelStyle", N_("Label style"), 0, 5),
            waypoint_label_style_choices),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::WAYPOINT_LABEL_VISIBILITY,
                        DisplaySettingGroup::WAYPOINTS,
                        "WaypointLabelVisibility", N_("Label visibility"),
                        0, 4),
            waypoint_visibility_choices),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::WAYPOINT_LANDABLE_SYMBOLS,
                        DisplaySettingGroup::WAYPOINTS,
                        "WaypointLandableSymbols", N_("Landable symbols"),
                        0, 2),
            waypoint_landable_choices),
    WAYPOINT_LOOK_EFFECTS));
  add(Overwritable(
    Integer(Key::WAYPOINT_ICON_SCALE, DisplaySettingGroup::WAYPOINTS,
            "WaypointIconScale", N_("Waypoint icon size"), 50, 200, 10,
            NumericFormat::PERCENT),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Boolean(Key::WAYPOINT_DETAILED_LANDABLES,
            DisplaySettingGroup::WAYPOINTS,
            "WaypointDetailedLandables", N_("Detailed landables")),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Integer(Key::WAYPOINT_LANDABLE_SIZE, DisplaySettingGroup::WAYPOINTS,
            "WaypointLandableSize", N_("Landable size"), 50, 200, 10,
            NumericFormat::PERCENT),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Boolean(Key::WAYPOINT_SCALE_RUNWAY_LENGTH,
            DisplaySettingGroup::WAYPOINTS,
            "WaypointScaleRunwayLength", N_("Scale runway length")),
    WAYPOINT_EFFECTS));
  for (const auto &item : GetWaypointMapFilterTypes())
    add(Overwritable(
      Boolean(DisplaySettingKey{item.display_setting_key},
              DisplaySettingGroup::WAYPOINTS,
              item.override_profile_suffix, item.label,
              N_("Display this waypoint type on the map.")),
      WAYPOINT_EFFECTS));

  add(Overwritable(
    Choices(Enumeration(Key::AIRSPACE_DISPLAY,
                        DisplaySettingGroup::AIRSPACE,
                        "AirspaceDisplay", N_("Airspace display"), 0, 5),
            airspace_display_choices),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::AIRSPACE_LABEL_VISIBILITY,
                        DisplaySettingGroup::AIRSPACE,
                        "AirspaceLabelVisibility", N_("Label visibility"),
                        0, 1),
            airspace_label_choices),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_SHOW_NOTAM_LABELS,
            DisplaySettingGroup::AIRSPACE,
            "AirspaceShowNotamLabels", N_("Show NOTAM labels")),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Integer(Key::AIRSPACE_CLIP_ALTITUDE, DisplaySettingGroup::AIRSPACE,
            "AirspaceClipAltitude", N_("Clip altitude"), 0, 20000, 100,
            NumericFormat::ALTITUDE),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Integer(Key::AIRSPACE_MARGIN, DisplaySettingGroup::AIRSPACE,
            "AirspaceMargin", N_("Margin"), 0, 10000, 100,
            NumericFormat::ALTITUDE),
    AIRSPACE_COMPUTER_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_WARNINGS, DisplaySettingGroup::AIRSPACE,
            "AirspaceWarnings", N_("Warnings")),
    AIRSPACE_COMPUTER_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_WARNING_DIALOG, DisplaySettingGroup::AIRSPACE,
            "AirspaceWarningDialog", N_("Warnings dialog")),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Integer(Key::AIRSPACE_WARNING_TIME, DisplaySettingGroup::AIRSPACE,
            "AirspaceWarningTime", N_("Warning time"), 10, 1000, 5,
            NumericFormat::DURATION),
    AIRSPACE_COMPUTER_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_REPETITIVE_SOUND, DisplaySettingGroup::AIRSPACE,
            "AirspaceRepetitiveSound", N_("Repetitive sound")),
    AIRSPACE_COMPUTER_EFFECTS));
  add(Overwritable(
    Integer(Key::AIRSPACE_ACKNOWLEDGE_TIME,
            DisplaySettingGroup::AIRSPACE,
            "AirspaceAcknowledgeTime", N_("Acknowledge time"),
            10, 1000, 5, NumericFormat::DURATION),
    AIRSPACE_COMPUTER_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_BLACK_OUTLINE, DisplaySettingGroup::AIRSPACE,
            "AirspaceBlackOutline", N_("Use black outline")),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Choices(Enumeration(Key::AIRSPACE_FILL_MODE,
                        DisplaySettingGroup::AIRSPACE,
                        "AirspaceFillMode", N_("Airspace fill mode"), 0, 3),
            airspace_fill_choices),
    AIRSPACE_EFFECTS));
  add(Overwritable(
    Boolean(Key::AIRSPACE_TRANSPARENCY, DisplaySettingGroup::AIRSPACE,
            "AirspaceTransparency", N_("Airspace transparency")),
    AIRSPACE_EFFECTS));
  for (const auto &item : GetAirspaceClassDisplaySettings())
    add(Overwritable(
      Boolean(DisplaySettingKey{item.display_setting_key},
              DisplaySettingGroup::AIRSPACE,
              item.override_profile_suffix, item.label,
              N_("Display this airspace class or type on the map.")),
      AIRSPACE_EFFECTS));

  assert(n == result.size());
  return result;
}();

} // namespace

std::span<const DisplaySettingDescriptor>
GetMapDisplaySettingCatalog() noexcept
{
  return catalog;
}
