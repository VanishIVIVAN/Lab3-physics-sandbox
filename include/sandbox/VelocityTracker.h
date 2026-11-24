#pragma once

#include <deque>

#include "sandbox/Vec2.h"

namespace sandbox {

/// Estimates the velocity of a moving point (the mouse) from its most recent
/// positions. Used to give a released object the velocity of the mouse.
class VelocityTracker {
 public:
  void reset();
  void addSample(Vec2 position, float timeSeconds);
  /// Average velocity over the last ~0.1 s, in position units per second.
  Vec2 velocity() const;

 private:
  struct Sample {
    Vec2 position;
    float time;
  };
  static constexpr float kWindowSeconds = 0.1f;

  std::deque<Sample> samples_;
};

}  // namespace sandbox
