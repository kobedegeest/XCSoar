// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceClassDisplay.hpp"

#include "Language/Language.hpp"

#include <array>

static constexpr std::array<AirspaceClassDisplaySetting,
                            AIRSPACE_CLASS_DISPLAY_SETTING_COUNT> settings{{
  {RESTRICTED, 0x4101, "AirspaceClassRestricted", N_("Restricted")},
  {PROHIBITED, 0x4102, "AirspaceClassProhibited", N_("Prohibited")},
  {DANGER, 0x4103, "AirspaceClassDanger", N_("Danger Area")},
  {CLASSA, 0x4104, "AirspaceClassA", N_("Class A")},
  {CLASSB, 0x4105, "AirspaceClassB", N_("Class B")},
  {CLASSC, 0x4106, "AirspaceClassC", N_("Class C")},
  {CLASSD, 0x4107, "AirspaceClassD", N_("Class D")},
  {NOGLIDER, 0x4108, "AirspaceClassNoGlider", N_("No Gliders")},
  {CTR, 0x4109, "AirspaceClassCTR", N_("CTR")},
  {WAVE, 0x410a, "AirspaceClassWave", N_("Wave")},
  {AATASK, 0x410b, "AirspaceClassAATask", N_("Task Area")},
  {CLASSE, 0x410c, "AirspaceClassE", N_("Class E")},
  {CLASSF, 0x410d, "AirspaceClassF", N_("Class F")},
  {TMZ, 0x410e, "AirspaceClassTMZ", N_("Transponder Mandatory Zone")},
  {CLASSG, 0x410f, "AirspaceClassG", N_("Class G")},
  {MATZ, 0x4110, "AirspaceClassMATZ",
   N_("Military Aerodrome Traffic Zone")},
  {RMZ, 0x4111, "AirspaceClassRMZ", N_("Radio Mandatory Zone")},
  {UNCLASSIFIED, 0x4112, "AirspaceClassUnclassified", N_("Unclassified")},
  {TMA, 0x4113, "AirspaceClassTMA", N_("TMA")},
  {TRA, 0x4114, "AirspaceClassTRA", N_("Temporary Reserved Airspace")},
  {TSA, 0x4115, "AirspaceClassTSA", N_("Temporary Segregated Area")},
  {FIR, 0x4116, "AirspaceClassFIR", N_("Flight Information Region")},
  {UIR, 0x4117, "AirspaceClassUIR",
   N_("Upper Flight Information Region")},
  {ADIZ, 0x4118, "AirspaceClassADIZ",
   N_("Air Defense Identification Zone")},
  {ATZ, 0x4119, "AirspaceClassATZ", N_("Aerodrome Traffic Zone")},
  {AWY, 0x411a, "AirspaceClassAirway", N_("Airway")},
  {MTR, 0x411b, "AirspaceClassMTR", N_("Military Training Route")},
  {ALERT, 0x411c, "AirspaceClassAlert", N_("Alert Area")},
  {WARNING, 0x411d, "AirspaceClassWarning", N_("Warning Area")},
  {PROTECTED, 0x411e, "AirspaceClassProtected", N_("Protected Area")},
  {HTZ, 0x411f, "AirspaceClassHTZ", N_("Hazardous Area")},
  {GLIDING_SECTOR, 0x4120, "AirspaceClassGlidingSector",
   N_("Gliding Sector")},
  {TRP, 0x4121, "AirspaceClassTRP",
   N_("Temporary Reserved Prohibited Area")},
  {TIZ, 0x4122, "AirspaceClassTIZ", N_("Terminal Information Zone")},
  {TIA, 0x4123, "AirspaceClassTIA",
   N_("Terminal Instrument Approach Procedure Area")},
  {MTA, 0x4124, "AirspaceClassMTA", N_("Military Training Area")},
  {CTA, 0x4125, "AirspaceClassCTA", N_("Control Area")},
  {ACC_SECTOR, 0x4126, "AirspaceClassACCSector",
   N_("Area Control Center Sector")},
  {AERIAL_SPORTING_RECREATIONAL, 0x4127,
   "AirspaceClassAerialSportingRecreational",
   N_("Aerial Sporting Recreational")},
  {OVERFLIGHT_RESTRICTION, 0x4128,
   "AirspaceClassOverflightRestriction", N_("Overflight Restriction")},
  {MRT, 0x4129, "AirspaceClassMRT", N_("Military Restricted Area")},
  {TFR, 0x412a, "AirspaceClassTFR", N_("Temporary Flight Restriction")},
  {VFR_SECTOR, 0x412b, "AirspaceClassVFRSector",
   N_("Visual Flight Rules Sector")},
  {FIS_SECTOR, 0x412c, "AirspaceClassFISSector",
   N_("Flight Information Sector")},
  {LTA, 0x412d, "AirspaceClassLTA", N_("Lower Traffic Area")},
  {UTA, 0x412e, "AirspaceClassUTA", N_("Upper Traffic Area")},
  {ASRA, 0x412f, "AirspaceClassASRA",
   N_("Aerial Sporting Or Recreational Activity")},
  {NOTAM, 0x4130, "AirspaceClassNOTAM", N_("NOTAM Affected Area")},
  {NONE, 0x4131, "AirspaceClassNone", N_("Airspace without type")},
  {TRAFR, 0x4132, "AirspaceClassTRAFR", N_("TRA/TSA Feeding Route")},
  {TRZ, 0x4133, "AirspaceClassTRZ", N_("Transponder Recommended Zone")},
  {VFR_ROUTE, 0x4134, "AirspaceClassVFRRoute",
   N_("Designated Route for VFR")},
}};

std::span<const AirspaceClassDisplaySetting>
GetAirspaceClassDisplaySettings() noexcept
{
  return settings;
}

const AirspaceClassDisplaySetting *
FindAirspaceClassDisplaySetting(AirspaceClass airspace_class) noexcept
{
  for (const auto &item : settings)
    if (item.airspace_class == airspace_class)
      return &item;

  return nullptr;
}
