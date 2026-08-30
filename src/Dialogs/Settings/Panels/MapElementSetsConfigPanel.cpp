// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapElementSetsConfigPanel.hpp"
#include "../dlgConfigMapElements.hpp"
#include "Profile/Profile.hpp"
#include "Profile/Current.hpp"
#include "Profile/MapElementConfig.hpp"
#include "PageActions.hpp"
#include "Form/Button.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"

class MapElementSetsConfigPanel final : public RowFormWidget {
public:
  MapElementSetsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

private:
  void OnAction(unsigned i) noexcept;
};

void
MapElementSetsConfigPanel::OnAction(unsigned i) noexcept
{
  MapElementSettings &settings =
    CommonInterface::SetUISettings().map_elements;
  MapElementSet &data = settings.sets[i];

  if (dlgConfigMapElementsShowModal(
        UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(), data,
        i >= MapElementSettings::PREASSIGNED_SETS)) {
    Profile::Save(Profile::map, data, i);
    Profile::Save();
    PageActions::Update();
    static_cast<Button &>(GetRow(i)).SetCaption(gettext(data.name));
  }
}

void
MapElementSetsConfigPanel::Prepare(ContainerWindow &parent,
                                   const PixelRect &rc) noexcept
{
  const MapElementSettings &settings =
    CommonInterface::GetUISettings().map_elements;

  RowFormWidget::Prepare(parent, rc);

  for (unsigned i = 0; i < MapElementSettings::MAX_SETS; ++i) {
    AddButton(gettext(settings.sets[i].name),
              [this, i](){ OnAction(i); });
    if (i >= MapElementSettings::PREASSIGNED_SETS)
      SetExpertRow(i);
  }
}

bool
MapElementSetsConfigPanel::Save([[maybe_unused]] bool &changed) noexcept
{
  return true;
}

std::unique_ptr<Widget>
CreateMapElementSetsConfigPanel()
{
  return std::make_unique<MapElementSetsConfigPanel>();
}
