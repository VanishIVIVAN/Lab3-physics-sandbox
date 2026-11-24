#pragma once

#include <SFML/Graphics.hpp>

#include "sandbox/DebrisEffect.h"
#include "sandbox/PhysicsWorld.h"

namespace sandbox {

/// Wrapper around SFML drawing. Owns the window. Converts meters to pixels only
/// through units.h and never touches Box2D.
class Renderer {
 public:
  Renderer();

  sf::RenderWindow& window() { return window_; }
  const sf::RenderWindow& window() const { return window_; }

  void beginFrame();
  void drawBoundaries();
  void drawBodies(const PhysicsWorld& world);
  void drawDebris(const DebrisEffect& debris);
  void endFrame();

 private:
  void drawBody(const BodyView& body);

  sf::RenderWindow window_;
};

}  // namespace sandbox
