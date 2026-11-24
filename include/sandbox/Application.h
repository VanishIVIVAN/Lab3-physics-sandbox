#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "sandbox/DebrisEffect.h"
#include "sandbox/PhysicsWorld.h"
#include "sandbox/Renderer.h"
#include "sandbox/UiPanel.h"
#include "sandbox/VelocityTracker.h"

namespace sandbox {

/// Owns the components and runs the main loop: input -> menu -> fixed-step
/// physics -> rendering.
class Application {
 public:
  Application();
  int run();

 private:
  void handleEvent(const sf::Event& event);
  void handleKey(sf::Keyboard::Key key);
  void handleActions(const UiActions& actions);

  void spawnAtCursor();
  void spawnAt(Vec2 position);
  void beginDrag(Vec2 pointMeters);
  void updateDrag();
  void endDrag();

  void togglePause();
  void clearScene();
  void saveScene();
  void loadScene();

  void advanceSimulation(float frameSeconds);
  void render();

  Vec2 mouseWorldPosition() const;
  float now() const { return clock_.getElapsedTime().asSeconds(); }

  // Declaration order = construction order: the window must exist before the
  // UI, and the UI must be destroyed before the window.
  Renderer renderer_;
  UiPanel ui_;
  PhysicsWorld world_;
  DebrisEffect debris_;
  UiState state_;
  VelocityTracker mouseVelocity_;
  sf::Clock clock_;
  float accumulator_ = 0.0f;
};

}  // namespace sandbox
