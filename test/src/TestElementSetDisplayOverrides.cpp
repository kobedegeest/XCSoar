// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapDisplay/ElementSetDisplayOverrides.hpp"
#include "MapDisplay/DisplaySettingCatalog.hpp"
#include "Airspace/AirspaceClassDisplay.hpp"
#include "Waypoint/MapFilterTypes.hpp"
#include "TestUtil.hpp"

#include <cstring>

static constexpr DisplaySettingDescriptor
MakeBooleanDescriptor(uint16_t key,
                      bool element_set_overwritable=true) noexcept
{
  return {
    DisplaySettingKey{key}, DisplaySettingGroup::TERRAIN,
    "TestBoolean", "Test boolean", nullptr,
    DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr, element_set_overwritable,
  };
}

static constexpr DisplaySettingDescriptor
MakeIntegerDescriptor(uint16_t key, int minimum=0, int maximum=100) noexcept
{
  return {
    DisplaySettingKey{key}, DisplaySettingGroup::ORIENTATION,
    "TestInteger", "Test integer", nullptr,
    DisplaySettingValueType::INTEGER,
    DisplaySettingValue::Integer(minimum),
    DisplaySettingValue::Integer(maximum), nullptr, true,
  };
}

static constexpr bool
RejectTwo(DisplaySettingValue value) noexcept
{
  return value.value != 2;
}

static void
TestDescriptorValidation()
{
  constexpr auto disabled = MakeBooleanDescriptor(9, false);
  ok1(!disabled.element_set_overwritable);

  constexpr auto boolean = MakeBooleanDescriptor(10);
  ok1(boolean.element_set_overwritable);
  ok1(boolean.IsValid(DisplaySettingValue::Boolean(false)));
  ok1(boolean.IsValid(DisplaySettingValue::Boolean(true)));
  ok1(!boolean.IsValid(DisplaySettingValue::Integer(-1)));
  ok1(!boolean.IsValid(DisplaySettingValue::Integer(2)));

  constexpr DisplaySettingDescriptor enumeration{
    DisplaySettingKey{11}, DisplaySettingGroup::WAYPOINTS,
    "TestEnum", "Test enum", nullptr, DisplaySettingValueType::ENUM,
    DisplaySettingValue::Enum(1), DisplaySettingValue::Enum(3), RejectTwo,
    true,
  };
  ok1(enumeration.IsValid(DisplaySettingValue::Enum(1)));
  ok1(!enumeration.IsValid(DisplaySettingValue::Enum(2)));
  ok1(enumeration.IsValid(DisplaySettingValue::Enum(3)));
  ok1(!enumeration.IsValid(DisplaySettingValue::Enum(4)));
}

static void
TestSparseOverrides()
{
  ElementSetDisplayOverrides overrides{};
  overrides.Clear();
  ok1(overrides.IsEmpty());
  ok1(overrides.Size() == 0);
  ok1(overrides.Get(DisplaySettingKey{10}) == nullptr);
  ok1(!overrides.Remove(DisplaySettingKey{10}));

  constexpr auto boolean = MakeBooleanDescriptor(10);
  constexpr auto disabled = MakeBooleanDescriptor(9, false);
  ok1(overrides.Set(disabled, DisplaySettingValue::Boolean(true)) ==
      SetDisplayOverrideResult::NOT_OVERWRITABLE);
  ok1(overrides.IsEmpty());

  ok1(overrides.Set(boolean, DisplaySettingValue::Integer(2)) ==
      SetDisplayOverrideResult::INVALID_VALUE);
  ok1(overrides.IsEmpty());

  ok1(overrides.Set(boolean, DisplaySettingValue::Boolean(false)) ==
      SetDisplayOverrideResult::ADDED);
  ok1(overrides.Size() == 1);
  ok1(overrides.Get(boolean.key) != nullptr);
  ok1(!overrides.Get(boolean.key)->AsBoolean());

  ok1(overrides.Set(boolean, DisplaySettingValue::Boolean(false)) ==
      SetDisplayOverrideResult::UNCHANGED);
  ok1(overrides.Set(boolean, DisplaySettingValue::Boolean(true)) ==
      SetDisplayOverrideResult::REPLACED);
  ok1(overrides.Size() == 1);
  ok1(overrides.Get(boolean.key)->AsBoolean());

  ok1(overrides.Remove(boolean.key));
  ok1(overrides.IsEmpty());
  ok1(!overrides.Remove(boolean.key));
}

static void
TestCanonicalOrderAndEquality()
{
  constexpr auto first = MakeIntegerDescriptor(20);
  constexpr auto second = MakeIntegerDescriptor(21);

  ElementSetDisplayOverrides a{};
  a.Clear();
  ok1(a.Set(second, DisplaySettingValue::Integer(42)) ==
      SetDisplayOverrideResult::ADDED);
  ok1(a.Set(first, DisplaySettingValue::Integer(7)) ==
      SetDisplayOverrideResult::ADDED);

  ElementSetDisplayOverrides b{};
  b.Clear();
  ok1(b.Set(first, DisplaySettingValue::Integer(7)) ==
      SetDisplayOverrideResult::ADDED);
  ok1(b.Set(second, DisplaySettingValue::Integer(42)) ==
      SetDisplayOverrideResult::ADDED);

  ok1(a == b);
  ok1(a.begin()->key == first.key);
  ok1((a.begin() + 1)->key == second.key);

  ok1(b.Set(first, DisplaySettingValue::Integer(8)) ==
      SetDisplayOverrideResult::REPLACED);
  ok1(!(a == b));
}

static void
TestCapacityIsExplicit()
{
  ElementSetDisplayOverrides overrides{};
  overrides.Clear();

  for (std::size_t i = 0; i < overrides.Capacity(); ++i) {
    const auto descriptor =
      MakeIntegerDescriptor(static_cast<uint16_t>(1000 + i));
    ok1(overrides.Set(descriptor, DisplaySettingValue::Integer(1)) ==
        SetDisplayOverrideResult::ADDED);
  }

  ok1(overrides.Size() == overrides.Capacity());
  const auto extra = MakeIntegerDescriptor(2000);
  ok1(overrides.Set(extra, DisplaySettingValue::Integer(1)) ==
      SetDisplayOverrideResult::FULL);
  ok1(overrides.Get(extra.key) == nullptr);
}

static void
TestCatalogWhitelist()
{
  const auto catalog = GetMapDisplaySettingCatalog();
  ok1(catalog.size() == DisplaySettingCatalog::COUNT);
  ok1(catalog.size() <= ElementSetDisplayOverrides::MAX_OVERRIDES);

  std::size_t terrain = 0, orientation = 0, waypoints = 0, airspace = 0;
  std::size_t overwritable = 0;
  for (std::size_t i = 0; i < catalog.size(); ++i) {
    const auto &descriptor = catalog[i];
    const bool expected_overwritable =
      descriptor.group == DisplaySettingGroup::TERRAIN ||
      descriptor.group == DisplaySettingGroup::ORIENTATION ||
      descriptor.group == DisplaySettingGroup::WAYPOINTS ||
      descriptor.group == DisplaySettingGroup::AIRSPACE;
    ok1(descriptor.element_set_overwritable == expected_overwritable);
    ok1(descriptor.profile_suffix != nullptr &&
        descriptor.profile_suffix[0] != '\0');
    ok1((descriptor.accessor.get_global != nullptr) ==
        expected_overwritable);
    ok1((descriptor.accessor.set_global != nullptr) ==
        expected_overwritable);
    ok1((descriptor.accessor.set_effective != nullptr) ==
        expected_overwritable);

    if (descriptor.element_set_overwritable)
      ++overwritable;

    bool key_unique = true;
    bool suffix_unique = true;
    for (std::size_t j = 0; j < i; ++j) {
      key_unique &= descriptor.key != catalog[j].key;
      suffix_unique &= std::strcmp(descriptor.profile_suffix,
                                   catalog[j].profile_suffix) != 0;
    }
    ok1(key_unique);
    ok1(suffix_unique);

    switch (descriptor.group) {
    case DisplaySettingGroup::TERRAIN:
      ++terrain;
      break;
    case DisplaySettingGroup::ORIENTATION:
      ++orientation;
      break;
    case DisplaySettingGroup::WAYPOINTS:
      ++waypoints;
      break;
    case DisplaySettingGroup::AIRSPACE:
      ++airspace;
      break;
    }
  }

  ok1(terrain == DisplaySettingCatalog::TERRAIN_COUNT);
  ok1(orientation == DisplaySettingCatalog::ORIENTATION_COUNT);
  ok1(waypoints == DisplaySettingCatalog::WAYPOINT_COUNT);
  ok1(airspace == DisplaySettingCatalog::AIRSPACE_COUNT);
  ok1(overwritable == DisplaySettingCatalog::TERRAIN_COUNT +
      DisplaySettingCatalog::ORIENTATION_COUNT +
      DisplaySettingCatalog::WAYPOINT_COUNT +
      DisplaySettingCatalog::AIRSPACE_COUNT);

  const auto waypoint_types = GetWaypointMapFilterTypes();
  ok1(waypoint_types.size() ==
      DisplaySettingCatalog::WAYPOINT_TYPE_COUNT);
  for (const auto &item : waypoint_types) {
    const DisplaySettingDescriptor *found = nullptr;
    for (const auto &descriptor : catalog)
      if (descriptor.key.value == item.display_setting_key) {
        found = &descriptor;
        break;
      }

    ok1(found != nullptr);
    ok1(found != nullptr &&
        found->group == DisplaySettingGroup::WAYPOINTS);
    ok1(found != nullptr &&
        found->value_type == DisplaySettingValueType::BOOLEAN);
    ok1(found != nullptr && found->element_set_overwritable);
    ok1(found != nullptr &&
        std::strcmp(found->profile_suffix,
                    item.override_profile_suffix) == 0);
  }

  const auto airspace_classes = GetAirspaceClassDisplaySettings();
  ok1(airspace_classes.size() ==
      DisplaySettingCatalog::AIRSPACE_CLASS_COUNT);
  for (const auto &item : airspace_classes) {
    const DisplaySettingDescriptor *found = nullptr;
    for (const auto &descriptor : catalog)
      if (descriptor.key.value == item.display_setting_key) {
        found = &descriptor;
        break;
      }

    ok1(found != nullptr);
    ok1(found != nullptr &&
        found->group == DisplaySettingGroup::AIRSPACE);
    ok1(found != nullptr &&
        found->value_type == DisplaySettingValueType::BOOLEAN);
    ok1(found != nullptr && found->element_set_overwritable);
    ok1(found != nullptr &&
        std::strcmp(found->profile_suffix,
                    item.override_profile_suffix) == 0);
  }
}

int
main()
{
  plan_tests(50 + ElementSetDisplayOverrides::MAX_OVERRIDES +
             7 * DisplaySettingCatalog::COUNT +
             5 * DisplaySettingCatalog::WAYPOINT_TYPE_COUNT +
             5 * DisplaySettingCatalog::AIRSPACE_CLASS_COUNT);

  TestDescriptorValidation();
  TestSparseOverrides();
  TestCanonicalOrderAndEquality();
  TestCapacityIsExplicit();
  TestCatalogWhitelist();

  return exit_status();
}
