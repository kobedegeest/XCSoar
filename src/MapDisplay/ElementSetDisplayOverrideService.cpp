// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ElementSetDisplayOverrideService.hpp"

#include <array>

static constexpr std::size_t DISPLAY_SETTING_GROUP_COUNT = 4;

static constexpr std::size_t
GetGroupIndex(DisplaySettingGroup group) noexcept
{
  return static_cast<std::size_t>(group);
}

ElementSetDisplayOverrideService::Slot *
ElementSetDisplayOverrideService::FindSlot(DisplaySettingKey key) noexcept
{
  for (std::size_t i = 0; i < count; ++i)
    if (slots[i].descriptor->key == key)
      return &slots[i];

  return nullptr;
}

const ElementSetDisplayOverrideService::Slot *
ElementSetDisplayOverrideService::FindSlot(DisplaySettingKey key) const noexcept
{
  for (std::size_t i = 0; i < count; ++i)
    if (slots[i].descriptor->key == key)
      return &slots[i];

  return nullptr;
}

bool
ElementSetDisplayOverrideService::Initialise() noexcept
{
  std::array<Slot, ElementSetDisplayOverrides::MAX_OVERRIDES> new_slots;
  std::size_t new_count = 0;

  for (const auto &descriptor : catalog) {
    if (!descriptor.element_set_overwritable)
      continue;

    if (new_count == new_slots.size() ||
        descriptor.accessor.get_global == nullptr ||
        descriptor.accessor.set_effective == nullptr)
      return false;

    for (std::size_t i = 0; i < new_count; ++i)
      if (new_slots[i].descriptor->key == descriptor.key)
        return false;

    const DisplaySettingValue global = descriptor.accessor.get_global();
    if (!descriptor.IsValid(global))
      return false;

    new_slots[new_count++] = {&descriptor, global, {}, false};
  }

  for (std::size_t i = 0; i < new_count; ++i)
    slots[i] = new_slots[i];
  count = new_count;
  initialised = true;

  Apply(active_overrides);
  return true;
}

const DisplaySettingValue *
ElementSetDisplayOverrideService::GetGlobalValue(
  DisplaySettingKey key) const noexcept
{
  const auto *slot = FindSlot(key);
  return slot != nullptr ? &slot->global : nullptr;
}

const DisplaySettingValue *
ElementSetDisplayOverrideService::GetEffectiveValue(
  DisplaySettingKey key) const noexcept
{
  const auto *slot = FindSlot(key);
  return slot != nullptr && slot->effective_valid
    ? &slot->effective
    : nullptr;
}

bool
ElementSetDisplayOverrideService::SetGlobalValue(
  DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  auto *slot = FindSlot(key);
  if (slot == nullptr || !slot->descriptor->IsValid(value))
    return false;

  if (slot->global == value)
    return true;

  slot->global = value;
  Apply(active_overrides);
  return true;
}

bool
ElementSetDisplayOverrideService::Apply(
  const ElementSetDisplayOverrides &overrides) noexcept
{
  if (!initialised)
    return false;

  active_overrides = overrides;

  std::array<DisplaySettingEffects, DISPLAY_SETTING_GROUP_COUNT>
    group_effects{};
  std::array<bool, DISPLAY_SETTING_GROUP_COUNT> group_changed{};
  bool any_changed = false;

  for (std::size_t i = 0; i < count; ++i) {
    auto &slot = slots[i];
    const auto &descriptor = *slot.descriptor;

    DisplaySettingValue next = slot.global;
    if (const auto *override_value = overrides.Get(descriptor.key);
        override_value != nullptr && descriptor.IsValid(*override_value))
      next = *override_value;

    if (slot.effective_valid && slot.effective == next)
      continue;

    descriptor.accessor.set_effective(next);
    slot.effective = next;
    slot.effective_valid = true;

    const std::size_t group_index = GetGroupIndex(descriptor.group);
    if (group_index < group_changed.size()) {
      group_changed[group_index] = true;
      group_effects[group_index] |= descriptor.effects;
    }
    any_changed = true;
  }

  if (!any_changed)
    return false;

  if (apply_group_effects != nullptr)
    for (std::size_t i = 0; i < group_changed.size(); ++i)
      if (group_changed[i])
        apply_group_effects(static_cast<DisplaySettingGroup>(i),
                            group_effects[i]);

  if (final_notification != nullptr)
    final_notification();

  return true;
}
