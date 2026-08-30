// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "dlgConfigMapElements.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Widget/VScrollWidget.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "MapElementSettings.hpp"
#include "Language/Language.hpp"
#include "util/StringAPI.hxx"

#include <memory>
#include <utility>

using namespace UI;

class MapElementSetConfigWidget final
  : public RowFormWidget, private DataFieldListener {
  enum Controls {
    NAME,
    FINAL_GLIDE_BAR,
    FINAL_GLIDE_BAR_MC0,
    TRAIL_LENGTH,
    TRAIL_DRIFT,
    TRAIL_TYPE,
    TRAIL_SCALED,
    DISTANCE_RINGS,
    GROUND_TRACK,
    FLARM_ON_MAP,
    FLARM_GAUGE,
    REACH_DISPLAY,
    THERMAL_BAND,
    VARIO_BAR,
  };

  MapElementSet &data;
  const bool allow_name_change;

  void UpdateFinalGlideBarControls() noexcept;
  void UpdateTrailControls() noexcept;

public:
  MapElementSetConfigWidget(const DialogLook &look, MapElementSet &_data,
                            bool _allow_name_change) noexcept
    :RowFormWidget(look), data(_data),
     allow_name_change(_allow_name_change) {}

  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

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

  AddEnum(_("Final glide bar"),
          _("If set to \"On\" the final glide will always be shown, if set to \"Auto\" it will be shown when approaching the final glide possibility."),
          final_glide_bar_display_mode_list,
          static_cast<unsigned>(data.final_glide_bar_display_mode), this);
  AddBoolean(_("Final glide bar MC0"),
             _("Show a second arrow indicating the height required to reach the final waypoint at MC zero."),
             data.final_glide_bar_mc0_enabled);

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

  AddBoolean(C_("Setting", "Distance rings"),
             _("Display distance rings around the aircraft on the map."),
             data.distance_rings_enabled);
  AddEnum(_("Ground track"),
          _("Display the ground track as a grey line on the map."),
          ground_track_mode_list,
          static_cast<unsigned>(data.display_ground_track));
  AddBoolean(_("FLARM Traffic"),
             _("Enable the display of FLARM traffic on the map window."),
             data.show_flarm_on_map);
  AddBoolean(_("FLARM Radar"),
             _("Enable the display of the FLARM radar gauge."),
             data.flarm_gauge_enabled);
  AddEnum(_("Reach display"), nullptr, final_glide_terrain_list,
          static_cast<unsigned>(data.final_glide_terrain));
  AddBoolean(_("Thermal Band"),
             _("Enable the thermal profile display on the map."),
             data.show_thermal_profile);
  AddBoolean(_("Vario bar"), _("Show the vario bar."),
             data.vario_bar_enabled);

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

  changed |= SaveValueEnum(FINAL_GLIDE_BAR,
                           data.final_glide_bar_display_mode);
  changed |= SaveValue(FINAL_GLIDE_BAR_MC0,
                       data.final_glide_bar_mc0_enabled);
  changed |= SaveValueEnum(TRAIL_LENGTH, data.trail.length);
  changed |= SaveValue(TRAIL_DRIFT, data.trail.wind_drift_enabled);
  changed |= SaveValueEnum(TRAIL_TYPE, data.trail.type);
  changed |= SaveValue(TRAIL_SCALED, data.trail.scaling_enabled);
  changed |= SaveValue(DISTANCE_RINGS, data.distance_rings_enabled);
  changed |= SaveValueEnum(GROUND_TRACK, data.display_ground_track);
  changed |= SaveValue(FLARM_ON_MAP, data.show_flarm_on_map);
  changed |= SaveValue(FLARM_GAUGE, data.flarm_gauge_enabled);
  changed |= SaveValueEnum(REACH_DISPLAY, data.final_glide_terrain);
  changed |= SaveValue(THERMAL_BAND, data.show_thermal_profile);
  changed |= SaveValue(VARIO_BAR, data.vario_bar_enabled);

  _changed |= changed;
  return true;
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
  WidgetDialog dialog(
    WidgetDialog::Full{}, parent, dialog_look, _("Map Element Set"),
    new VScrollWidget(std::move(widget), dialog_look));
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  return dialog.ShowModal() == mrOK && dialog.GetChanged();
}
