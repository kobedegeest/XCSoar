// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceLabelRenderer.hpp"
#include "AirspaceLabelPlacement.hpp"
#include "AirspaceRendererSettings.hpp"
#include "Projection/WindowProjection.hpp"
#include "Look/AirspaceLook.hpp"
#include "Airspace/Airspaces.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "Airspace/AirspaceVisibility.hpp"
#include "Airspace/AirspaceWarningCopy.hpp"
#include "Engine/Airspace/AbstractAirspace.hpp"
#include "Airspace/AirspaceClass.hpp"
#include "Formatter/AirspaceFormatter.hpp"
#include "Language/Language.hpp"
#include "Renderer/TextInBox.hpp"
#include "Geo/GeoBounds.hpp"
#include "NMEA/Aircraft.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Font.hpp"
#include "Screen/Layout.hpp"
#include "LogFile.hpp"
#include "util/CharUtil.hxx"
#include "util/StaticArray.hxx"
#include "util/StaticString.hxx"
#include "util/UTF8.hpp"
#include "Sizes.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>

static constexpr double NOTAM_LABEL_MAX_MAP_SCALE = 4000;
static constexpr std::size_t NOTAM_LABEL_MAX_CHARS = 40;
static constexpr unsigned NOTAM_CLUSTER_VISIBLE_LINES = 3;
static constexpr unsigned NOTAM_CLUSTER_LABEL_LINES = 2;
static constexpr unsigned NOTAM_CLUSTER_ANCHOR_OFFSET = 18;
static constexpr unsigned NOTAM_CLUSTER_SCREEN_MARGIN = 12;
static constexpr std::size_t NOTAM_CLUSTER_MAX_COUNT = 64;

struct NotamLabelCluster {
  PixelPoint anchor;
  unsigned count = 0;
  StaticArray<StaticString<64>, NOTAM_CLUSTER_VISIBLE_LINES> labels;
};

struct AirspaceLabelLayout {
  char top_text[NAME_SIZE + 1];
  char base_text[NAME_SIZE + 1];
  PixelSize top_size;
  PixelSize base_size;
  PixelSize visual_size;
  PixelRect visual_rect;
  PixelPoint top_text_origin;
  PixelPoint base_text_origin;
  int separator_y;
  unsigned padding;

  void SetVisualRect(const PixelRect rect) noexcept {
    visual_rect = rect;

    const int text_right = rect.right - int(padding);
    top_text_origin = {text_right - int(top_size.width), rect.top};
    base_text_origin = {text_right - int(base_size.width),
                        rect.bottom - int(base_size.height)};
    separator_y = rect.top + int(visual_size.height) / 2;
  }
};

static AirspaceLabelLayout
MakeAirspaceLabelLayout(Canvas &canvas,
                        const AirspaceLabelList::Label &label) noexcept
{
  AirspaceLabelLayout layout{};
  AirspaceFormatter::FormatAltitudeShort(layout.top_text, label.top, false);
  layout.top_size = canvas.CalcTextSize(layout.top_text);

  AirspaceFormatter::FormatAltitudeShort(layout.base_text, label.base, false);
  layout.base_size = canvas.CalcTextSize(layout.base_text);

  layout.padding = Layout::GetTextPadding();
  layout.visual_size = {
    std::max(layout.top_size.width, layout.base_size.width) +
      2 * layout.padding,
    layout.top_size.height + layout.base_size.height,
  };
  return layout;
}

class AirspaceMapVisible
{
  const AirspaceVisibility visible_predicate;
  const AirspaceWarningCopy &warnings;

public:
  AirspaceMapVisible(const AirspaceComputerSettings &_computer_settings,
                     const AirspaceRendererSettings &_renderer_settings,
                     const AircraftState &_state,
                     const AirspaceWarningCopy &_warnings) noexcept
    :visible_predicate(_computer_settings, _renderer_settings, _state),
     warnings(_warnings) {}

  [[gnu::pure]]
  bool operator()(const AbstractAirspace& airspace) const noexcept {
    return visible_predicate(airspace) ||
      warnings.IsInside(airspace) ||
      warnings.HasWarning(airspace);
  }
};

[[gnu::pure]]
static AirspaceClass
GetAirspaceBorderClass(const AbstractAirspace &airspace,
                       const AirspaceRendererSettings &settings) noexcept
{
  const AirspaceClass type_or_class = airspace.GetTypeOrClass();
  return settings.classes[type_or_class].display
    ? type_or_class
    : airspace.GetClass();
}

static void
AddPlacementEligibilityValue(std::uint64_t &signature,
                             const std::uint64_t value) noexcept
{
  // FNV-1a: this is only an in-process change detector, not a hash exposed
  // outside the renderer.
  signature ^= value;
  signature *= 1099511628211ULL;
}

[[gnu::pure]]
static std::uint64_t
GetPlacementEligibility(const AirspaceRendererSettings &settings,
                        const AirspaceWarningConfig &config) noexcept
{
  std::uint64_t signature = 1469598103934665603ULL;
  AddPlacementEligibilityValue(signature, unsigned(settings.enable));
  AddPlacementEligibilityValue(signature, unsigned(settings.label_selection));
  AddPlacementEligibilityValue(signature, unsigned(settings.altitude_mode));
  AddPlacementEligibilityValue(signature, settings.clip_altitude);
  AddPlacementEligibilityValue(signature, config.altitude_warning_margin);

  for (unsigned i = 0; i < AIRSPACECLASSCOUNT; ++i) {
    AddPlacementEligibilityValue(signature,
                                 unsigned(settings.classes[i].display));
    AddPlacementEligibilityValue(signature,
                                 unsigned(config.class_warnings[i]));
  }

  return signature;
}

[[gnu::pure]]
static bool
Equal(const PixelRect a, const PixelRect b) noexcept
{
  return a.left == b.left && a.top == b.top &&
    a.right == b.right && a.bottom == b.bottom;
}

static StaticString<64>
MakeNotamLabelText(const AbstractAirspace &airspace) noexcept
{
  const char *const name = airspace.GetName();
  if (name == nullptr || name[0] == '\0')
    return StaticString<64>{C_("Status", "NOTAM")};

  if (!ValidateUTF8(name))
    return StaticString<64>{C_("Status", "NOTAM")};

  StaticString<64> label;
  constexpr std::size_t max_bytes_without_ellipsis =
    64 - 1 - 3; // storage minus terminator and "..."
  const std::size_t length =
    TruncateStringUTF8(name, NOTAM_LABEL_MAX_CHARS,
                       max_bytes_without_ellipsis);
  std::copy_n(name, length, label.buffer());
  label.buffer()[length] = '\0';

  // NOTAM names may contain line breaks; replace them before rendering.
  std::replace(label.buffer(), label.buffer() + length, '\r', ' ');
  std::replace(label.buffer(), label.buffer() + length, '\n', ' ');

  if (std::all_of(label.buffer(), label.buffer() + length,
                  [](const char ch) { return IsWhitespaceOrNull(ch); }))
    return StaticString<64>{C_("Status", "NOTAM")};

  if (name[length] != '\0')
    label += "...";

  return label;
}

[[gnu::pure]]
static unsigned
GetNotamClusterDistance() noexcept
{
  return Layout::Scale(20u);
}

[[gnu::pure]]
static unsigned
GetNotamLabelLineStep(const Canvas &canvas) noexcept
{
  return canvas.GetFontHeight() + Layout::GetTextPadding() + Layout::Scale(2u);
}

[[gnu::pure]]
static bool
MatchesNotamCluster(const NotamLabelCluster &cluster,
                    const PixelPoint pos,
                    const unsigned distance) noexcept
{
  return std::abs(pos.x - cluster.anchor.x) <= int(distance) &&
         std::abs(pos.y - cluster.anchor.y) <= int(distance);
}

static void
AddToNotamCluster(NotamLabelCluster &cluster,
                  const StaticString<64> &label) noexcept
{
  ++cluster.count;

  if (cluster.labels.size() < NOTAM_CLUSTER_VISIBLE_LINES)
    cluster.labels.append(label);
}

static StaticString<32>
MakeNotamOverflowLabel(const unsigned hidden_count) noexcept
{
  StaticString<32> summary;
  if (hidden_count == 1)
    summary = _("+ 1 NOTAM");
  else
    summary.Format(_("+ %u NOTAMs"), hidden_count);
  return summary;
}

[[gnu::pure]]
static PixelPoint
AdjustNotamClusterAnchor(const PixelPoint anchor,
                         const PixelRect &screen_rect) noexcept
{
  const int offset = Layout::Scale(NOTAM_CLUSTER_ANCHOR_OFFSET);
  const int margin = Layout::Scale(NOTAM_CLUSTER_SCREEN_MARGIN);

  if (anchor.y - offset > screen_rect.top + margin)
    return anchor.At(0, -offset);

  if (anchor.y + offset < screen_rect.bottom - margin)
    return anchor.At(0, offset);

  return anchor;
}

static void
DrawNotamCluster(Canvas &canvas,
                 const PixelPoint anchor,
                 const NotamLabelCluster &cluster,
                 const TextInBoxMode mode,
                 const PixelRect &screen_rect,
                 LabelBlock *label_block) noexcept
{
  const unsigned visible_lines = std::min(cluster.count,
                                          NOTAM_CLUSTER_VISIBLE_LINES);
  if (visible_lines == 0)
    return;

  const int line_step = GetNotamLabelLineStep(canvas);
  const int first_offset = -int((visible_lines - 1) * line_step) / 2;

  unsigned index = 0;
  const unsigned label_lines = cluster.count > NOTAM_CLUSTER_VISIBLE_LINES
    ? NOTAM_CLUSTER_LABEL_LINES
    : cluster.labels.size();

  for (; index < label_lines; ++index)
    TextInBox(canvas, cluster.labels[index].c_str(),
              anchor.At(0, first_offset + int(index * line_step)),
              mode, screen_rect, label_block);

  if (cluster.count > NOTAM_CLUSTER_VISIBLE_LINES) {
    const auto summary = MakeNotamOverflowLabel(cluster.count - label_lines);
    TextInBox(canvas, summary.c_str(),
              anchor.At(0, first_offset + int(index * line_step)),
              mode, screen_rect, label_block);
  }
}

void
AirspaceLabelRenderer::InvalidatePlacementCache() noexcept
{
  placement_cache.Clear();
  placement_cache_context_valid = false;
}

bool
AirspaceLabelRenderer::IsPlacementCacheCurrent(
  const PixelRect &screen_rect, const std::uint64_t eligibility) const noexcept
{
  return placement_cache_context_valid &&
    placement_cache_airspaces == airspaces &&
    placement_cache_serial == airspaces->GetSerial() &&
    Equal(placement_cache_screen_rect, screen_rect) &&
    placement_cache_font == look.name_font &&
    placement_cache_font_height == look.name_font->GetHeight() &&
    placement_cache_scale == Layout::scale_1024 &&
    placement_cache_font_scale == Layout::font_scale &&
    placement_cache_text_padding == Layout::GetTextPadding() &&
    placement_cache_landscape == Layout::landscape &&
    placement_cache_eligibility == eligibility;
}

void
AirspaceLabelRenderer::UpdatePlacementCacheContext(
  const PixelRect &screen_rect, const std::uint64_t eligibility) noexcept
{
  placement_cache_airspaces = airspaces;
  placement_cache_serial = airspaces->GetSerial();
  placement_cache_screen_rect = screen_rect;
  placement_cache_font = look.name_font;
  placement_cache_font_height = look.name_font->GetHeight();
  placement_cache_scale = Layout::scale_1024;
  placement_cache_font_scale = Layout::font_scale;
  placement_cache_text_padding = Layout::GetTextPadding();
  placement_cache_landscape = Layout::landscape;
  placement_cache_eligibility = eligibility;
  placement_cache_context_valid = true;
}

void
AirspaceLabelRenderer::Draw(Canvas &canvas,
                            const WindowProjection &projection,
                            const MoreData &basic, const DerivedInfo &calculated,
                            const AirspaceComputerSettings &computer_settings,
                            const AirspaceRendererSettings &settings,
                            LabelBlock *label_block) noexcept
{
  const bool draw_altitude_labels =
    settings.label_selection == AirspaceRendererSettings::LabelSelection::ALL;
  const bool draw_notam_labels =
    settings.show_notam_labels &&
    projection.GetMapScale() <= NOTAM_LABEL_MAX_MAP_SCALE;

  if (!draw_altitude_labels)
    InvalidatePlacementCache();

  if ((!draw_altitude_labels && !draw_notam_labels) ||
      airspaces == nullptr || airspaces->IsEmpty()) {
    if (airspaces == nullptr || airspaces->IsEmpty())
      InvalidatePlacementCache();
    return;
  }

  AirspaceWarningCopy awc;
  if (warning_manager != nullptr)
    awc.Visit(*warning_manager);

  const AircraftState aircraft = ToAircraftState(basic, calculated);
  const AirspaceMapVisible visible(computer_settings, settings,
                                   aircraft, awc);

  DrawInternal(canvas,
               projection, visible, settings, computer_settings.warnings,
               draw_altitude_labels, draw_notam_labels, label_block);
}

inline void
AirspaceLabelRenderer::DrawInternal(Canvas &canvas,
                                    const WindowProjection &projection,
                                    AirspacePredicate visible,
                                    const AirspaceRendererSettings &settings,
                                    const AirspaceWarningConfig &config,
                                    const bool draw_altitude_labels,
                                    const bool draw_notam_labels,
                                    LabelBlock *label_block) noexcept
{
  AirspaceLabelList labels;

  if (draw_altitude_labels) {
    for (const auto &i : airspaces->QueryWithinRange(projection.GetGeoScreenCenter(),
                                                     projection.GetScreenDistanceMeters())) {
      const AbstractAirspace &airspace = i.GetAirspace();
      if (visible(airspace))
        labels.Add(airspace.GetCenter(), airspace.GetClass(),
                   GetAirspaceBorderClass(airspace, settings),
                   airspace.GetBase(), airspace.GetTop(),
                   reinterpret_cast<AirspaceLabelList::Identity>(&airspace));
    }

    labels.Sort(config);
  }

  // default paint settings
  canvas.Select(*look.name_font);
  canvas.Select(look.label_brush);
  canvas.SetBackgroundTransparent();

  if (draw_altitude_labels) {
    const PixelRect screen_rect = projection.GetScreenRect();
    const auto eligibility = GetPlacementEligibility(settings, config);
    const bool cache_current =
      IsPlacementCacheCurrent(screen_rect, eligibility);
    if (!cache_current)
      InvalidatePlacementCache();

    const auto now = AirspaceLabelPlacementCache::Clock::now();
    const bool fresh_layout = !cache_current ||
      placement_cache.IsFreshLayoutDue(now, PLACEMENT_DECISION_INTERVAL);
    if (fresh_layout)
      placement_cache.BeginFreshLayout();

    bool geometry_changed = false;
    for (const auto &label : labels) {
      auto layout = MakeAirspaceLabelLayout(canvas, label);
      auto *const cached = placement_cache.Find(label.identity);
      const bool cached_geometry_matches = cached != nullptr &&
        AirspaceLabelPlacementCache::Matches(*cached, layout.visual_size,
                                             layout.padding);

      if (!fresh_layout && !cached_geometry_matches) {
        // A cache entry with obsolete label geometry must not be used with a
        // different-sized box.  Defer the fresh layout to the next frame; the
        // current frame remains collision-safe by omitting only this label.
        geometry_changed |= cached != nullptr;
        continue;
      }

      const std::optional<AirspaceLabelCandidate> preferred_candidate =
        cached_geometry_matches
          ? std::optional<AirspaceLabelCandidate>{cached->candidate_index}
          : std::nullopt;
      auto candidate_index = AirspaceLabelCandidate::BELOW;
      if (!DrawLabel(canvas, projection.GeoToScreen(label.pos), screen_rect,
                     label, settings, label_block, layout,
                     preferred_candidate, fresh_layout, candidate_index))
        continue;

      if (fresh_layout)
        placement_cache.Store(label.identity, layout.visual_size,
                              layout.padding, candidate_index);
      else
        placement_cache.MarkUsed(*cached);
    }

    if (fresh_layout) {
      placement_cache.CompleteFreshLayout(now);
      UpdatePlacementCacheContext(screen_rect, eligibility);
    } else if (geometry_changed) {
      // Invalidate the whole cache before the following render, as a changed
      // label format can affect placement decisions for its neighbours too.
      InvalidatePlacementCache();
    }
  }

  if (draw_notam_labels) {
    TextInBoxMode mode{};
    mode.shape = LabelShape::ROUNDED_WHITE;
    mode.align = TextInBoxMode::Alignment::CENTER;
    mode.vertical_position = TextInBoxMode::VerticalPosition::CENTERED;
    mode.move_in_view = true;

    StaticArray<NotamLabelCluster, NOTAM_CLUSTER_MAX_COUNT> clusters;
    const unsigned cluster_distance = GetNotamClusterDistance();

    for (const auto &i : airspaces->QueryWithinRange(projection.GetGeoScreenCenter(),
                                                     projection.GetScreenDistanceMeters())) {
      const AbstractAirspace &airspace = i.GetAirspace();
      if (!visible(airspace) ||
          airspace.GetType() != AirspaceClass::NOTAM)
        continue;

      const GeoBounds screen_bounds = projection.GetScreenBounds();
      GeoBounds airspace_bounds = airspace.GetGeoBounds();
      if (!airspace_bounds.Overlaps(screen_bounds))
        continue;

      auto pos = projection.GeoToScreenIfVisible(airspace.GetCenter());
      if (!pos) {
        if (!airspace_bounds.IntersectWith(screen_bounds))
          continue;

        pos = projection.GeoToScreenIfVisible(airspace_bounds.GetCenter());
        if (!pos)
          continue;
      }

      const auto label = MakeNotamLabelText(airspace);

      NotamLabelCluster *cluster = nullptr;
      for (auto &candidate : clusters)
        if (MatchesNotamCluster(candidate, *pos, cluster_distance)) {
          cluster = &candidate;
          break;
        }

      if (cluster == nullptr) {
        if (clusters.full()) {
#ifndef NDEBUG
          LogFmt("AirspaceLabelRenderer: skipped NOTAM label at {},{} "
                 "(cluster buffer full)",
                 pos->x, pos->y);
#endif
          continue;
        }

        cluster = &clusters.append();
        cluster->anchor = *pos;
        cluster->count = 0;
        cluster->labels.clear();
      }

      AddToNotamCluster(*cluster, label);
    }

    const PixelRect screen_rect = projection.GetScreenRect();
    for (const auto &cluster : clusters)
      DrawNotamCluster(canvas,
                       AdjustNotamClusterAnchor(cluster.anchor, screen_rect),
                       cluster, mode, screen_rect, label_block);
  }
}

bool
AirspaceLabelRenderer::DrawLabel(Canvas &canvas, const PixelPoint anchor,
                                 const PixelRect &map_rect,
                                 const AirspaceLabelList::Label &label,
                                 const AirspaceRendererSettings &settings,
                                 LabelBlock *const label_block,
                                 AirspaceLabelLayout &layout,
                                 const std::optional<AirspaceLabelCandidate>
                                   preferred_candidate,
                                 const bool allow_fallback,
                                 AirspaceLabelCandidate &candidate_index) noexcept
{
  std::optional<AirspaceLabelPlacement> placement;
  if (allow_fallback)
    placement = PlaceAirspaceLabel(anchor, layout.visual_size, layout.padding,
                                   map_rect, label_block, preferred_candidate);
  else if (preferred_candidate)
    placement = PlaceAirspaceLabelCandidate(anchor, layout.visual_size,
                                            layout.padding, map_rect,
                                            label_block,
                                            *preferred_candidate);

  if (!placement)
    return false;

  candidate_index = placement->candidate_index;

  const Color color = settings.black_outline
    ? COLOR_BLACK
    : Color(settings.classes[label.border_class].border_color);
  canvas.SetTextColor(color);
  if (settings.black_outline)
    canvas.SelectBlackPen();
  else
    canvas.Select(look.classes[label.border_class].label_pen);

  layout.SetVisualRect(placement->visual_rect);
  canvas.DrawRectangle(layout.visual_rect);

#ifdef USE_GDI
  canvas.DrawLine(layout.visual_rect.left + layout.padding,
                  layout.separator_y,
                  layout.visual_rect.right - layout.padding,
                  layout.separator_y);
#else
  canvas.DrawHLine(layout.visual_rect.left + layout.padding,
                   layout.visual_rect.right - layout.padding,
                   layout.separator_y, color);
#endif

  canvas.DrawText(layout.top_text_origin, layout.top_text);
  canvas.DrawText(layout.base_text_origin, layout.base_text);
  return true;
}
