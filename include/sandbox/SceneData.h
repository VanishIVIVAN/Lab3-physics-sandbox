#pragma once

#include <vector>

#include "sandbox/Config.h"
#include "sandbox/Shape.h"
#include "sandbox/Vec2.h"

namespace sandbox {

/// Surface properties given to a body at spawn time.
struct Material {
  float friction = config::kDefaultFriction;
  float restitution = config::kDefaultRestitution;
};

/// Everything needed to re-create one body.
struct BodyRecord {
  ShapeType type = ShapeType::Box;
  Vec2 position;
  float angle = 0.0f;
  Vec2 linearVelocity;
  float angularVelocity = 0.0f;
  Material material;
};

/// A whole scene. Plain data: knows nothing about Box2D or JSON.
struct SceneData {
  int version = 1;
  float gravity = config::kDefaultGravity;
  std::vector<BodyRecord> bodies;
};

}  // namespace sandbox
