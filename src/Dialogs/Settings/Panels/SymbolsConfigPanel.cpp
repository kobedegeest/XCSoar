// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "SymbolsConfigPanel.hpp"
#include "Profile/Keys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "MapSettings.hpp"

enum ControlIndex {
  FADE_TRAFFIC,
  ENABLE_DETOUR_COST_MARKERS,
  AIRCRAFT_SYMBOL,
  WIND_ARROW_STYLE,
  SKYLINES_TRAFFIC_MAP_MODE,
};

class SymbolsConfigPanel final : public RowFormWidget {
public:
  SymbolsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  /* methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;
};

static constexpr StaticEnumChoice  aircraft_symbol_list[] = {
  { AircraftSymbol::SIMPLE, N_("Simple"),
    N_("Simplified line graphics, black with white contours.") },
  { AircraftSymbol::SIMPLE_LARGE, N_("Simple (large)"),
    N_("Enlarged simple graphics.") },
  { AircraftSymbol::DETAILED, N_("Detailed"),
    N_("Detailed rendered aircraft graphics.") },
  { AircraftSymbol::HANGGLIDER, N_("HangGlider"),
    N_("Simplified hang glider as line graphics, white with black contours.") },
  { AircraftSymbol::PARAGLIDER, N_("Paraglider"),
    N_("Simplified para glider as line graphics, white with black contours.") },
  nullptr
};

static constexpr StaticEnumChoice wind_arrow_list[] = {
  { WindArrowStyle::NO_ARROW, N_("Off"), N_("No wind arrow is drawn.") },
  { WindArrowStyle::ARROW_HEAD, N_("Arrow head"), N_("Draws an arrow head only.") },
  { WindArrowStyle::FULL_ARROW, N_("Full arrow"), N_("Draws an arrow head with a dashed arrow line.") },
  nullptr
};

static constexpr StaticEnumChoice online_traffic_map_mode_list[] = {
  { DisplayOnlineTrafficMapMode::OFF, N_("Off"), N_("No online traffic is drawn.") },
  { DisplayOnlineTrafficMapMode::SYMBOL, N_("Symbol"), N_("Draws the traffic symbol only.") },
  { DisplayOnlineTrafficMapMode::SYMBOL_NAME, N_("Symbol and Name"), N_("Draws the traffic symbol with name.") },
  nullptr
};

void
SymbolsConfigPanel::Prepare([[maybe_unused]] ContainerWindow &parent,
                            [[maybe_unused]] const PixelRect &rc) noexcept
{
  const MapSettings &settings_map = CommonInterface::GetMapSettings();

  AddBoolean(_("Fade traffic"), _("Keep showing traffic for a while after it has disappeared."),
             settings_map.fade_traffic);

  AddBoolean(_("Detour cost markers"),
             _("If the aircraft heading deviates from the current waypoint, markers are displayed "
                 "at points ahead of the aircraft. The value of each marker is the extra distance "
                 "required to reach that point as a percentage of straight-line distance to the waypoint."),
             settings_map.detour_cost_markers_enabled);
  SetExpertRow(ENABLE_DETOUR_COST_MARKERS);

  AddEnum(_("Aircraft symbol"), nullptr, aircraft_symbol_list,
          (unsigned)settings_map.aircraft_symbol);
  SetExpertRow(AIRCRAFT_SYMBOL);

  AddEnum(_("Wind arrow"), _("Determines the way the wind arrow is drawn on the map."),
          wind_arrow_list, (unsigned)settings_map.wind_arrow_style);
  SetExpertRow(WIND_ARROW_STYLE);

  AddEnum(C_("Setting", "Online traffic on map"),
          _("Show traffic from SkyLines and XCSoar Cloud on the map."),
          online_traffic_map_mode_list,
          (unsigned)settings_map.online_traffic_map_mode);
}

bool
SymbolsConfigPanel::Save(bool &_changed) noexcept
{
  bool changed = false;

  MapSettings &settings_map = CommonInterface::SetMapSettings();

  changed |= SaveValue(FADE_TRAFFIC, ProfileKeys::FadeTraffic,
                       settings_map.fade_traffic);

  changed |= SaveValue(ENABLE_DETOUR_COST_MARKERS, ProfileKeys::DetourCostMarker,
                       settings_map.detour_cost_markers_enabled);

  changed |= SaveValueEnum(AIRCRAFT_SYMBOL, ProfileKeys::AircraftSymbol, settings_map.aircraft_symbol);

  changed |= SaveValueEnum(WIND_ARROW_STYLE, ProfileKeys::WindArrowStyle, settings_map.wind_arrow_style);

  changed |= SaveValueEnum(SKYLINES_TRAFFIC_MAP_MODE, ProfileKeys::OnlineTrafficMapMode,
                           settings_map.online_traffic_map_mode);

  _changed |= changed;

  return true;
}

std::unique_ptr<Widget>
CreateSymbolsConfigPanel()
{
  return std::make_unique<SymbolsConfigPanel>();
}
