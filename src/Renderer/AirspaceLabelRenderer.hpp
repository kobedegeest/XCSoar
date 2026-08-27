// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "AirspaceLabelList.hpp"
#include "AirspaceLabelPlacementCache.hpp"
#include "Engine/Airspace/Predicate/AirspacePredicate.hpp"
#include "ui/dim/Rect.hpp"
#include "util/Serial.hpp"

#include <chrono>
#include <cstdint>
#include <optional>

struct AirspaceLook;
struct MoreData;
struct DerivedInfo;
struct AirspaceComputerSettings;
struct AirspaceRendererSettings;
struct AirspaceWarningConfig;
class Airspaces;
class ProtectedAirspaceWarningManager;
class Canvas;
class LabelBlock;
class WindowProjection;
class Font;
struct PixelPoint;
struct AirspaceLabelLayout;

class AirspaceLabelRenderer
{
  const AirspaceLook &look;
  const Airspaces *airspaces = nullptr;
  const ProtectedAirspaceWarningManager *warning_manager = nullptr;

  AirspaceLabelPlacementCache placement_cache;
  const Airspaces *placement_cache_airspaces = nullptr;
  Serial placement_cache_serial;
  PixelRect placement_cache_screen_rect{};
  const Font *placement_cache_font = nullptr;
  unsigned placement_cache_font_height = 0;
  unsigned placement_cache_scale = 0;
  unsigned placement_cache_font_scale = 0;
  unsigned placement_cache_text_padding = 0;
  bool placement_cache_landscape = false;
  std::uint64_t placement_cache_eligibility = 0;
  bool placement_cache_context_valid = false;

  /**
   * Candidate changes are limited to one complete layout every one seconds.
   */
  static constexpr auto PLACEMENT_DECISION_INTERVAL =
    std::chrono::seconds{1};

public:
  explicit AirspaceLabelRenderer(const AirspaceLook &_look) noexcept
    :look(_look) {}

  const AirspaceLook &GetLook() const noexcept {
    return look;
  }

  const Airspaces *GetAirspaces() const noexcept {
    return airspaces;
  }

  const ProtectedAirspaceWarningManager *GetWarningManager() const noexcept {
    return warning_manager;
  }

  void SetAirspaces(const Airspaces *_airspaces) noexcept {
    airspaces = _airspaces;
    InvalidatePlacementCache();
  }

  void SetAirspaceWarnings(
    const ProtectedAirspaceWarningManager *_warning_manager) noexcept {
    warning_manager = _warning_manager;
    InvalidatePlacementCache();
  }

  void Clear() noexcept {
    airspaces = nullptr;
    warning_manager = nullptr;
    InvalidatePlacementCache();
  }

private:
  void InvalidatePlacementCache() noexcept;

  [[gnu::pure]]
  bool IsPlacementCacheCurrent(const PixelRect &screen_rect,
                               std::uint64_t eligibility) const noexcept;

  void UpdatePlacementCacheContext(const PixelRect &screen_rect,
                                   std::uint64_t eligibility) noexcept;

  void DrawInternal(Canvas &canvas,
                    const WindowProjection &projection,
                    AirspacePredicate visible,
                    const AirspaceRendererSettings &settings,
                    const AirspaceWarningConfig &config,
                    bool draw_altitude_labels,
                    bool draw_notam_labels,
                    LabelBlock *label_block) noexcept;

  bool DrawLabel(Canvas &canvas, PixelPoint anchor,
                 const PixelRect &map_rect,
                 const AirspaceLabelList::Label &label,
                 const AirspaceRendererSettings &settings,
                 LabelBlock *label_block,
                 AirspaceLabelLayout &layout,
                 std::optional<AirspaceLabelCandidate> preferred_candidate,
                 bool allow_fallback,
                 AirspaceLabelCandidate &candidate_index) noexcept;

public:
  /**
   * Draw labels that are visible according to standard rules.
   *
   * @param label_block Optional label block for overlap prevention;
   * nullptr to skip overlap checking.
   */
  void Draw(Canvas &canvas,
            const WindowProjection &projection,
            const MoreData &basic, const DerivedInfo &calculated,
            const AirspaceComputerSettings &computer_settings,
            const AirspaceRendererSettings &settings,
            LabelBlock *label_block = nullptr) noexcept;
};
