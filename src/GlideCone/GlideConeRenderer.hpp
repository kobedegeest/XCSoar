// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "GlideConeField.hpp"
#include "Geo/GeoPoint.hpp"
#include "thread/Mutex.hxx"
#include "util/Serial.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

class Canvas;
class WindowProjection;
class RasterTerrain;
class Waypoints;
struct MapLook;
struct ComputerSettings;

/**
 * Owns the glide cone GPU computation and draws the resulting relay path
 * on the moving map.
 *
 * The seed(s) come from the active navigation target (single mode) or all
 * landables in a moving window around the aircraft (combined mode).  The
 * expensive GPU compute and the path drawing happen on the draw thread
 * (where the OpenGL context is current) inside Draw().  An explicit "Goto"
 * target may also be pushed from the UI thread as a single-mode fallback.
 */
class GlideConeRenderer {
  Mutex mutex;

  /* shared pending target (guarded by #mutex; single-mode fallback) */
  GeoPoint pending_seed = GeoPoint::Invalid();
  double pending_seed_alt = 0;
  bool pending_valid = false;
  std::uint64_t pending_generation = 0;

  /* draw-thread-owned computed state */
  GlideConeField field;
  GeoPoint computed_center = GeoPoint::Invalid();
  std::size_t computed_signature = 0;
  Serial computed_terrain_serial{};
  bool have_field = false;
  bool computed_contours = false;

  /* debounce for parameter (e.g. glide ratio) changes */
  std::size_t debounce_signature = ~std::size_t{0};
  std::chrono::steady_clock::time_point debounce_since{};

  /* debounce for terrain tile loads so we do not recompute every batch */
  Serial debounce_terrain_serial{};
  std::chrono::steady_clock::time_point terrain_debounce_since{};

public:
  /**
   * Set the airport for which the glide cone should be computed (single
   * mode fallback).  May be called from any thread.
   */
  void SetTarget(GeoPoint seed, double elevation) noexcept;

  /** Forget the current target.  May be called from any thread. */
  void ClearTarget() noexcept;

  /**
   * Draw the glide cone path.  Must be called on the draw thread with the
   * OpenGL context current.
   *
   * @param target the active navigation target (Goto/task destination)
   * used as the seed in single mode
   * @param waypoints waypoint store used to gather landables in combined
   * mode (may be nullptr)
   */
  void Draw(Canvas &canvas, const WindowProjection &projection,
            GeoPoint aircraft, bool aircraft_valid,
            GeoPoint target, bool target_valid,
            const ComputerSettings &settings,
            const RasterTerrain *terrain, const Waypoints *waypoints,
            const MapLook &look) noexcept;

  /**
   * Expand a map-view terrain request so it also covers the glide-cone
   * compute window (seed in single mode, aircraft in combined).
   *
   * @p location and @p radius are the visible-map request on input and
   * the coverage to load on output.
   */
  static void AdjustTerrainCoverage(const ComputerSettings &settings,
                                    GeoPoint aircraft, bool aircraft_valid,
                                    GeoPoint target, bool target_valid,
                                    GeoPoint &location,
                                    double &radius) noexcept;

private:
  /**
   * Build the field for a window centred on @p center with the given
   * half-width, seeded by @p seeds (geographic locations).
   */
  bool BuildField(GeoPoint center, double radius_m,
                  const std::vector<GeoPoint> &seeds,
                  const ComputerSettings &settings,
                  const RasterTerrain &terrain) noexcept;
};
