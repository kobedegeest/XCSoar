// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "AirspaceLabelList.hpp"
#include "ui/dim/Size.hpp"
#include "util/NonCopyable.hpp"
#include "util/StaticArray.hxx"

#include <chrono>
#include <cstdint>

/**
 * Allocation-free state for reusing airspace-label candidate positions.
 *
 * The renderer owns one of these objects.  It supplies the surrounding
 * Airspaces/layout invalidation and calls BeginFreshLayout()/
 * CompleteFreshLayout() around a complete placement decision.  Keeping the
 * timer here makes that decision policy independently testable with injected
 * steady-clock timestamps.
 */
class AirspaceLabelPlacementCache : private NonCopyable {
public:
  static constexpr unsigned capacity = AirspaceLabelList::capacity;
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;

  struct Entry {
    AirspaceLabelList::Identity identity;
    PixelSize size;
    unsigned clearance;
    unsigned candidate_index;
    std::uint64_t last_use;

  private:
    bool in_current_layout;

    friend class AirspaceLabelPlacementCache;
  };

private:
  StaticArray<Entry, capacity> entries;
  std::uint64_t next_use = 0;
  TimePoint last_layout{};
  bool has_layout = false;

  Entry *FindReplacement() noexcept {
    auto *replacement = &entries[0];
    for (auto &entry : entries)
      if (entry.last_use < replacement->last_use ||
          (entry.last_use == replacement->last_use &&
           entry.identity < replacement->identity))
        replacement = &entry;

    return replacement;
  }

  void Touch(Entry &entry) noexcept {
    entry.last_use = ++next_use;
  }

public:
  void Clear() noexcept {
    entries.clear();
    next_use = 0;
    last_layout = {};
    has_layout = false;
  }

  [[nodiscard]] Entry *Find(
    const AirspaceLabelList::Identity identity) noexcept {
    for (auto &entry : entries)
      if (entry.identity == identity)
        return &entry;

    return nullptr;
  }

  [[nodiscard]] const Entry *Find(
    const AirspaceLabelList::Identity identity) const noexcept {
    for (const auto &entry : entries)
      if (entry.identity == identity)
        return &entry;

    return nullptr;
  }

  [[gnu::pure]]
  static bool Matches(const Entry &entry, const PixelSize size,
                      const unsigned clearance) noexcept {
    return entry.size == size && entry.clearance == clearance;
  }

  void MarkUsed(Entry &entry) noexcept {
    Touch(entry);
  }

  template<typename Rep, typename Period>
  [[gnu::pure]]
  bool IsFreshLayoutDue(
    const TimePoint now,
    const std::chrono::duration<Rep, Period> &interval) const noexcept {
    return !has_layout ||
      now >= last_layout +
        std::chrono::duration_cast<Clock::duration>(interval);
  }

  void BeginFreshLayout() noexcept {
    for (auto &entry : entries)
      entry.in_current_layout = false;
  }

  /**
   * Store only placements that have successfully reserved their current-frame
   * collision rectangle.  A full cache uses deterministic LRU replacement.
   */
  void Store(const AirspaceLabelList::Identity identity,
             const PixelSize size, const unsigned clearance,
             const unsigned candidate_index) noexcept {
    auto *entry = Find(identity);
    if (entry == nullptr) {
      if (entries.full())
        entry = FindReplacement();
      else
        entry = &entries.append();
    }

    entry->identity = identity;
    entry->size = size;
    entry->clearance = clearance;
    entry->candidate_index = candidate_index;
    entry->in_current_layout = true;
    Touch(*entry);
  }

  /**
   * Drop entries which were not successfully placed in this fresh layout, and
   * start the decision interval only after the whole decision has completed.
   */
  void CompleteFreshLayout(const TimePoint now) noexcept {
    for (unsigned i = 0; i < entries.size();)
      if (!entries[i].in_current_layout)
        entries.remove(i);
      else
        ++i;

    last_layout = now;
    has_layout = true;
  }
};
