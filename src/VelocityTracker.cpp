#include "sandbox/VelocityTracker.h"

namespace sandbox {

void VelocityTracker::reset() { samples_.clear(); }

void VelocityTracker::addSample(Vec2 position, float timeSeconds) {
  samples_.push_back({position, timeSeconds});
  // Drop samples older than the window but always keep at least two.
  while (samples_.size() > 2 && timeSeconds - samples_.front().time > kWindowSeconds) {
    samples_.pop_front();
  }
}

Vec2 VelocityTracker::velocity() const {
  if (samples_.size() < 2) return Vec2{};
  const Sample& first = samples_.front();
  const Sample& last = samples_.back();
  const float elapsed = last.time - first.time;
  if (elapsed < 1e-4f) return Vec2{};
  return (last.position - first.position) / elapsed;
}

}  // namespace sandbox
