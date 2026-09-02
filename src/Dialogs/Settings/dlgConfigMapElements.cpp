// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "dlgConfigMapElements.hpp"
#include "ElementSetDisplayOverridesDialog.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Widget/VScrollWidget.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "MapElementSettings.hpp"
#include "MapDisplay/DisplaySettingCatalog.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "util/StringAPI.hxx"

#include <memory>
#include <utility>

using namespace UI;

static MapElementSet clipboard;
static bool clipboard_valid;

class MapElementSetConfigWidget final
  : public RowFormWidget, private DataFieldListener {
  enum Controls {
    NAME,
    NAME_SEPARATOR,

    FINAL_GLIDE_BAR,
    FINAL_GLIDE_BAR_MC0,
    REACH_DISPLAY,
    TURN_BACK_MARKER,
    DETOUR_COST_MARKERS,
    GROUND_TRACK,
    DISTANCE_RINGS,
    NAVIGATION_SEPARATOR,

    TRAIL_LENGTH,
    TRAIL_DRIFT,
    TRAIL_TYPE,
    TRAIL_SCALED,
    TRAIL_SEPARATOR,

    FLARM_ON_MAP,
    ONLINE_TRAFFIC,
    TRAFFIC_SEPARATOR,

    WIND_ARROW,
    THERMAL_BAND,
    THERMAL_ASSISTANT,
#ifdef HAVE_TRACKING
    CLOUD_SHOW_THERMALS,
#endif
#ifdef HAVE_HTTP
    THERMAL_INFORMATION_MAP,
#endif
    THERMAL_SEPARATOR,

    FLARM_GAUGE,
    VARIO_BAR,
    GAUGES_SEPARATOR,

    MENU_BUTTON,
    ZOOM_BUTTON,
    QUICKMENU_BUTTON,
    DISPLAY_OVERRIDES_SEPARATOR,
    DISPLAY_OVERRIDES,
    // TODO: missing elements that need small refactoring: SkyLines traffic,
    // XCSoar Cloud traffic
  };

  static_assert(DISPLAY_OVERRIDES < 32);

  MapElementSet &data;
  ElementSetDisplayOverrides display_overrides;
  const bool allow_name_change;

  void UpdateFinalGlideBarControls() noexcept;
  void UpdateTrailControls() noexcept;
  bool SaveSettings(MapElementSet &settings) const noexcept;
  void LoadSettings(const MapElementSet &settings) noexcept;

public:
  MapElementSetConfigWidget(const DialogLook &look, MapElementSet &_data,
                            bool _allow_name_change) noexcept
    :RowFormWidget(look), data(_data),
     display_overrides(_data.display_overrides),
     allow_name_change(_allow_name_change) {}

  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

  PixelSize GetMinimumSize() const noexcept override {
    /* This is a long, scrollable form.  Using the maximum height keeps the
       controls at their normal touch-friendly size instead of compressing
       them to their minimum height to fit the viewport. */
    PixelSize size = RowFormWidget::GetMinimumSize();
    size.height = RowFormWidget::GetMaximumSize().height;
    return size;
  }

  void Copy() noexcept;
  void Paste();
  void EditDisplayOverrides();

private:
  void OnModified(DataField &df) noexcept override;
};

static constexpr StaticEnumChoice final_glide_bar_display_mode_list[] = {
  { FinalGlideBarDisplayMode::OFF, N_("Off"),
    N_("Disable final glide bar.") },
  { FinalGlideBarDisplayMode::ON, N_("On"),
    N_("Always show final glide bar.") },
  { FinalGlideBarDisplayMode::AUTO, NC_("Setting", "Auto"),
    N_("Show final glide bar if approaching final glide range.") },
  nullptr
};

static constexpr StaticEnumChoice trail_length_list[] = {
  { TrailSettings::Length::OFF, N_("Off") },
  { TrailSettings::Length::LONG, N_("Long") },
  { TrailSettings::Length::SHORT, N_("Short") },
  { TrailSettings::Length::FULL, N_("Full") },
  nullptr
};

static constexpr StaticEnumChoice trail_type_list[] = {
  { TrailSettings::Type::VARIO_1, N_("Vario #1") },
  { TrailSettings::Type::VARIO_1_DOTS, N_("Vario #1 (with dots)") },
  { TrailSettings::Type::VARIO_2, N_("Vario #2") },
  { TrailSettings::Type::VARIO_2_DOTS, N_("Vario #2 (with dots)") },
  { TrailSettings::Type::VARIO_DOTS_AND_LINES,
    N_("Vario-scaled dots and lines") },
  { TrailSettings::Type::VARIO_EINK, N_("Vario E-ink") },
  { TrailSettings::Type::ALTITUDE, N_("Altitude") },
  nullptr
};

static constexpr StaticEnumChoice ground_track_mode_list[] = {
  { DisplayGroundTrack::OFF, N_("Off"),
    N_("Disable display of ground track line.") },
  { DisplayGroundTrack::ON, N_("On"),
    N_("Always display ground track line.") },
  { DisplayGroundTrack::AUTO, NC_("Setting", "Auto"),
    N_("Display ground track line if there is a significant difference to plane heading.") },
  nullptr
};

static constexpr StaticEnumChoice wind_arrow_list[] = {
  { WindArrowStyle::NO_ARROW, N_("Off"), N_("No wind arrow is drawn.") },
  { WindArrowStyle::ARROW_HEAD, N_("Arrow head"),
    N_("Draws an arrow head only.") },
  { WindArrowStyle::FULL_ARROW, N_("Full arrow"),
    N_("Draws an arrow head with a dashed arrow line.") },
  nullptr
};

static constexpr StaticEnumChoice online_traffic_map_mode_list[] = {
  { DisplayOnlineTrafficMapMode::OFF, N_("Off"),
    N_("No online traffic is drawn.") },
  { DisplayOnlineTrafficMapMode::SYMBOL, N_("Symbol"),
    N_("Draws the traffic symbol only.") },
  { DisplayOnlineTrafficMapMode::SYMBOL_NAME, N_("Symbol and Name"),
    N_("Draws the traffic symbol with name.") },
  nullptr
};

static constexpr StaticEnumChoice thermal_assistant_position_list[] = {
  { ThermalAssistantPosition::OFF, N_("Off"),
    N_("Disable thermal assistant.") },
  { ThermalAssistantPosition::BOTTOM_LEFT, N_("Bottom left"),
    N_("Show thermal assistant in bottom left.") },
  { ThermalAssistantPosition::BOTTOM_LEFT_AVOID_IB,
    N_("Bottom left (avoid InfoBoxes)"),
    N_("Show thermal assistant in bottom left, above or to the right of "
       "InfoBoxes (if present).") },
  { ThermalAssistantPosition::BOTTOM_RIGHT, N_("Bottom right"),
    N_("Show thermal assistant in bottom right.") },
  { ThermalAssistantPosition::BOTTOM_RIGHT_AVOID_IB,
    N_("Bottom right (avoid InfoBoxes)"),
    N_("Show thermal assistant in bottom right, above or to the left of "
       "InfoBoxes (if present).") },
  { ThermalAssistantPosition::TOP_LEFT, N_("Top left"),
    N_("Show thermal assistant in top left.") },
  { ThermalAssistantPosition::TOP_RIGHT, N_("Top right"),
    N_("Show thermal assistant in top right.") },
  { ThermalAssistantPosition::CENTER_TOP, N_("Center top"),
    N_("Show thermal assistant in center top.") },
  { ThermalAssistantPosition::TOP_LEFT_AVOID_IB,
    N_("Top left (avoid InfoBoxes)"),
    N_("Show thermal assistant in top left (avoid InfoBoxes).") },
  { ThermalAssistantPosition::TOP_RIGHT_AVOID_IB,
    N_("Top right (avoid InfoBoxes)"),
    N_("Show thermal assistant in top right (avoid InfoBoxes).") },
  { ThermalAssistantPosition::CENTER_TOP_AVOID_IB,
    N_("Center top (avoid InfoBoxes)"),
    N_("Show thermal assistant in center top (avoid InfoBoxes).") },
  nullptr
};

static constexpr StaticEnumChoice final_glide_terrain_list[] = {
  { FeaturesSettings::FinalGlideTerrain::OFF, N_("Off"),
    N_("Disables the reach display.") },
  { FeaturesSettings::FinalGlideTerrain::TERRAIN_LINE, N_("Terrain line"),
    N_("Draws a dashed line at the terrain glide reach.") },
  { FeaturesSettings::FinalGlideTerrain::TERRAIN_SHADE, N_("Terrain shade"),
    N_("Shades terrain outside glide reach.") },
  { FeaturesSettings::FinalGlideTerrain::WORKING, N_("Working line"),
    N_("Draws a dashed line at the working glide reach.") },
  { FeaturesSettings::FinalGlideTerrain::WORKING_TERRAIN_LINE,
    N_("Working line, terrain line"),
    N_("Draws a dashed line at the working and terrain glide reaches.") },
  { FeaturesSettings::FinalGlideTerrain::WORKING_TERRAIN_SHADE,
    N_("Working line, terrain shade"),
    N_("Draws a dashed line at working, and shade terrain, glide reaches.") },
  nullptr
};

void
MapElementSetConfigWidget::UpdateFinalGlideBarControls() noexcept
{
  const auto mode = static_cast<FinalGlideBarDisplayMode>(
    GetValueEnum(FINAL_GLIDE_BAR));
  SetRowVisible(FINAL_GLIDE_BAR_MC0, mode != FinalGlideBarDisplayMode::OFF);
}

void
MapElementSetConfigWidget::UpdateTrailControls() noexcept
{
  const auto length = static_cast<TrailSettings::Length>(
    GetValueEnum(TRAIL_LENGTH));
  const bool visible = length != TrailSettings::Length::OFF;
  SetRowVisible(TRAIL_DRIFT, visible);
  SetRowVisible(TRAIL_TYPE, visible);
  SetRowVisible(TRAIL_SCALED, visible);
}

void
MapElementSetConfigWidget::Prepare([[maybe_unused]] ContainerWindow &parent,
                                   [[maybe_unused]] const PixelRect &rc) noexcept
{
  AddText(_("Name"), _("The name of this map element set."),
          allow_name_change ? data.name.c_str() : gettext(data.name));
  SetReadOnly(NAME, !allow_name_change);
  AddSpacer();

  AddEnum(_("Final glide bar"),
          _("If set to \"On\" the final glide will always be shown, if set to \"Auto\" it will be shown when approaching the final glide possibility."),
          final_glide_bar_display_mode_list,
          static_cast<unsigned>(data.final_glide_bar_display_mode), this);
  AddBoolean(_("Final glide bar MC0"),
             _("Show a second arrow indicating the height required to reach the final waypoint at MC zero."),
             data.final_glide_bar_mc0_enabled);
  AddEnum(_("Reach display"), nullptr, final_glide_terrain_list,
          static_cast<unsigned>(data.final_glide_terrain));
  AddBoolean(C_("Setting", "Turn back marker"),
             _("Show a green triangle on the map along the current track "
               "indicating the furthest point from which the active task "
               "waypoint or Goto target can still be reached with the "
               "current altitude and conditions. "
               "The triangle is only shown during cruise when the target "
               "is reachable."),
             data.turn_back_marker_enabled);
  AddBoolean(_("Detour cost markers"),
             _("If the aircraft heading deviates from the current waypoint, "
               "markers are displayed at points ahead of the aircraft. The "
               "value of each marker is the extra distance required to reach "
               "that point as a percentage of straight-line distance to the "
               "waypoint."),
             data.detour_cost_markers_enabled);
  SetExpertRow(DETOUR_COST_MARKERS);
  AddEnum(_("Ground track"),
          _("Display the ground track as a grey line on the map."),
          ground_track_mode_list,
          static_cast<unsigned>(data.display_ground_track));
  AddBoolean(C_("Setting", "Distance rings"),
             _("Display distance rings around the aircraft on the map."),
             data.distance_rings_enabled);
  AddSpacer();

  AddEnum(_("Trail length"),
          _("Determines whether and how long a snail trail is drawn behind the glider."),
          trail_length_list, static_cast<unsigned>(data.trail.length), this);
  AddBoolean(_("Trail drift"),
             _("Drift the snail trail with the wind in circling mode at near map scales."),
             data.trail.wind_drift_enabled);
  AddEnum(_("Trail type"), _("Sets the type of the snail trail display."),
          trail_type_list, static_cast<unsigned>(data.trail.type));
  AddBoolean(_("Trail scaled"),
             _("Scale the snail trail width according to the vario signal."),
             data.trail.scaling_enabled);
  AddSpacer();

  AddBoolean(_("FLARM Traffic"),
             _("Enable the display of FLARM traffic on the map window."),
             data.show_flarm_on_map);
  AddEnum(C_("Setting", "Online traffic on map"),
          _("Show traffic from SkyLines and XCSoar Cloud on the map."),
          online_traffic_map_mode_list,
          static_cast<unsigned>(data.online_traffic_map_mode));
  AddSpacer();

  AddEnum(_("Wind arrow"),
          _("Determines the way the wind arrow is drawn on the map."),
          wind_arrow_list, static_cast<unsigned>(data.wind_arrow_style));
  SetExpertRow(WIND_ARROW);
  AddBoolean(_("Thermal Band"),
             _("Enable the thermal profile display on the map."),
             data.show_thermal_profile);
  AddEnum(_("Thermal Assistant"),
          _("Enable and select the position of the thermal assistant when "
            "overlayed on the main screen."),
          thermal_assistant_position_list,
          static_cast<unsigned>(data.thermal_assistant_position));
#ifdef HAVE_TRACKING
  AddBoolean(_("XCSoar Cloud thermals"),
             _("Obtain and show thermal locations reported by others."),
             data.cloud_show_thermals);
#endif
#ifdef HAVE_HTTP
  AddBoolean(_("Thermal Information Map"),
             _("Show thermal locations downloaded from Thermal Information "
               "Map (thermalmap.info)."),
             data.enable_thermal_information_map);
#endif
  AddSpacer();

  AddBoolean(_("FLARM Radar"),
             _("Enable the display of the FLARM radar gauge."),
             data.flarm_gauge_enabled);
  AddBoolean(_("Vario bar"), _("Show the vario bar."),
             data.vario_bar_enabled);
  AddSpacer();
  SetExpertRow(GAUGES_SEPARATOR);

  AddBoolean(_("Show Menu button"), _("Show the Menu button"),
             data.show_menu_button);
  SetExpertRow(MENU_BUTTON);
  AddBoolean(_("Show Zoom button"), _("Show the Zoom button"),
             data.show_zoom_button);
  SetExpertRow(ZOOM_BUTTON);
  AddBoolean(C_("Setting", "Show QuickMenu button"),
             _("Show the QuickMenu button"),
             data.show_quickmenu_button);
  SetExpertRow(QUICKMENU_BUTTON);

  AddSpacer();
  AddButton(_("Setting overrides"),
            [this](){ EditDisplayOverrides(); });

  UpdateFinalGlideBarControls();
  UpdateTrailControls();
}

bool
MapElementSetConfigWidget::SaveSettings(MapElementSet &settings) const noexcept
{
  bool changed = false;

  changed |= SaveValueEnum(FINAL_GLIDE_BAR,
                           settings.final_glide_bar_display_mode);
  changed |= SaveValue(FINAL_GLIDE_BAR_MC0,
                       settings.final_glide_bar_mc0_enabled);
  changed |= SaveValueEnum(TRAIL_LENGTH, settings.trail.length);
  changed |= SaveValue(TRAIL_DRIFT, settings.trail.wind_drift_enabled);
  changed |= SaveValueEnum(TRAIL_TYPE, settings.trail.type);
  changed |= SaveValue(TRAIL_SCALED, settings.trail.scaling_enabled);
  changed |= SaveValue(DISTANCE_RINGS, settings.distance_rings_enabled);
  changed |= SaveValueEnum(GROUND_TRACK, settings.display_ground_track);
  changed |= SaveValue(FLARM_ON_MAP, settings.show_flarm_on_map);
  changed |= SaveValue(FLARM_GAUGE, settings.flarm_gauge_enabled);
  changed |= SaveValueEnum(REACH_DISPLAY, settings.final_glide_terrain);
  changed |= SaveValue(THERMAL_BAND, settings.show_thermal_profile);
  changed |= SaveValue(VARIO_BAR, settings.vario_bar_enabled);
  changed |= SaveValue(DETOUR_COST_MARKERS,
                       settings.detour_cost_markers_enabled);
  changed |= SaveValueEnum(WIND_ARROW, settings.wind_arrow_style);
  changed |= SaveValueEnum(ONLINE_TRAFFIC,
                           settings.online_traffic_map_mode);
  changed |= SaveValueEnum(THERMAL_ASSISTANT,
                           settings.thermal_assistant_position);
  changed |= SaveValue(TURN_BACK_MARKER,
                       settings.turn_back_marker_enabled);
  changed |= SaveValue(MENU_BUTTON, settings.show_menu_button);
  changed |= SaveValue(ZOOM_BUTTON, settings.show_zoom_button);
  changed |= SaveValue(QUICKMENU_BUTTON, settings.show_quickmenu_button);
#ifdef HAVE_TRACKING
  changed |= SaveValue(CLOUD_SHOW_THERMALS, settings.cloud_show_thermals);
#endif
#ifdef HAVE_HTTP
  changed |= SaveValue(THERMAL_INFORMATION_MAP,
                       settings.enable_thermal_information_map);
#endif

  return changed;
}

void
MapElementSetConfigWidget::LoadSettings(const MapElementSet &settings) noexcept
{
  LoadValueEnum(FINAL_GLIDE_BAR, settings.final_glide_bar_display_mode);
  LoadValue(FINAL_GLIDE_BAR_MC0, settings.final_glide_bar_mc0_enabled);
  LoadValueEnum(TRAIL_LENGTH, settings.trail.length);
  LoadValue(TRAIL_DRIFT, settings.trail.wind_drift_enabled);
  LoadValueEnum(TRAIL_TYPE, settings.trail.type);
  LoadValue(TRAIL_SCALED, settings.trail.scaling_enabled);
  LoadValue(DISTANCE_RINGS, settings.distance_rings_enabled);
  LoadValueEnum(GROUND_TRACK, settings.display_ground_track);
  LoadValue(FLARM_ON_MAP, settings.show_flarm_on_map);
  LoadValue(FLARM_GAUGE, settings.flarm_gauge_enabled);
  LoadValueEnum(REACH_DISPLAY, settings.final_glide_terrain);
  LoadValue(THERMAL_BAND, settings.show_thermal_profile);
  LoadValue(VARIO_BAR, settings.vario_bar_enabled);
  LoadValue(DETOUR_COST_MARKERS, settings.detour_cost_markers_enabled);
  LoadValueEnum(WIND_ARROW, settings.wind_arrow_style);
  LoadValueEnum(ONLINE_TRAFFIC, settings.online_traffic_map_mode);
  LoadValueEnum(THERMAL_ASSISTANT, settings.thermal_assistant_position);
  LoadValue(TURN_BACK_MARKER, settings.turn_back_marker_enabled);
  LoadValue(MENU_BUTTON, settings.show_menu_button);
  LoadValue(ZOOM_BUTTON, settings.show_zoom_button);
  LoadValue(QUICKMENU_BUTTON, settings.show_quickmenu_button);
#ifdef HAVE_TRACKING
  LoadValue(CLOUD_SHOW_THERMALS, settings.cloud_show_thermals);
#endif
#ifdef HAVE_HTTP
  LoadValue(THERMAL_INFORMATION_MAP,
            settings.enable_thermal_information_map);
#endif

  UpdateFinalGlideBarControls();
  UpdateTrailControls();
}

bool
MapElementSetConfigWidget::Save(bool &_changed) noexcept
{
  bool changed = false;

  if (allow_name_change) {
    const char *new_name = GetValueString(NAME);
    if (!StringIsEqual(new_name, data.name)) {
      data.name = new_name;
      changed = true;
    }
  }

  changed |= SaveSettings(data);

  if (data.display_overrides != display_overrides) {
    data.display_overrides = display_overrides;
    changed = true;
  }

  _changed |= changed;
  return true;
}

void
MapElementSetConfigWidget::Copy() noexcept
{
  clipboard = data;
  SaveSettings(clipboard);
  clipboard.display_overrides = display_overrides;
  clipboard_valid = true;
}

void
MapElementSetConfigWidget::Paste()
{
  if (!clipboard_valid)
    return;

  if (ShowMessageBox(_("Overwrite all map elements in this set?"),
                     _("Map element paste set"),
                     MB_YESNO | MB_ICONQUESTION) != IDYES)
    return;

  LoadSettings(clipboard);
  display_overrides = clipboard.display_overrides;
}

void
MapElementSetConfigWidget::EditDisplayOverrides()
{
  ShowElementSetDisplayOverridesDialog(
    UIGlobals::GetMainWindow(), GetLook(), display_overrides,
    GetMapDisplaySettingCatalog(), nullptr);
}

void
MapElementSetConfigWidget::OnModified(DataField &df) noexcept
{
  if (IsDataField(FINAL_GLIDE_BAR, df))
    UpdateFinalGlideBarControls();
  else if (IsDataField(TRAIL_LENGTH, df))
    UpdateTrailControls();
}

bool
dlgConfigMapElementsShowModal(SingleWindow &parent,
                              const DialogLook &dialog_look,
                              MapElementSet &data,
                              bool allow_name_change)
{
  auto widget = std::make_unique<MapElementSetConfigWidget>(
    dialog_look, data, allow_name_change);
  auto *const widget_ptr = widget.get();
  WidgetDialog dialog(
    WidgetDialog::Full{}, parent, dialog_look, _("Map Element Set"),
    new VScrollWidget(std::move(widget), dialog_look));
  Button *paste_button = nullptr;
  dialog.AddButton(_("Copy Set"), [widget_ptr, &paste_button](){
    widget_ptr->Copy();
    paste_button->SetEnabled(true);
  });
  paste_button = dialog.AddButton(_("Paste Set"),
                                  [widget_ptr](){ widget_ptr->Paste(); });
  paste_button->SetEnabled(clipboard_valid);
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  return dialog.ShowModal() == mrOK && dialog.GetChanged();
}
