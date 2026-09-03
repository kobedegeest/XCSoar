// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Airspace/AirspaceClassDisplay.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "Look/AirspaceLook.hpp"
#include "Profile/AirspaceConfig.hpp"
#include "Profile/Keys.hpp"
#include "Profile/Map.hpp"
#include "Renderer/AirspaceRendererSettings.hpp"
#include "TestUtil.hpp"

#include <chrono>
#include <cstring>

/* Profile::Load() retains support for legacy indexed colours.  These tests
   do not exercise colours, but its translation unit still needs the table. */
const RGB8Color AirspaceLook::preset_colors[NUMAIRSPACECOLORS]{};

static void
TestClassCatalog()
{
  const auto classes = GetAirspaceClassDisplaySettings();
  ok1(classes.size() == AIRSPACE_CLASS_DISPLAY_SETTING_COUNT);

  for (std::size_t i = 0; i < classes.size(); ++i) {
    const auto &item = classes[i];
    ok1(item.airspace_class != OTHER);
    ok1(static_cast<unsigned>(item.airspace_class) < AIRSPACECLASSCOUNT);
    ok1(item.display_setting_key != 0 &&
        item.override_profile_suffix[0] != '\0' && item.label[0] != '\0');
    ok1(FindAirspaceClassDisplaySetting(item.airspace_class) == &item);

    bool unique = true;
    for (std::size_t j = 0; j < i; ++j)
      unique &= item.airspace_class != classes[j].airspace_class &&
        item.display_setting_key != classes[j].display_setting_key &&
        std::strcmp(item.override_profile_suffix,
                    classes[j].override_profile_suffix) != 0;
    ok1(unique);
  }

  ok1(FindAirspaceClassDisplaySetting(OTHER) == nullptr);
}

static void
TestRendererProfileValidation()
{
  {
    ProfileMap map;
    map.Set(ProfileKeys::AirspaceLabelSelection, 99);
    map.Set(ProfileKeys::AltMode,
            static_cast<unsigned>(AirspaceDisplayMode::INSIDE));
    map.Set(ProfileKeys::ClipAlt, 20001u);
    map.Set(ProfileKeys::AirspaceFillMode, 99);
    map.Set("AirspaceFillMode1", 99);
    map.Set("AirspaceDisplay1", false);

    AirspaceRendererSettings settings;
    settings.SetDefaults();
    Profile::Load(map, settings);
    ok1(settings.label_selection ==
        AirspaceRendererSettings::LabelSelection::NONE);
    ok1(settings.altitude_mode == AirspaceDisplayMode::ALLON);
    ok1(settings.clip_altitude == 1000);
    ok1(settings.fill_mode == AirspaceRendererSettings::FillMode::DEFAULT);
    ok1(settings.classes[RESTRICTED].fill_mode ==
        AirspaceClassRendererSettings::FillMode::PADDING);
    ok1(!settings.classes[RESTRICTED].display);
  }

  {
    ProfileMap map;
    map.SetEnum(ProfileKeys::AirspaceLabelSelection,
                AirspaceRendererSettings::LabelSelection::ALL);
    map.SetEnum(ProfileKeys::AltMode, AirspaceDisplayMode::ALLOFF);
    map.Set(ProfileKeys::ClipAlt, 20000u);
    map.SetEnum(ProfileKeys::AirspaceFillMode,
                AirspaceRendererSettings::FillMode::NONE);
    map.Set(ProfileKeys::AirspaceShowNOTAMLabels, false);
    map.Set(ProfileKeys::AirspaceBlackOutline, true);
    map.Set("AirspaceDisplay1", false);

    AirspaceRendererSettings settings;
    settings.SetDefaults();
    Profile::Load(map, settings);
    ok1(settings.label_selection ==
        AirspaceRendererSettings::LabelSelection::ALL);
    ok1(settings.altitude_mode == AirspaceDisplayMode::ALLOFF);
    ok1(settings.clip_altitude == 20000);
    ok1(settings.fill_mode == AirspaceRendererSettings::FillMode::NONE);
    ok1(!settings.show_notam_labels);
    ok1(settings.black_outline);
    ok1(!settings.classes[RESTRICTED].display);
  }

  {
    ProfileMap map;
    map.Set("AirspaceMode1", 2u);

    AirspaceRendererSettings settings;
    settings.SetDefaults();
    Profile::Load(map, settings);
    ok1(!settings.classes[RESTRICTED].display);
  }
}

static void
TestComputerProfileValidation()
{
  using namespace std::chrono;

  {
    ProfileMap map;
    map.Set(ProfileKeys::AirspaceWarning, false);
    map.Set(ProfileKeys::AltMargin, 10001u);
    map.Set(ProfileKeys::WarningTime, 9u);
    map.Set(ProfileKeys::AcknowledgementTime, 1001u);
    map.Set(ProfileKeys::RepetitiveSound, true);
    map.Set("AirspaceMode1", 1u);

    AirspaceComputerSettings settings;
    settings.SetDefaults();
    Profile::Load(map, settings);
    ok1(!settings.enable_warnings);
    ok1(settings.warnings.altitude_warning_margin == 100);
    ok1(settings.warnings.warning_time == seconds{30});
    ok1(settings.warnings.acknowledgement_time == seconds{30});
    ok1(settings.warnings.repetitive_sound);
    ok1(!settings.warnings.class_warnings[RESTRICTED]);
  }

  {
    ProfileMap map;
    map.Set(ProfileKeys::AltMargin, 10000u);
    map.Set(ProfileKeys::WarningTime, 10u);
    map.Set(ProfileKeys::AcknowledgementTime, 1000u);

    AirspaceComputerSettings settings;
    settings.SetDefaults();
    Profile::Load(map, settings);
    ok1(settings.warnings.altitude_warning_margin == 10000);
    ok1(settings.warnings.warning_time == seconds{10});
    ok1(settings.warnings.acknowledgement_time == seconds{1000});
  }
}

static void
TestWarningDialogProfile()
{
  ProfileMap map;
  bool enabled = true;
  Profile::LoadAirspaceWarningDialog(map, enabled);
  ok1(enabled);

  map.Set(ProfileKeys::AirspaceWarningDialog, false);
  Profile::LoadAirspaceWarningDialog(map, enabled);
  ok1(!enabled);
}

int
main()
{
  plan_tests(27 + 5 * AIRSPACE_CLASS_DISPLAY_SETTING_COUNT);
  TestClassCatalog();
  TestRendererProfileValidation();
  TestComputerProfileValidation();
  TestWarningDialogProfile();
  return exit_status();
}
