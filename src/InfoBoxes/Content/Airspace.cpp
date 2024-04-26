// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "InfoBoxes/Content/Airspace.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "BackendComponents.hpp"
#include "DataComponents.hpp"
#include "Engine/Airspace/AbstractAirspace.hpp"
#include "Airspace/NearestAirspace.hpp"
#include "Dialogs/Airspace/Airspace.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "Widget/CallbackWidget.hpp"
#include "Language/Language.hpp"
#include "Engine/Airspace/Ptr.hpp"
#include "Airspace/ProtectedAirspaceWarningManager.hpp"

static void
ShowNearHorizontalAirspaceDetails() noexcept
{
  NearestAirspace nearest = NearestAirspace::FindHorizontal(CommonInterface::Basic(),
                                                            backend_components->GetAirspaceWarnings(),
                                                            *data_components->airspaces);
  // auto *warnings = backend_components->GetAirspaceWarnings();
  if (!nearest.IsDefined()) {
    return;
  }
  ConstAirspacePtr nearestPtr(nearest.airspace);
  // Acquire a lease for exclusive access to airspace warnings
  ProtectedAirspaceWarningManager::ExclusiveLease lease(backend_components->GetAirspaceWarnings());
  
  dlgAirspaceDetails(nearestPtr, backend_components->GetAirspaceWarnings());
}

static std::unique_ptr<Widget>
LoadAirspaceHorizontalDetailsPanel([[maybe_unused]] unsigned id) noexcept
{
  return std::make_unique<CallbackWidget>(ShowNearHorizontalAirspaceDetails);
}

#ifdef __clang__
constexpr
#endif
const InfoBoxPanel nearest_airspace_horizontal_infobox_panels[] = {
    { N_("Details"), LoadAirspaceHorizontalDetailsPanel },
    { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxNearestAirspaceHorizontal::GetDialogContent() noexcept
{
  return nearest_airspace_horizontal_infobox_panels;
}

void
InfoBoxNearestAirspaceHorizontal::Update(InfoBoxData &data) noexcept
{
  NearestAirspace nearest = NearestAirspace::FindHorizontal(CommonInterface::Basic(),
                                                            backend_components->GetAirspaceWarnings(),
                                                            *data_components->airspaces);
  if (!nearest.IsDefined()) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromDistance(nearest.distance);
  data.SetComment(nearest.airspace->GetName());
}

void
UpdateInfoBoxNearestAirspaceVertical(InfoBoxData &data) noexcept
{
  NearestAirspace nearest = NearestAirspace::FindVertical(CommonInterface::Basic(),
                                                          CommonInterface::Calculated(),
                                                          backend_components->GetAirspaceWarnings(),
                                                          *data_components->airspaces);
  if (!nearest.IsDefined()) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromArrival(nearest.distance);
  data.SetComment(nearest.airspace->GetName());
}
