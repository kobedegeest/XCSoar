// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ElementSetDisplayOverrides.hpp"

#include <array>
#include <cstddef>
#include <span>

/**
 * Resolves validated global display values plus the active element set's
 * sparse overrides, then coalesces application by setting group.
 */
class ElementSetDisplayOverrideService {
public:
  using ApplyGroupEffects = void (*)(DisplaySettingGroup group,
                                     DisplaySettingEffects effects) noexcept;
  using FinalNotification = void (*)() noexcept;

private:
  struct Slot {
    const DisplaySettingDescriptor *descriptor;
    DisplaySettingValue global;
    DisplaySettingValue effective;
    bool effective_valid;
  };

  std::span<const DisplaySettingDescriptor> catalog;
  ApplyGroupEffects apply_group_effects;
  FinalNotification final_notification;

  std::array<Slot, ElementSetDisplayOverrides::MAX_OVERRIDES> slots;
  std::size_t count = 0;
  ElementSetDisplayOverrides active_overrides{};
  bool initialised = false;

  Slot *FindSlot(DisplaySettingKey key) noexcept;
  const Slot *FindSlot(DisplaySettingKey key) const noexcept;

public:
  explicit ElementSetDisplayOverrideService(
    std::span<const DisplaySettingDescriptor> _catalog,
    ApplyGroupEffects _apply_group_effects=nullptr,
    FinalNotification _final_notification=nullptr) noexcept
    : catalog(_catalog), apply_group_effects(_apply_group_effects),
      final_notification(_final_notification) {
    active_overrides.Clear();
  }

  /**
   * Capture the validated global baseline through descriptor accessors.
   * This is atomic: an invalid catalog/value leaves the old state untouched.
   */
  bool Initialise() noexcept;

  bool IsInitialised() const noexcept {
    return initialised;
  }

  const DisplaySettingValue *
  GetGlobalValue(DisplaySettingKey key) const noexcept;

  const DisplaySettingValue *
  GetEffectiveValue(DisplaySettingKey key) const noexcept;

  /** Change a global baseline value and immediately reapply the active set. */
  bool SetGlobalValue(DisplaySettingKey key,
                      DisplaySettingValue value) noexcept;

  /** Apply one element set's global-plus-override effective values. */
  bool Apply(const ElementSetDisplayOverrides &overrides) noexcept;
};
