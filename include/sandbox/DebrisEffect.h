#pragma once

#include <random>
#include <vector>

#include "sandbox/PhysicsWorld.h"
#include "sandbox/Shape.h"
#include "sandbox/Vec2.h"

namespace sandbox {

/// One small piece of a broken object. Purely visual: it is not a Box2D body.
struct Fragment {
  Vec2 position;  // meters
  Vec2 velocity;  // m/s
  float angle = 0.0f;
  float angularVelocity = 0.0f;
  float size = 0.1f;  // meters
  Rgba color{255, 255, 255, 255};
  float age = 0.0f;       // seconds
  float lifetime = 1.0f;  // seconds; the fragment fades out over its lifetime
};

/// Breaks destroyed objects into 3-4 fragments that fall away and fade out.
class DebrisEffect {
 public:
  DebrisEffect();

  void spawnFragments(const DestructionEvent& event);
  /// Advances all fragments (call from the fixed-step loop so Pause freezes them).
  void update(float dt, float gravity);
  void clear() { fragments_.clear(); }

  const std::vector<Fragment>& fragments() const { return fragments_; }

 private:
  float randomRange(float low, float high);

  std::vector<Fragment> fragments_;
  std::mt19937 rng_;
};

}  // namespace sandbox
