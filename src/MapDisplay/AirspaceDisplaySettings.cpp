// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceDisplaySettings.hpp"

#include "DisplaySettingCatalog.hpp"
#include "DisplaySettingRuntime.hpp"
#include "ActionInterface.hpp"
#include "Airspace/AirspaceClassDisplay.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "BackendComponents.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Profile/AirspaceConfig.hpp"
#include "Profile/Current.hpp"
#include "Renderer/AirspaceRendererSettings.hpp"
#include "UISettings.hpp"

namespace {

using namespace DisplaySettingCatalog;
using Duration = AirspaceWarningConfig::Duration;

struct AirspaceBundle {
  AirspaceRendererSettings renderer;
  bool enable_warnings;
  AirspaceWarningConfig warnings;
  bool enable_warning_dialog;
  bool transparency;
};

static AirspaceBundle global_airspace, effective_airspace;

static bool
LoadGlobal() noexcept
{
  AirspaceRendererSettings renderer;
  renderer.SetDefaults();
  Profile::Load(Profile::map, renderer);

  AirspaceComputerSettings computer;
  computer.SetDefaults();
  Profile::Load(Profile::map, computer);

  bool enable_warning_dialog = true;
  Profile::LoadAirspaceWarningDialog(Profile::map,
                                     enable_warning_dialog);

  bool transparency = false;
#if defined(HAVE_HATCHED_BRUSH) && defined(HAVE_ALPHA_BLEND)
  transparency = renderer.transparency;
#endif

  global_airspace = {
    renderer,
    computer.enable_warnings,
    computer.warnings,
    enable_warning_dialog,
    transparency,
  };
  effective_airspace = global_airspace;
  return true;
}

static bool
GetAirspaceGlobal(DisplaySettingKey key,
                  DisplaySettingValue &value) noexcept
{
  if (key == Key::AIRSPACE_DISPLAY)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_airspace.renderer.altitude_mode));
  else if (key == Key::AIRSPACE_LABEL_VISIBILITY)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_airspace.renderer.label_selection));
  else if (key == Key::AIRSPACE_SHOW_NOTAM_LABELS)
    value = DisplaySettingValue::Boolean(
      global_airspace.renderer.show_notam_labels);
  else if (key == Key::AIRSPACE_CLIP_ALTITUDE)
    value = DisplaySettingValue::Integer(
      global_airspace.renderer.clip_altitude);
  else if (key == Key::AIRSPACE_MARGIN)
    value = DisplaySettingValue::Integer(
      global_airspace.warnings.altitude_warning_margin);
  else if (key == Key::AIRSPACE_WARNINGS)
    value = DisplaySettingValue::Boolean(global_airspace.enable_warnings);
  else if (key == Key::AIRSPACE_WARNING_DIALOG)
    value = DisplaySettingValue::Boolean(
      global_airspace.enable_warning_dialog);
  else if (key == Key::AIRSPACE_WARNING_TIME)
    value = DisplaySettingValue::Integer(
      global_airspace.warnings.warning_time.count());
  else if (key == Key::AIRSPACE_REPETITIVE_SOUND)
    value = DisplaySettingValue::Boolean(
      global_airspace.warnings.repetitive_sound);
  else if (key == Key::AIRSPACE_ACKNOWLEDGE_TIME)
    value = DisplaySettingValue::Integer(
      global_airspace.warnings.acknowledgement_time.count());
  else if (key == Key::AIRSPACE_BLACK_OUTLINE)
    value = DisplaySettingValue::Boolean(
      global_airspace.renderer.black_outline);
  else if (key == Key::AIRSPACE_FILL_MODE)
    value = DisplaySettingValue::Enum(
      static_cast<int32_t>(global_airspace.renderer.fill_mode));
  else if (key == Key::AIRSPACE_TRANSPARENCY)
    value = DisplaySettingValue::Boolean(global_airspace.transparency);
  else {
    for (const auto &item : GetAirspaceClassDisplaySettings())
      if (key.value == item.display_setting_key) {
        value = DisplaySettingValue::Boolean(
          global_airspace.renderer.classes[item.airspace_class].display);
        return true;
      }

    return false;
  }

  return true;
}

static bool
SetAirspace(AirspaceBundle &airspace, DisplaySettingKey key,
            DisplaySettingValue value) noexcept
{
  if (key == Key::AIRSPACE_DISPLAY)
    airspace.renderer.altitude_mode =
      static_cast<AirspaceDisplayMode>(value.value);
  else if (key == Key::AIRSPACE_LABEL_VISIBILITY)
    airspace.renderer.label_selection =
      static_cast<AirspaceRendererSettings::LabelSelection>(value.value);
  else if (key == Key::AIRSPACE_SHOW_NOTAM_LABELS)
    airspace.renderer.show_notam_labels = value.AsBoolean();
  else if (key == Key::AIRSPACE_CLIP_ALTITUDE)
    airspace.renderer.clip_altitude = static_cast<unsigned>(value.value);
  else if (key == Key::AIRSPACE_MARGIN)
    airspace.warnings.altitude_warning_margin =
      static_cast<unsigned>(value.value);
  else if (key == Key::AIRSPACE_WARNINGS)
    airspace.enable_warnings = value.AsBoolean();
  else if (key == Key::AIRSPACE_WARNING_DIALOG)
    airspace.enable_warning_dialog = value.AsBoolean();
  else if (key == Key::AIRSPACE_WARNING_TIME)
    airspace.warnings.warning_time = Duration{
      static_cast<unsigned>(value.value)};
  else if (key == Key::AIRSPACE_REPETITIVE_SOUND)
    airspace.warnings.repetitive_sound = value.AsBoolean();
  else if (key == Key::AIRSPACE_ACKNOWLEDGE_TIME)
    airspace.warnings.acknowledgement_time = Duration{
      static_cast<unsigned>(value.value)};
  else if (key == Key::AIRSPACE_BLACK_OUTLINE)
    airspace.renderer.black_outline = value.AsBoolean();
  else if (key == Key::AIRSPACE_FILL_MODE)
    airspace.renderer.fill_mode =
      static_cast<AirspaceRendererSettings::FillMode>(value.value);
  else if (key == Key::AIRSPACE_TRANSPARENCY)
    airspace.transparency = value.AsBoolean();
  else {
    for (const auto &item : GetAirspaceClassDisplaySettings())
      if (key.value == item.display_setting_key) {
        airspace.renderer.classes[item.airspace_class].display =
          value.AsBoolean();
        return true;
      }

    return false;
  }

  return true;
}

static bool
SetAirspaceGlobal(DisplaySettingKey key,
                  DisplaySettingValue value) noexcept
{
  return SetAirspace(global_airspace, key, value);
}

static bool
SetAirspaceEffective(DisplaySettingKey key,
                     DisplaySettingValue value) noexcept
{
  return SetAirspace(effective_airspace, key, value);
}

static void
ApplyAirspace(DisplaySettingEffects effects) noexcept
{
  auto &renderer = CommonInterface::SetMapSettings().airspace;
  renderer.altitude_mode = effective_airspace.renderer.altitude_mode;
  renderer.label_selection = effective_airspace.renderer.label_selection;
  renderer.show_notam_labels =
    effective_airspace.renderer.show_notam_labels;
  renderer.clip_altitude = effective_airspace.renderer.clip_altitude;
  renderer.black_outline = effective_airspace.renderer.black_outline;
  renderer.fill_mode = effective_airspace.renderer.fill_mode;
#if defined(HAVE_HATCHED_BRUSH) && defined(HAVE_ALPHA_BLEND)
  renderer.transparency = effective_airspace.transparency;
#endif
  for (const auto &item : GetAirspaceClassDisplaySettings())
    renderer.classes[item.airspace_class].display =
      effective_airspace.renderer.classes[item.airspace_class].display;

  auto &computer = CommonInterface::SetComputerSettings().airspace;
  computer.enable_warnings = effective_airspace.enable_warnings;
  computer.warnings.altitude_warning_margin =
    effective_airspace.warnings.altitude_warning_margin;
  computer.warnings.warning_time = effective_airspace.warnings.warning_time;
  computer.warnings.repetitive_sound =
    effective_airspace.warnings.repetitive_sound;
  computer.warnings.acknowledgement_time =
    effective_airspace.warnings.acknowledgement_time;

  CommonInterface::SetUISettings().enable_airspace_warning_dialog =
    effective_airspace.enable_warning_dialog;

  if ((effects &
       ToDisplaySettingEffects(DisplaySettingEffect::AIRSPACE_COMPUTER)) != 0 &&
      CommonInterface::main_window != nullptr &&
      backend_components != nullptr &&
      backend_components->calculation_thread != nullptr)
    ActionInterface::SendComputerSettings();
}

} // namespace

void
RegisterAirspaceDisplaySettings() noexcept
{
  DisplaySettingRuntime::Register(DisplaySettingGroup::AIRSPACE, {
    LoadGlobal, GetAirspaceGlobal, SetAirspaceGlobal,
    SetAirspaceEffective, ApplyAirspace,
  });
}

bool
GetGlobalAirspaceClassDisplay(AirspaceClass airspace_class) noexcept
{
  if (static_cast<unsigned>(airspace_class) >= AIRSPACECLASSCOUNT)
    return true;

  return global_airspace.renderer.classes[airspace_class].display;
}
