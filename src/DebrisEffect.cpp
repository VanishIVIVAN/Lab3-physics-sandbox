#include "sandbox/DebrisEffect.h"

#include <algorithm>
#include <cmath>

#include "sandbox/Config.h"
#include "sandbox/Units.h"

namespace sandbox {
namespace {
constexpr float kFragmentSizeFactor = 0.3f;  // fragment size relative to sqrt(area)
constexpr float kInheritedVelocity = 0.3f;   // share of the parent's velocity kept
constexpr float kRestitution = 0.3f;         // fragments bounce a little off the floor/walls
}  // namespace

DebrisEffect::DebrisEffect() : rng_(std::random_device{}()) {}

float DebrisEffect::randomRange(float low, float high) {
  return std::uniform_real_distribution<float>(low, high)(rng_);
}

void DebrisEffect::spawnFragments(const DestructionEvent& event) {
  const int count = std::uniform_int_distribution<int>(3, 4)(rng_);
  const float baseSize = kFragmentSizeFactor * std::sqrt(event.shape->area());
  const float startAngle = randomRange(0.0f, 2.0f * units::kPi);

  for (int i = 0; i < count; ++i) {
    // Spread the fragments evenly around the centre, with a bit of jitter.
    const float direction = startAngle + 2.0f * units::kPi * static_cast<float>(i) /
                                             static_cast<float>(count) +
                            randomRange(-0.3f, 0.3f);
    const Vec2 outward{std::cos(direction), std::sin(direction)};

    Fragment fragment;
    fragment.position = event.position + outward * (0.4f * baseSize);
    fragment.velocity = event.linearVelocity * kInheritedVelocity + outward * randomRange(2.0f, 5.0f);
    fragment.angle = randomRange(0.0f, 2.0f * units::kPi);
    fragment.angularVelocity = randomRange(-8.0f, 8.0f);
    fragment.size = baseSize * randomRange(0.7f, 1.2f);
    fragment.color = event.shape->color();
    fragment.lifetime = randomRange(1.0f, 1.6f);
    fragments_.push_back(fragment);
  }
}

void DebrisEffect::update(float dt, float gravity) {
  const float floorY = config::kBoundaryThicknessM;
  const float leftX = config::kBoundaryThicknessM;
  const float rightX = config::kWorldWidthM - config::kBoundaryThicknessM;

  for (Fragment& f : fragments_) {
    f.age += dt;
    f.velocity.y -= gravity * dt;
    f.position = f.position + f.velocity * dt;
    f.angle += f.angularVelocity * dt;

    if (f.position.y < floorY + f.size * 0.5f) {
      f.position.y = floorY + f.size * 0.5f;
      f.velocity.y = -f.velocity.y * kRestitution;
      f.velocity.x *= 0.7f;
      f.angularVelocity *= 0.7f;
    }
    if (f.position.x < leftX) {
      f.position.x = leftX;
      f.velocity.x = -f.velocity.x * kRestitution;
    } else if (f.position.x > rightX) {
      f.position.x = rightX;
      f.velocity.x = -f.velocity.x * kRestitution;
    }
  }

  fragments_.erase(std::remove_if(fragments_.begin(), fragments_.end(),
                                  [](const Fragment& f) { return f.age >= f.lifetime; }),
                   fragments_.end());
}

}  // namespace sandbox
