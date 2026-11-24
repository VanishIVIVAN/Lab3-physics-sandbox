#pragma once

#include "sandbox/Shape.h"
#include "sandbox/Units.h"

namespace sandbox {

/// 1 m x 1 m square. Medium strength.
class BoxShape final : public Shape {
 public:
  BoxShape();
  ShapeType type() const override { return ShapeType::Box; }
  std::string name() const override { return "Box"; }
  const ShapeGeometry& geometry() const override { return geometry_; }
  float area() const override { return 4.0f * kHalfExtent * kHalfExtent; }
  float mass() const override { return 4.0f; }
  float strength() const override { return 70.0f; }
  Rgba color() const override { return {230, 145, 56, 255}; }

 private:
  static constexpr float kHalfExtent = 0.5f;
  ShapeGeometry geometry_;
};

/// Disc of radius 0.5 m. Slightly weaker than the box.
class CircleShape final : public Shape {
 public:
  CircleShape();
  ShapeType type() const override { return ShapeType::Circle; }
  std::string name() const override { return "Circle"; }
  const ShapeGeometry& geometry() const override { return geometry_; }
  float area() const override { return units::kPi * kRadius * kRadius; }
  float mass() const override { return 2.0f; }
  float strength() const override { return 50.0f; }
  Rgba color() const override { return {86, 156, 214, 255}; }

 private:
  static constexpr float kRadius = 0.5f;
  ShapeGeometry geometry_;
};

/// Isosceles triangle, 1.2 m base and height. The most fragile object.
class TriangleShape final : public Shape {
 public:
  TriangleShape();
  ShapeType type() const override { return ShapeType::Triangle; }
  std::string name() const override { return "Triangle"; }
  const ShapeGeometry& geometry() const override { return geometry_; }
  float area() const override { return 0.5f * kBase * kHeight; }
  float mass() const override { return 1.5f; }
  float strength() const override { return 30.0f; }
  Rgba color() const override { return {106, 190, 120, 255}; }

 private:
  static constexpr float kBase = 1.2f;
  static constexpr float kHeight = 1.2f;
  ShapeGeometry geometry_;
};

/// Small, very dense ball (30 kg). Hard to break, breaks other things.
class HeavyBallShape final : public Shape {
 public:
  HeavyBallShape();
  ShapeType type() const override { return ShapeType::HeavyBall; }
  std::string name() const override { return "Heavy Ball"; }
  const ShapeGeometry& geometry() const override { return geometry_; }
  float area() const override { return units::kPi * kRadius * kRadius; }
  float mass() const override { return 30.0f; }
  float strength() const override { return 1000.0f; }
  Rgba color() const override { return {140, 120, 170, 255}; }

 private:
  static constexpr float kRadius = 0.45f;
  ShapeGeometry geometry_;
};

}  // namespace sandbox
