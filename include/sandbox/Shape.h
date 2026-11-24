#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "sandbox/Vec2.h"

namespace sandbox {

enum class ShapeType { Box, Circle, Triangle, HeavyBall };

/// Library independent colour (converted to sf::Color inside Renderer).
struct Rgba {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
  std::uint8_t a;
};

/// Collision/drawing geometry in the body's local space (meters).
/// Polygons are convex, counter-clockwise, with the centroid at (0, 0).
struct ShapeGeometry {
  bool isCircle = false;
  float radius = 0.0f;         // circles only
  std::vector<Vec2> vertices;  // polygons only
};

/// A kind of object that can be spawned. Instances are immutable "descriptions"
/// shared (via shared_ptr) by every body of that kind: each subclass defines
/// its own size, mass and strength.
class Shape {
 public:
  virtual ~Shape() = default;

  virtual ShapeType type() const = 0;
  virtual std::string name() const = 0;
  virtual const ShapeGeometry& geometry() const = 0;
  virtual float area() const = 0;      // m^2
  virtual float mass() const = 0;      // kg
  virtual float strength() const = 0;  // max normal impulse (N*s) it survives in one step
  virtual Rgba color() const = 0;

  /// Density that gives the body exactly mass() once the fixture is created.
  float density() const { return mass() / area(); }
};

}  // namespace sandbox
