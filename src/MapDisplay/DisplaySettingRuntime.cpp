// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "DisplaySettingRuntime.hpp"

#include <array>
#include <cstddef>

namespace DisplaySettingRuntime {

namespace {

static constexpr std::size_t GROUP_COUNT = 4;

struct HandlerSlot {
  GroupHandler handler{};
  bool registered = false;
};

static std::array<HandlerSlot, GROUP_COUNT> handlers;

static constexpr std::size_t
GetGroupIndex(DisplaySettingGroup group) noexcept
{
  return static_cast<std::size_t>(group);
}

} // namespace

void
Register(DisplaySettingGroup group, GroupHandler handler) noexcept
{
  const auto index = GetGroupIndex(group);
  if (index < handlers.size())
    handlers[index] = {handler, true};
}

bool
LoadGlobalValues() noexcept
{
  using Loader = bool (*)() noexcept;
  std::array<Loader, GROUP_COUNT> called{};
  std::size_t called_count = 0;

  for (const auto &slot : handlers) {
    if (!slot.registered || slot.handler.load_global == nullptr)
      continue;

    bool already_called = false;
    for (std::size_t i = 0; i < called_count; ++i)
      already_called |= called[i] == slot.handler.load_global;

    if (already_called)
      continue;

    if (!slot.handler.load_global())
      return false;

    called[called_count++] = slot.handler.load_global;
  }

  return true;
}

DisplaySettingValue
GetGlobalValue(DisplaySettingKey key) noexcept
{
  DisplaySettingValue value{};
  for (const auto &slot : handlers)
    if (slot.registered && slot.handler.get_global != nullptr &&
        slot.handler.get_global(key, value))
      return value;

  return {};
}

bool
SetGlobalValue(DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  for (const auto &slot : handlers)
    if (slot.registered && slot.handler.set_global != nullptr &&
        slot.handler.set_global(key, value))
      return true;

  return false;
}

bool
SetEffectiveValue(DisplaySettingKey key, DisplaySettingValue value) noexcept
{
  for (const auto &slot : handlers)
    if (slot.registered && slot.handler.set_effective != nullptr &&
        slot.handler.set_effective(key, value))
      return true;

  return false;
}

void
ApplyGroup(DisplaySettingGroup group, DisplaySettingEffects effects) noexcept
{
  const auto index = GetGroupIndex(group);
  if (index < handlers.size() && handlers[index].registered &&
      handlers[index].handler.apply_effective != nullptr)
    handlers[index].handler.apply_effective(effects);
}

} // namespace DisplaySettingRuntime
