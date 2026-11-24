#pragma once

#include <array>
#include <cstddef>

#include "sandbox/Vec2.h"

/// All tunable constants of the sandbox live here.
namespace sandbox::config {

// --- Window layout (pixels) -------------------------------------------------
inline constexpr unsigned kWindowWidthPx = 1280;
inline constexpr unsigned kWindowHeightPx = 720;
inline constexpr unsigned kMenuWidthPx = 280;  // ImGui panel on the right
inline constexpr unsigned kWorldWidthPx = kWindowWidthPx - kMenuWidthPx;

// --- Scale and world size ---------------------------------------------------
inline constexpr float kPixelsPerMeter = 50.0f;
inline constexpr float kWorldWidthM = static_cast<float>(kWorldWidthPx) / kPixelsPerMeter;
inline constexpr float kWorldHeightM = static_cast<float>(kWindowHeightPx) / kPixelsPerMeter;
inline constexpr float kBoundaryThicknessM = 0.6f;  // visible thickness of ground and walls

// --- Simulation -------------------------------------------------------------
inline constexpr float kTimeStep = 1.0f / 60.0f;      // fixed physics step
inline constexpr float kMaxFrameTimeSeconds = 0.25f;  // avoids the "spiral of death"
inline constexpr int kVelocityIterations = 8;
inline constexpr int kPositionIterations = 3;
inline constexpr float kDefaultGravity = 9.81f;  // m/s^2, positive = downwards
inline constexpr float kMaxGravity = 30.0f;
inline constexpr float kDefaultFriction = 0.5f;
inline constexpr float kDefaultRestitution = 0.3f;
inline constexpr float kGroundFriction = 1.0f;
inline constexpr std::size_t kMaxBodies = 400;
inline constexpr Vec2 kMenuSpawnPointM{kWorldWidthM / 2.0f, 8.0f};

// --- Mouse dragging (Box2D mouse joint) ------------------------------------
inline constexpr float kDragFrequencyHz = 5.0f;
inline constexpr float kDragDampingRatio = 0.7f;
inline constexpr float kDragMaxForcePerKg = 500.0f;
inline constexpr float kMaxThrowSpeed = 40.0f;  // m/s

/// Axis-aligned rectangle given by its centre and half extents (meters).
struct Rect {
  Vec2 center;
  Vec2 halfSize;
};

/// Static ground + left wall + right wall. Used by both the physics world and
/// the renderer, so they can never disagree. The playable area is
/// x in [kBoundaryThicknessM, kWorldWidthM - kBoundaryThicknessM], y >= kBoundaryThicknessM.
inline std::array<Rect, 3> boundaries() {
  constexpr float t = kBoundaryThicknessM;
  constexpr float overshoot = 1.0f;  // extends outside the visible window
  constexpr float wallTop = 40.0f;
  const float w = kWorldWidthM;
  return {{
      // Ground: x in [-overshoot, w + overshoot], y in [-overshoot, t].
      Rect{Vec2{w / 2.0f, (t - overshoot) / 2.0f},
           Vec2{w / 2.0f + overshoot, (t + overshoot) / 2.0f}},
      // Left wall: x in [-overshoot, t], y in [-overshoot, wallTop].
      Rect{Vec2{(t - overshoot) / 2.0f, (wallTop - overshoot) / 2.0f},
           Vec2{(t + overshoot) / 2.0f, (wallTop + overshoot) / 2.0f}},
      // Right wall: x in [w - t, w + overshoot], y in [-overshoot, wallTop].
      Rect{Vec2{w + (overshoot - t) / 2.0f, (wallTop - overshoot) / 2.0f},
           Vec2{(t + overshoot) / 2.0f, (wallTop + overshoot) / 2.0f}},
  }};
}

}  // namespace sandbox::config
