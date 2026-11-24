#pragma once

#include "sandbox/Config.h"
#include "sandbox/Vec2.h"

/// The ONLY place where pixels <-> meters conversion happens.
///
/// Box2D works in meters with +Y pointing up; SFML works in pixels with +Y
/// pointing down. The world origin (0, 0) is the bottom-left corner of the
/// window, so:   screenX = x * ppm,   screenY = windowHeight - y * ppm.
namespace sandbox::units {

inline constexpr float kPi = 3.14159265358979323846f;

constexpr float lengthToPixels(float meters) { return meters * config::kPixelsPerMeter; }
constexpr float lengthToMeters(float pixels) { return pixels / config::kPixelsPerMeter; }

/// World position (meters) -> window position (pixels).
constexpr Vec2 toPixels(Vec2 meters) {
  return {meters.x * config::kPixelsPerMeter,
          static_cast<float>(config::kWindowHeightPx) - meters.y * config::kPixelsPerMeter};
}

/// Window position (pixels) -> world position (meters).
constexpr Vec2 toMeters(Vec2 pixels) {
  return {pixels.x / config::kPixelsPerMeter,
          (static_cast<float>(config::kWindowHeightPx) - pixels.y) / config::kPixelsPerMeter};
}

/// Offset/direction (no translation) in meters -> pixels, with the Y flip.
constexpr Vec2 offsetToPixels(Vec2 meters) {
  return {meters.x * config::kPixelsPerMeter, -meters.y * config::kPixelsPerMeter};
}

/// Box2D angle (radians, counter-clockwise, Y up) -> SFML rotation (degrees,
/// clockwise on screen). The Y flip reverses the sign.
constexpr float angleToScreenDegrees(float radians) { return -radians * 180.0f / kPi; }

}  // namespace sandbox::units
