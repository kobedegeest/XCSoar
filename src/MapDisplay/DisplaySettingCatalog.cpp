// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"

#include "Engine/Airspace/AirspaceClass.hpp"
#include "Language/Language.hpp"
#include "Waypoint/MapFilterTypes.hpp"

#include <array>
#include <cassert>

namespace {

using namespace DisplaySettingCatalog;

static_assert(::WAYPOINT_MAP_FILTER_TYPE_COUNT ==
              DisplaySettingCatalog::WAYPOINT_TYPE_COUNT);

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
        const char *help=nullptr) noexcept
{
  DisplaySettingDescriptor descriptor{
    key, group, profile_suffix, label, help,
    DisplaySettingValueType::INTEGER,
    DisplaySettingValue::Integer(minimum),
    DisplaySettingValue::Integer(maximum), nullptr,
  };
  descriptor.integer_step = step;
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

struct GeneratedSettingMetadata {
  unsigned source_value;
  DisplaySettingKey key;
  const char *profile_suffix;
  const char *label;
};

static constexpr GeneratedSettingMetadata airspace_classes[] = {
  {unsigned(RESTRICTED), {0x4101}, "AirspaceClassRestricted", N_("Restricted")},
  {unsigned(PROHIBITED), {0x4102}, "AirspaceClassProhibited", N_("Prohibited")},
  {unsigned(DANGER), {0x4103}, "AirspaceClassDanger", N_("Danger Area")},
  {unsigned(CLASSA), {0x4104}, "AirspaceClassA", N_("Class A")},
  {unsigned(CLASSB), {0x4105}, "AirspaceClassB", N_("Class B")},
  {unsigned(CLASSC), {0x4106}, "AirspaceClassC", N_("Class C")},
  {unsigned(CLASSD), {0x4107}, "AirspaceClassD", N_("Class D")},
  {unsigned(NOGLIDER), {0x4108}, "AirspaceClassNoGlider", N_("No Gliders")},
  {unsigned(CTR), {0x4109}, "AirspaceClassCTR", N_("CTR")},
  {unsigned(WAVE), {0x410a}, "AirspaceClassWave", N_("Wave")},
  {unsigned(AATASK), {0x410b}, "AirspaceClassAATask", N_("Task Area")},
  {unsigned(CLASSE), {0x410c}, "AirspaceClassE", N_("Class E")},
  {unsigned(CLASSF), {0x410d}, "AirspaceClassF", N_("Class F")},
  {unsigned(TMZ), {0x410e}, "AirspaceClassTMZ", N_("Transponder Mandatory Zone")},
  {unsigned(CLASSG), {0x410f}, "AirspaceClassG", N_("Class G")},
  {unsigned(MATZ), {0x4110}, "AirspaceClassMATZ", N_("Military Aerodrome Traffic Zone")},
  {unsigned(RMZ), {0x4111}, "AirspaceClassRMZ", N_("Radio Mandatory Zone")},
  {unsigned(UNCLASSIFIED), {0x4112}, "AirspaceClassUnclassified", N_("Unclassified")},
  {unsigned(TMA), {0x4113}, "AirspaceClassTMA", N_("TMA")},
  {unsigned(TRA), {0x4114}, "AirspaceClassTRA", N_("Temporary Reserved Airspace")},
  {unsigned(TSA), {0x4115}, "AirspaceClassTSA", N_("Temporary Segregated Area")},
  {unsigned(FIR), {0x4116}, "AirspaceClassFIR", N_("Flight Information Region")},
  {unsigned(UIR), {0x4117}, "AirspaceClassUIR", N_("Upper Flight Information Region")},
  {unsigned(ADIZ), {0x4118}, "AirspaceClassADIZ", N_("Air Defense Identification Zone")},
  {unsigned(ATZ), {0x4119}, "AirspaceClassATZ", N_("Aerodrome Traffic Zone")},
  {unsigned(AWY), {0x411a}, "AirspaceClassAirway", N_("Airway")},
  {unsigned(MTR), {0x411b}, "AirspaceClassMTR", N_("Military Training Route")},
  {unsigned(ALERT), {0x411c}, "AirspaceClassAlert", N_("Alert Area")},
  {unsigned(WARNING), {0x411d}, "AirspaceClassWarning", N_("Warning Area")},
  {unsigned(PROTECTED), {0x411e}, "AirspaceClassProtected", N_("Protected Area")},
  {unsigned(HTZ), {0x411f}, "AirspaceClassHTZ", N_("Hazardous Area")},
  {unsigned(GLIDING_SECTOR), {0x4120}, "AirspaceClassGlidingSector", N_("Gliding Sector")},
  {unsigned(TRP), {0x4121}, "AirspaceClassTRP", N_("Temporary Reserved Prohibited Area")},
  {unsigned(TIZ), {0x4122}, "AirspaceClassTIZ", N_("Terminal Information Zone")},
  {unsigned(TIA), {0x4123}, "AirspaceClassTIA", N_("Terminal Instrument Approach Procedure Area")},
  {unsigned(MTA), {0x4124}, "AirspaceClassMTA", N_("Military Training Area")},
  {unsigned(CTA), {0x4125}, "AirspaceClassCTA", N_("Control Area")},
  {unsigned(ACC_SECTOR), {0x4126}, "AirspaceClassACCSector", N_("Area Control Center Sector")},
  {unsigned(AERIAL_SPORTING_RECREATIONAL), {0x4127}, "AirspaceClassAerialSportingRecreational", N_("Aerial Sporting Recreational")},
  {unsigned(OVERFLIGHT_RESTRICTION), {0x4128}, "AirspaceClassOverflightRestriction", N_("Overflight Restriction")},
  {unsigned(MRT), {0x4129}, "AirspaceClassMRT", N_("Military Restricted Area")},
  {unsigned(TFR), {0x412a}, "AirspaceClassTFR", N_("Temporary Flight Restriction")},
  {unsigned(VFR_SECTOR), {0x412b}, "AirspaceClassVFRSector", N_("Visual Flight Rules Sector")},
  {unsigned(FIS_SECTOR), {0x412c}, "AirspaceClassFISSector", N_("Flight Information Sector")},
  {unsigned(LTA), {0x412d}, "AirspaceClassLTA", N_("Lower Traffic Area")},
  {unsigned(UTA), {0x412e}, "AirspaceClassUTA", N_("Upper Traffic Area")},
  {unsigned(ASRA), {0x412f}, "AirspaceClassASRA", N_("Aerial Sporting Or Recreational Activity")},
  {unsigned(NOTAM), {0x4130}, "AirspaceClassNOTAM", N_("NOTAM Affected Area")},
  {unsigned(NONE), {0x4131}, "AirspaceClassNone", N_("Airspace without type")},
  {unsigned(TRAFR), {0x4132}, "AirspaceClassTRAFR", N_("TRA/TSA Feeding Route")},
  {unsigned(TRZ), {0x4133}, "AirspaceClassTRZ", N_("Transponder Recommended Zone")},
  {unsigned(VFR_ROUTE), {0x4134}, "AirspaceClassVFRRoute", N_("Designated Route for VFR")},
};

static_assert(std::size(airspace_classes) == AIRSPACE_CLASS_COUNT);
static_assert(AIRSPACECLASSCOUNT == AIRSPACE_CLASS_COUNT + 1);

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
            "TerrainContrast", N_("Terrain contrast"), 0, 100, 5),
    TERRAIN_EFFECTS));
  add(Overwritable(
    Integer(Key::TERRAIN_BRIGHTNESS, DisplaySettingGroup::TERRAIN,
            "TerrainBrightness", N_("Terrain brightness"), 0, 100, 5),
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
            10, 50, 5),
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
            "WaypointIconScale", N_("Waypoint icon size"), 50, 200, 10),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Boolean(Key::WAYPOINT_DETAILED_LANDABLES,
            DisplaySettingGroup::WAYPOINTS,
            "WaypointDetailedLandables", N_("Detailed landables")),
    WAYPOINT_EFFECTS));
  add(Overwritable(
    Integer(Key::WAYPOINT_LANDABLE_SIZE, DisplaySettingGroup::WAYPOINTS,
            "WaypointLandableSize", N_("Landable size"), 50, 200, 10),
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

  add(Enumeration(Key::AIRSPACE_DISPLAY, DisplaySettingGroup::AIRSPACE,
                  "AirspaceDisplay", N_("Airspace display"), 0, 3));
  add(Enumeration(Key::AIRSPACE_LABEL_VISIBILITY,
                  DisplaySettingGroup::AIRSPACE,
                  "AirspaceLabelVisibility", N_("Label visibility"), 0, 1));
  add(Boolean(Key::AIRSPACE_SHOW_NOTAM_LABELS,
              DisplaySettingGroup::AIRSPACE,
              "AirspaceShowNotamLabels", N_("Show NOTAM labels")));
  add(Integer(Key::AIRSPACE_CLIP_ALTITUDE, DisplaySettingGroup::AIRSPACE,
              "AirspaceClipAltitude", N_("Clip altitude"), 0, 20000, 100));
  add(Integer(Key::AIRSPACE_MARGIN, DisplaySettingGroup::AIRSPACE,
              "AirspaceMargin", N_("Margin"), 0, 10000, 100));
  add(Boolean(Key::AIRSPACE_WARNINGS, DisplaySettingGroup::AIRSPACE,
              "AirspaceWarnings", N_("Warnings")));
  add(Boolean(Key::AIRSPACE_WARNING_DIALOG, DisplaySettingGroup::AIRSPACE,
              "AirspaceWarningDialog", N_("Warnings dialog")));
  add(Integer(Key::AIRSPACE_WARNING_TIME, DisplaySettingGroup::AIRSPACE,
              "AirspaceWarningTime", N_("Warning time"), 10, 1000, 5));
  add(Boolean(Key::AIRSPACE_REPETITIVE_SOUND, DisplaySettingGroup::AIRSPACE,
              "AirspaceRepetitiveSound", N_("Repetitive sound")));
  add(Integer(Key::AIRSPACE_ACKNOWLEDGE_TIME,
              DisplaySettingGroup::AIRSPACE,
              "AirspaceAcknowledgeTime", N_("Acknowledge time"),
              10, 1000, 5));
  add(Boolean(Key::AIRSPACE_BLACK_OUTLINE, DisplaySettingGroup::AIRSPACE,
              "AirspaceBlackOutline", N_("Use black outline")));
  add(Enumeration(Key::AIRSPACE_FILL_MODE, DisplaySettingGroup::AIRSPACE,
                  "AirspaceFillMode", N_("Airspace fill mode"), 0, 3));
  add(Boolean(Key::AIRSPACE_TRANSPARENCY, DisplaySettingGroup::AIRSPACE,
              "AirspaceTransparency", N_("Airspace transparency")));
  for (const auto &item : airspace_classes)
    add(Boolean(item.key, DisplaySettingGroup::AIRSPACE,
                item.profile_suffix, item.label,
                N_("Display this airspace class or type on the map.")));

  assert(n == result.size());
  return result;
}();

} // namespace

std::span<const DisplaySettingDescriptor>
GetMapDisplaySettingCatalog() noexcept
{
  return catalog;
}
