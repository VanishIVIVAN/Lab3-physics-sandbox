#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "sandbox/SceneData.h"
#include "sandbox/Shape.h"
#include "sandbox/Vec2.h"

namespace sandbox {

/// Read-only snapshot of one body, handed to the renderer.
struct BodyView {
  const Shape& shape;
  Vec2 position;  // meters
  float angle;    // radians, counter-clockwise
};

/// Emitted when a body broke; consumed by the visual effect.
struct DestructionEvent {
  std::shared_ptr<const Shape> shape;
  Vec2 position;
  float angle = 0.0f;
  Vec2 linearVelocity;
  float impulse = 0.0f;
};

/// Wrapper around Box2D. No Box2D type appears in this header (pimpl), and
/// everything is expressed in meters / radians / our own Vec2.
class PhysicsWorld {
 public:
  explicit PhysicsWorld(float gravity);
  ~PhysicsWorld();
  PhysicsWorld(const PhysicsWorld&) = delete;
  PhysicsWorld& operator=(const PhysicsWorld&) = delete;

  /// Gravity magnitude in m/s^2 (positive = down). Clamped to [0, kMaxGravity].
  void setGravity(float gravity);
  float gravity() const;

  /// Advances the simulation by one fixed step, then destroys every body whose
  /// strength was exceeded during the step (never inside the Box2D callback).
  void step(float dt);

  /// Returns (and forgets) the destructions that happened since the last call.
  std::vector<DestructionEvent> takeDestructionEvents();

  /// Spawns a body at rest. Returns false if the body limit is reached.
  bool spawn(ShapeType type, Vec2 position, float angle, const Material& material);

  void clear();
  std::size_t objectCount() const;
  void forEachBody(const std::function<void(const BodyView&)>& visitor) const;

  // --- Mouse dragging (Box2D mouse joint) -----------------------------------
  /// Grabs the dynamic body under `point`, if any.
  bool beginDrag(Vec2 point);
  void updateDrag(Vec2 target);
  /// Releases the grabbed body and gives it `releaseVelocity` (m/s).
  void endDrag(Vec2 releaseVelocity);
  bool isDragging() const;

  // --- Scenes ---------------------------------------------------------------
  SceneData captureScene() const;
  void restoreScene(const SceneData& scene);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace sandbox
