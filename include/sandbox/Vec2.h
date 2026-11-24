#pragma once

#include <cmath>

namespace sandbox {

/// Minimal 2D vector shared by the whole project. It is library independent:
/// only PhysicsWorld.cpp converts it to Box2D and only Renderer.cpp to SFML.
struct Vec2 {
  float x = 0.0f;
  float y = 0.0f;
};

constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
constexpr Vec2 operator*(Vec2 v, float s) { return {v.x * s, v.y * s}; }
constexpr Vec2 operator*(float s, Vec2 v) { return {v.x * s, v.y * s}; }
constexpr Vec2 operator/(Vec2 v, float s) { return {v.x / s, v.y / s}; }

inline float length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

}  // namespace sandbox
