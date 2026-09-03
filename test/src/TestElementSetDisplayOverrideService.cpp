// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "MapDisplay/ElementSetDisplayOverrideService.hpp"
#include "TestUtil.hpp"

#include <array>

static bool global_a;
static int global_b;
static bool global_orientation;

static bool effective_a;
static int effective_b;
static bool effective_orientation;

static unsigned set_a_count;
static unsigned set_b_count;
static unsigned set_orientation_count;
static unsigned final_count;
static std::array<unsigned, 4> group_count;
static std::array<DisplaySettingEffects, 4> group_effects;

static DisplaySettingValue GetGlobalA(DisplaySettingKey) noexcept {
  return DisplaySettingValue::Boolean(global_a);
}

static DisplaySettingValue GetGlobalB(DisplaySettingKey) noexcept {
  return DisplaySettingValue::Integer(global_b);
}

static DisplaySettingValue GetGlobalOrientation(DisplaySettingKey) noexcept {
  return DisplaySettingValue::Boolean(global_orientation);
}

static bool SetGlobalA(DisplaySettingKey, DisplaySettingValue value) noexcept {
  global_a = value.AsBoolean();
  return true;
}

static bool SetGlobalB(DisplaySettingKey, DisplaySettingValue value) noexcept {
  global_b = value.value;
  return true;
}

static bool SetGlobalOrientation(DisplaySettingKey,
                                 DisplaySettingValue value) noexcept {
  global_orientation = value.AsBoolean();
  return true;
}

static bool SetEffectiveA(DisplaySettingKey,
                          DisplaySettingValue value) noexcept {
  effective_a = value.AsBoolean();
  ++set_a_count;
  return true;
}

static bool SetEffectiveB(DisplaySettingKey,
                          DisplaySettingValue value) noexcept {
  effective_b = value.value;
  ++set_b_count;
  return true;
}

static bool SetEffectiveOrientation(DisplaySettingKey,
                                    DisplaySettingValue value) noexcept {
  effective_orientation = value.AsBoolean();
  ++set_orientation_count;
  return true;
}

static void
ApplyGroup(DisplaySettingGroup group, DisplaySettingEffects effects) noexcept
{
  const auto index = static_cast<std::size_t>(group);
  ++group_count[index];
  group_effects[index] = effects;
}

static void FinalNotification() noexcept {
  ++final_count;
}

static constexpr DisplaySettingDescriptor descriptors[] = {
  {
    DisplaySettingKey{1}, DisplaySettingGroup::TERRAIN,
    "A", "A", nullptr, DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr, true, nullptr, 0, 1,
    {GetGlobalA, SetGlobalA, SetEffectiveA},
    ToDisplaySettingEffects(DisplaySettingEffect::REDRAW),
  },
  {
    DisplaySettingKey{2}, DisplaySettingGroup::TERRAIN,
    "B", "B", nullptr, DisplaySettingValueType::INTEGER,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(100),
    nullptr, true, nullptr, 0, 1,
    {GetGlobalB, SetGlobalB, SetEffectiveB},
    ToDisplaySettingEffects(DisplaySettingEffect::TERRAIN_CACHE),
  },
  {
    DisplaySettingKey{3}, DisplaySettingGroup::ORIENTATION,
    "Orientation", "Orientation", nullptr,
    DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr, true, nullptr, 0, 1,
    {GetGlobalOrientation, SetGlobalOrientation, SetEffectiveOrientation},
    ToDisplaySettingEffects(DisplaySettingEffect::PROJECTION),
  },
};

static void
ResetState() noexcept
{
  global_a = false;
  global_b = 10;
  global_orientation = true;
  effective_a = false;
  effective_b = 0;
  effective_orientation = false;
  set_a_count = 0;
  set_b_count = 0;
  set_orientation_count = 0;
  final_count = 0;
  group_count.fill(0);
  group_effects.fill(0);
}

static void
TestResolutionAndCoalescing()
{
  ResetState();
  ElementSetDisplayOverrideService service{
    descriptors, ApplyGroup, FinalNotification,
  };

  ok1(!service.IsInitialised());
  ok1(service.Initialise());
  ok1(service.IsInitialised());
  ok1(set_a_count == 1);
  ok1(set_b_count == 1);
  ok1(set_orientation_count == 1);
  ok1(!effective_a);
  ok1(effective_b == 10);
  ok1(effective_orientation);
  ok1(group_count[0] == 1);
  ok1(group_count[1] == 1);
  ok1(group_effects[0] ==
      (ToDisplaySettingEffects(DisplaySettingEffect::REDRAW) |
       ToDisplaySettingEffects(DisplaySettingEffect::TERRAIN_CACHE)));
  ok1(group_effects[1] ==
      ToDisplaySettingEffects(DisplaySettingEffect::PROJECTION));
  ok1(final_count == 1);

  ElementSetDisplayOverrides empty{};
  empty.Clear();
  ok1(!service.Apply(empty));
  ok1(set_a_count == 1);
  ok1(set_b_count == 1);
  ok1(set_orientation_count == 1);
  ok1(final_count == 1);

  ElementSetDisplayOverrides overridden{};
  overridden.Clear();
  ok1(overridden.Set(descriptors[0], DisplaySettingValue::Boolean(true)) ==
      SetDisplayOverrideResult::ADDED);
  ok1(service.Apply(overridden));
  ok1(effective_a);
  ok1(set_a_count == 2);
  ok1(set_b_count == 1);
  ok1(group_count[0] == 2);
  ok1(group_count[1] == 1);
  ok1(final_count == 2);

  ok1(service.SetGlobalValue(descriptors[1].key,
                             DisplaySettingValue::Integer(25)));
  ok1(effective_b == 25);
  ok1(set_b_count == 2);
  ok1(effective_a);
  ok1(group_count[0] == 3);
  ok1(final_count == 3);

  ok1(service.SetGlobalValue(descriptors[0].key,
                             DisplaySettingValue::Boolean(true)));
  ok1(service.GetGlobalValue(descriptors[0].key)->AsBoolean());
  ok1(effective_a);
  ok1(set_a_count == 2);
  ok1(group_count[0] == 3);
  ok1(final_count == 3);

  ok1(service.SetGlobalValue(descriptors[0].key,
                             DisplaySettingValue::Boolean(false)));
  ok1(!service.GetGlobalValue(descriptors[0].key)->AsBoolean());
  ok1(effective_a);
  ok1(set_a_count == 2);
  ok1(final_count == 3);

  ok1(service.Apply(empty));
  ok1(!effective_a);
  ok1(set_a_count == 3);
  ok1(group_count[0] == 4);
  ok1(final_count == 4);
  ok1(!service.Apply(empty));

  ok1(!service.SetGlobalValue(descriptors[1].key,
                              DisplaySettingValue::Integer(101)));
  ok1(service.GetGlobalValue(descriptors[1].key)->value == 25);
  ok1(effective_b == 25);
}

static void
TestInvalidCatalogIsAtomic()
{
  constexpr DisplaySettingDescriptor invalid_descriptor{
    DisplaySettingKey{50}, DisplaySettingGroup::AIRSPACE,
    "Invalid", "Invalid", nullptr, DisplaySettingValueType::BOOLEAN,
    DisplaySettingValue::Integer(0), DisplaySettingValue::Integer(1),
    nullptr, true,
  };

  ElementSetDisplayOverrideService service{
    std::span<const DisplaySettingDescriptor>{&invalid_descriptor, 1},
  };
  ok1(!service.Initialise());
  ok1(!service.IsInitialised());
}

int
main()
{
  plan_tests(55);
  TestResolutionAndCoalescing();
  TestInvalidCatalogIsAtomic();
  return exit_status();
}
