// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <type_traits>

/**
 * Stable identifier for a map-display setting.
 *
 * Catalog entries assign explicit values; array positions must never be used
 * as persisted identifiers.
 */
struct DisplaySettingKey {
  uint16_t value;

  constexpr auto operator<=>(const DisplaySettingKey &) const noexcept = default;
};

enum class DisplaySettingGroup : uint8_t {
  TERRAIN,
  ORIENTATION,
  WAYPOINTS,
  AIRSPACE,
};

enum class DisplaySettingValueType : uint8_t {
  BOOLEAN,
  ENUM,
  INTEGER,
};

/**
 * Type-erased value used by the descriptor catalog and sparse overrides.
 * The owning descriptor supplies the type and validation rules.
 */
struct DisplaySettingValue {
  int32_t value;

  static constexpr DisplaySettingValue Boolean(bool value) noexcept {
    return {value ? 1 : 0};
  }

  static constexpr DisplaySettingValue Enum(int32_t value) noexcept {
    return {value};
  }

  static constexpr DisplaySettingValue Integer(int32_t value) noexcept {
    return {value};
  }

  constexpr bool AsBoolean() const noexcept {
    return value != 0;
  }

  constexpr auto operator<=>(const DisplaySettingValue &) const noexcept = default;
};

/** One choice shown by an enum display-setting editor. */
struct DisplaySettingEnumChoice {
  int32_t value;
  const char *label;
  const char *help;
};

enum class DisplaySettingEffect : uint16_t {
  NONE = 0,
  REDRAW = 1u << 0,
  TERRAIN_CACHE = 1u << 1,
  PROJECTION = 1u << 2,
  WAYPOINT_LOOK = 1u << 3,
  AIRSPACE_COMPUTER = 1u << 4,
};

using DisplaySettingEffects = uint16_t;

constexpr DisplaySettingEffects
ToDisplaySettingEffects(DisplaySettingEffect effect) noexcept
{
  return static_cast<DisplaySettingEffects>(effect);
}

struct DisplaySettingAccessor {
  DisplaySettingValue (*get_global)(DisplaySettingKey key) noexcept;
  bool (*set_global)(DisplaySettingKey key,
                     DisplaySettingValue value) noexcept;
  bool (*set_effective)(DisplaySettingKey key,
                        DisplaySettingValue value) noexcept;
};

struct DisplaySettingDescriptor {
  using Validator = bool (*)(DisplaySettingValue value) noexcept;

  DisplaySettingKey key;
  DisplaySettingGroup group;
  const char *profile_suffix;
  const char *label;
  const char *help;
  DisplaySettingValueType value_type;
  DisplaySettingValue minimum;
  DisplaySettingValue maximum;
  Validator validator;

  /**
   * Settings are not offered as element-set overrides unless a catalog entry
   * opts in explicitly.
   */
  bool element_set_overwritable = false;

  /** Editor metadata; used only when value_type is ENUM/INTEGER. */
  const DisplaySettingEnumChoice *enum_choices = nullptr;
  uint16_t enum_choice_count = 0;
  int32_t integer_step = 1;

  DisplaySettingAccessor accessor{};
  DisplaySettingEffects effects =
    ToDisplaySettingEffects(DisplaySettingEffect::NONE);

  constexpr bool IsValid(DisplaySettingValue candidate) const noexcept {
    bool valid = false;

    switch (value_type) {
    case DisplaySettingValueType::BOOLEAN:
      valid = candidate.value == 0 || candidate.value == 1;
      break;

    case DisplaySettingValueType::ENUM:
      if (candidate.value < minimum.value || candidate.value > maximum.value)
        break;

      if (enum_choices == nullptr) {
        valid = true;
        break;
      }

      for (uint16_t i = 0; i < enum_choice_count; ++i)
        if (enum_choices[i].value == candidate.value) {
          valid = true;
          break;
        }
      break;

    case DisplaySettingValueType::INTEGER:
      valid = candidate.value >= minimum.value &&
        candidate.value <= maximum.value;
      break;
    }

    return valid && (validator == nullptr || validator(candidate));
  }
};

struct ElementSetDisplayOverride {
  DisplaySettingKey key;
  DisplaySettingValue value;

  constexpr auto operator<=>(const ElementSetDisplayOverride &) const noexcept =
    default;
};

enum class SetDisplayOverrideResult : uint8_t {
  UNCHANGED,
  ADDED,
  REPLACED,
  NOT_OVERWRITABLE,
  INVALID_VALUE,
  FULL,
};

/**
 * Sparse display-setting overrides owned by one MapElementSet.
 *
 * Entries are kept in key order, making equality independent of insertion
 * order.  The capacity exceeds the first catalog's planned 105 settings; a
 * catalog-size assertion will guard this bound when that catalog is added.
 */
class ElementSetDisplayOverrides {
public:
  static constexpr std::size_t MAX_OVERRIDES = 128;

  using value_type = ElementSetDisplayOverride;
  using iterator = value_type *;
  using const_iterator = const value_type *;

private:
  std::size_t count;
  std::array<value_type, MAX_OVERRIDES> entries;

public:
  void Clear() noexcept {
    count = 0;
  }

  constexpr bool IsEmpty() const noexcept {
    return count == 0;
  }

  constexpr std::size_t Size() const noexcept {
    return count;
  }

  static constexpr std::size_t Capacity() noexcept {
    return MAX_OVERRIDES;
  }

  constexpr const_iterator begin() const noexcept {
    return entries.data();
  }

  constexpr const_iterator end() const noexcept {
    return entries.data() + count;
  }

  constexpr iterator begin() noexcept {
    return entries.data();
  }

  constexpr iterator end() noexcept {
    return entries.data() + count;
  }

  const ElementSetDisplayOverride *Find(DisplaySettingKey key) const noexcept {
    for (const auto &item : *this) {
      if (item.key == key)
        return &item;

      if (item.key > key)
        break;
    }

    return nullptr;
  }

  ElementSetDisplayOverride *Find(DisplaySettingKey key) noexcept {
    for (auto &item : *this) {
      if (item.key == key)
        return &item;

      if (item.key > key)
        break;
    }

    return nullptr;
  }

  const DisplaySettingValue *Get(DisplaySettingKey key) const noexcept {
    const auto *item = Find(key);
    return item != nullptr ? &item->value : nullptr;
  }

  SetDisplayOverrideResult
  Set(const DisplaySettingDescriptor &descriptor,
      DisplaySettingValue value) noexcept {
    if (!descriptor.element_set_overwritable)
      return SetDisplayOverrideResult::NOT_OVERWRITABLE;

    if (!descriptor.IsValid(value))
      return SetDisplayOverrideResult::INVALID_VALUE;

    std::size_t position = 0;
    while (position < count && entries[position].key < descriptor.key)
      ++position;

    if (position < count && entries[position].key == descriptor.key) {
      if (entries[position].value == value)
        return SetDisplayOverrideResult::UNCHANGED;

      entries[position].value = value;
      return SetDisplayOverrideResult::REPLACED;
    }

    if (count == entries.size())
      return SetDisplayOverrideResult::FULL;

    for (std::size_t i = count; i > position; --i)
      entries[i] = entries[i - 1];

    entries[position] = {descriptor.key, value};
    ++count;
    return SetDisplayOverrideResult::ADDED;
  }

  bool Remove(DisplaySettingKey key) noexcept {
    std::size_t position = 0;
    while (position < count && entries[position].key < key)
      ++position;

    if (position == count || entries[position].key != key)
      return false;

    for (std::size_t i = position + 1; i < count; ++i)
      entries[i - 1] = entries[i];

    --count;
    return true;
  }

  constexpr bool
  operator==(const ElementSetDisplayOverrides &other) const noexcept {
    if (count != other.count)
      return false;

    for (std::size_t i = 0; i < count; ++i)
      if (entries[i] != other.entries[i])
        return false;

    return true;
  }
};

static_assert(std::is_trivial_v<DisplaySettingKey>);
static_assert(std::is_trivial_v<DisplaySettingValue>);
static_assert(std::is_trivial_v<ElementSetDisplayOverride>);
static_assert(std::is_trivial_v<ElementSetDisplayOverrides>);
