#include "sandbox/PhysicsWorld.h"

#include <box2d/box2d.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <utility>

#include "sandbox/Config.h"
#include "sandbox/ShapeFactory.h"

namespace sandbox {
namespace {

b2Vec2 toB2(Vec2 v) { return b2Vec2(v.x, v.y); }
Vec2 fromB2(b2Vec2 v) { return Vec2{v.x, v.y}; }

/// Data attached to every dynamic body (through Box2D's user data pointer).
struct GameObject {
  std::shared_ptr<const Shape> shape;
  Material material;
  bool markedForDestruction = false;
  float breakingImpulse = 0.0f;
};

/// Box2D 2.4.1 has no const overload of b2Body::GetUserData(), so the const is
/// cast away here; the body itself is only read, never modified.
GameObject* objectOf(const b2Body* body) {
  return reinterpret_cast<GameObject*>(const_cast<b2Body*>(body)->GetUserData().pointer);
}

/// Reads contact impulses after the solver ran. It only MARKS bodies; Box2D
/// forbids destroying bodies while the world is locked (inside a callback).
class BreakListener final : public b2ContactListener {
 public:
  void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override {
    float total = 0.0f;
    for (int i = 0; i < impulse->count; ++i) total += impulse->normalImpulses[i];
    consider(contact->GetFixtureA()->GetBody(), total);
    consider(contact->GetFixtureB()->GetBody(), total);
  }

  std::vector<b2Body*> takePending() { return std::exchange(pending_, {}); }
  void clearPending() { pending_.clear(); }

 private:
  void consider(b2Body* body, float impulse) {
    GameObject* object = objectOf(body);
    if (object == nullptr || object->markedForDestruction) return;  // ground / walls / already marked
    if (impulse > object->shape->strength()) {
      object->markedForDestruction = true;
      object->breakingImpulse = impulse;
      pending_.push_back(body);
    }
  }

  std::vector<b2Body*> pending_;
};

/// Finds the first dynamic body whose fixture contains a point.
class PointQuery final : public b2QueryCallback {
 public:
  explicit PointQuery(b2Vec2 point) : point_(point) {}

  bool ReportFixture(b2Fixture* fixture) override {
    b2Body* body = fixture->GetBody();
    if (body->GetType() == b2_dynamicBody && fixture->TestPoint(point_)) {
      found_ = body;
      return false;  // stop the query
    }
    return true;
  }

  b2Body* found() const { return found_; }

 private:
  b2Vec2 point_;
  b2Body* found_ = nullptr;
};

}  // namespace

struct PhysicsWorld::Impl {
  explicit Impl(float gravityValue) : world(b2Vec2(0.0f, -gravityValue)) {
    world.SetContactListener(&listener);
    createBoundaries();
  }

  void createBoundaries() {
    b2BodyDef definition;  // static by default
    boundaryBody = world.CreateBody(&definition);
    for (const config::Rect& rect : config::boundaries()) {
      b2PolygonShape box;
      box.SetAsBox(rect.halfSize.x, rect.halfSize.y, toB2(rect.center), 0.0f);
      b2FixtureDef fixture;
      fixture.shape = &box;
      fixture.friction = config::kGroundFriction;
      fixture.restitution = 0.0f;
      boundaryBody->CreateFixture(&fixture);
    }
  }

  bool spawnBody(const BodyRecord& record) {
    if (objects.size() >= config::kMaxBodies) return false;

    auto object = std::make_unique<GameObject>();
    object->shape = ShapeFactory::create(record.type);
    object->material.friction = std::clamp(record.material.friction, 0.0f, 2.0f);
    object->material.restitution = std::clamp(record.material.restitution, 0.0f, 1.0f);

    b2BodyDef definition;
    definition.type = b2_dynamicBody;
    definition.position = toB2(record.position);
    definition.angle = record.angle;
    definition.linearVelocity = toB2(record.linearVelocity);
    definition.angularVelocity = record.angularVelocity;
    definition.userData.pointer = reinterpret_cast<std::uintptr_t>(object.get());
    b2Body* body = world.CreateBody(&definition);

    const Shape& shape = *object->shape;
    const ShapeGeometry& geometry = shape.geometry();

    b2FixtureDef fixture;
    fixture.density = shape.density();
    fixture.friction = object->material.friction;
    fixture.restitution = object->material.restitution;

    if (geometry.isCircle) {
      b2CircleShape circle;
      circle.m_radius = geometry.radius;
      fixture.shape = &circle;
      body->CreateFixture(&fixture);
    } else {
      std::vector<b2Vec2> points;
      points.reserve(geometry.vertices.size());
      for (const Vec2& v : geometry.vertices) points.push_back(toB2(v));
      b2PolygonShape polygon;
      polygon.Set(points.data(), static_cast<int32>(points.size()));
      fixture.shape = &polygon;
      body->CreateFixture(&fixture);
    }

    objects.emplace(body, std::move(object));
    return true;
  }

  /// Runs after world.Step(): the world is unlocked, so bodies may be destroyed.
  void destroyBrokenBodies() {
    for (b2Body* body : listener.takePending()) {
      const auto it = objects.find(body);
      if (it == objects.end()) continue;
      const GameObject& object = *it->second;

      DestructionEvent event;
      event.shape = object.shape;
      event.position = fromB2(body->GetPosition());
      event.angle = body->GetAngle();
      event.linearVelocity = fromB2(body->GetLinearVelocity());
      event.impulse = object.breakingImpulse;
      events.push_back(event);

      spdlog::info("Destroyed {} at ({:.2f}, {:.2f}): impulse {:.1f} > strength {:.1f}",
                   object.shape->name(), event.position.x, event.position.y, event.impulse,
                   object.shape->strength());

      // DestroyBody also destroys joints attached to the body.
      if (mouseJoint != nullptr && mouseJoint->GetBodyB() == body) mouseJoint = nullptr;
      world.DestroyBody(body);
      objects.erase(it);
    }
  }

  // Declaration order matters: the listener must outlive the world, and the
  // GameObjects (owned by `objects`) are destroyed before the world.
  BreakListener listener;
  b2World world;
  b2Body* boundaryBody = nullptr;  // non-owning, owned by the b2World
  b2MouseJoint* mouseJoint = nullptr;
  std::unordered_map<const b2Body*, std::unique_ptr<GameObject>> objects;
  std::vector<DestructionEvent> events;
};

PhysicsWorld::PhysicsWorld(float gravity)
    : impl_(std::make_unique<Impl>(std::clamp(gravity, 0.0f, config::kMaxGravity))) {}

PhysicsWorld::~PhysicsWorld() = default;

void PhysicsWorld::setGravity(float gravity) {
  gravity = std::clamp(gravity, 0.0f, config::kMaxGravity);
  if (std::abs(gravity - this->gravity()) < 1e-4f) return;
  impl_->world.SetGravity(b2Vec2(0.0f, -gravity));
  // Sleeping bodies would ignore the new gravity until something touches them.
  for (b2Body* body = impl_->world.GetBodyList(); body != nullptr; body = body->GetNext()) {
    body->SetAwake(true);
  }
}

float PhysicsWorld::gravity() const { return -impl_->world.GetGravity().y; }

void PhysicsWorld::step(float dt) {
  impl_->world.Step(dt, config::kVelocityIterations, config::kPositionIterations);
  impl_->destroyBrokenBodies();
}

std::vector<DestructionEvent> PhysicsWorld::takeDestructionEvents() {
  return std::exchange(impl_->events, {});
}

bool PhysicsWorld::spawn(ShapeType type, Vec2 position, float angle, const Material& material) {
  BodyRecord record;
  record.type = type;
  record.position = position;
  record.angle = angle;
  record.material = material;
  if (!impl_->spawnBody(record)) {
    spdlog::warn("Spawn refused: body limit ({}) reached", config::kMaxBodies);
    return false;
  }
  spdlog::info("Spawned {} at ({:.2f}, {:.2f}), friction {:.2f}, restitution {:.2f}",
               ShapeFactory::toKey(type), position.x, position.y, material.friction,
               material.restitution);
  return true;
}

void PhysicsWorld::clear() {
  b2Body* body = impl_->world.GetBodyList();
  while (body != nullptr) {
    b2Body* next = body->GetNext();
    if (body != impl_->boundaryBody) impl_->world.DestroyBody(body);
    body = next;
  }
  impl_->mouseJoint = nullptr;  // destroyed together with its body
  impl_->objects.clear();
  impl_->listener.clearPending();
  impl_->events.clear();
  spdlog::info("Scene cleared");
}

std::size_t PhysicsWorld::objectCount() const { return impl_->objects.size(); }

void PhysicsWorld::forEachBody(const std::function<void(const BodyView&)>& visitor) const {
  for (const b2Body* body = impl_->world.GetBodyList(); body != nullptr; body = body->GetNext()) {
    const GameObject* object = objectOf(body);
    if (object == nullptr) continue;  // static boundary body
    visitor(BodyView{*object->shape, fromB2(body->GetPosition()), body->GetAngle()});
  }
}

bool PhysicsWorld::beginDrag(Vec2 point) {
  if (impl_->mouseJoint != nullptr) return false;

  const b2Vec2 p = toB2(point);
  const b2Vec2 margin(0.001f, 0.001f);
  b2AABB box;
  box.lowerBound = p - margin;
  box.upperBound = p + margin;

  PointQuery query(p);
  impl_->world.QueryAABB(&query, box);
  b2Body* body = query.found();
  if (body == nullptr) return false;

  b2MouseJointDef definition;
  definition.bodyA = impl_->boundaryBody;  // any static body works as the anchor
  definition.bodyB = body;
  definition.target = p;
  definition.maxForce = config::kDragMaxForcePerKg * body->GetMass();
  b2LinearStiffness(definition.stiffness, definition.damping, config::kDragFrequencyHz,
                    config::kDragDampingRatio, definition.bodyA, definition.bodyB);
  impl_->mouseJoint = static_cast<b2MouseJoint*>(impl_->world.CreateJoint(&definition));
  body->SetAwake(true);
  return true;
}

void PhysicsWorld::updateDrag(Vec2 target) {
  if (impl_->mouseJoint != nullptr) impl_->mouseJoint->SetTarget(toB2(target));
}

void PhysicsWorld::endDrag(Vec2 releaseVelocity) {
  if (impl_->mouseJoint == nullptr) return;
  b2Body* body = impl_->mouseJoint->GetBodyB();
  impl_->world.DestroyJoint(impl_->mouseJoint);
  impl_->mouseJoint = nullptr;
  body->SetLinearVelocity(toB2(releaseVelocity));  // the throw
  body->SetAwake(true);
}

bool PhysicsWorld::isDragging() const { return impl_->mouseJoint != nullptr; }

SceneData PhysicsWorld::captureScene() const {
  SceneData scene;
  scene.gravity = gravity();
  for (const b2Body* body = impl_->world.GetBodyList(); body != nullptr; body = body->GetNext()) {
    const GameObject* object = objectOf(body);
    if (object == nullptr) continue;
    BodyRecord record;
    record.type = object->shape->type();
    record.position = fromB2(body->GetPosition());
    record.angle = body->GetAngle();
    record.linearVelocity = fromB2(body->GetLinearVelocity());
    record.angularVelocity = body->GetAngularVelocity();
    record.material = object->material;
    scene.bodies.push_back(record);
  }
  // Box2D lists the newest body first; store in spawn order.
  std::reverse(scene.bodies.begin(), scene.bodies.end());
  return scene;
}

void PhysicsWorld::restoreScene(const SceneData& scene) {
  clear();
  setGravity(scene.gravity);
  std::size_t restored = 0;
  for (const BodyRecord& record : scene.bodies) {
    if (!impl_->spawnBody(record)) {
      spdlog::warn("Scene has more bodies than the limit ({}); the rest was skipped",
                   config::kMaxBodies);
      break;
    }
    ++restored;
  }
  spdlog::info("Restored {} bodies, gravity {:.2f}", restored, gravity());
}

}  // namespace sandbox
