#include "sandbox/Application.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <optional>

#include "sandbox/Config.h"
#include "sandbox/SceneSerializer.h"
#include "sandbox/Units.h"

namespace sandbox {
namespace {

const std::filesystem::path kScenePath = "scene.json";

/// True if `p` (meters) is inside the walls, above the ground.
bool isInsidePlayfield(Vec2 p) {
  constexpr float t = config::kBoundaryThicknessM;
  return p.x >= t && p.x <= config::kWorldWidthM - t && p.y >= t && p.y <= config::kWorldHeightM;
}

Vec2 clampToPlayfield(Vec2 p) {
  constexpr float t = config::kBoundaryThicknessM;
  return Vec2{std::clamp(p.x, t, config::kWorldWidthM - t),
              std::clamp(p.y, t, config::kWorldHeightM)};
}

Vec2 limitSpeed(Vec2 velocity, float maxSpeed) {
  const float speed = length(velocity);
  return speed > maxSpeed ? velocity * (maxSpeed / speed) : velocity;
}

}  // namespace

Application::Application() : renderer_(), ui_(renderer_.window()), world_(config::kDefaultGravity) {
  spdlog::info("Physics sandbox started");
}

int Application::run() {
  sf::RenderWindow& window = renderer_.window();
  sf::Clock frameClock;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      ui_.processEvent(window, event);
      handleEvent(event);
    }
    if (!window.isOpen()) break;

    const sf::Time frameTime = frameClock.restart();
    ui_.beginFrame(window, frameTime);
    handleActions(ui_.drawMenu(state_, world_.objectCount()));

    world_.setGravity(state_.gravity);
    updateDrag();
    advanceSimulation(frameTime.asSeconds());
    render();
  }

  spdlog::info("Physics sandbox closed");
  return 0;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void Application::handleEvent(const sf::Event& event) {
  switch (event.type) {
    case sf::Event::Closed:
      renderer_.window().close();
      break;
    case sf::Event::LostFocus:
      endDrag();
      break;
    case sf::Event::KeyPressed:
      if (!ui_.wantsKeyboard()) handleKey(event.key.code);
      break;
    case sf::Event::MouseButtonPressed: {
      const bool overMenu = ui_.wantsMouse() ||
                            event.mouseButton.x >= static_cast<int>(config::kWorldWidthPx);
      if (event.mouseButton.button == sf::Mouse::Left && !overMenu) {
        beginDrag(units::toMeters(
            Vec2{static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)}));
      }
      break;
    }
    case sf::Event::MouseButtonReleased:
      if (event.mouseButton.button == sf::Mouse::Left) endDrag();
      break;
    default:
      break;
  }
}

void Application::handleKey(sf::Keyboard::Key key) {
  switch (key) {
    case sf::Keyboard::S:
      spawnAtCursor();
      break;
    case sf::Keyboard::Space:
      togglePause();
      break;
    case sf::Keyboard::C:
      clearScene();
      break;
    case sf::Keyboard::F5:
      saveScene();
      break;
    case sf::Keyboard::F9:
      loadScene();
      break;
    default:
      break;
  }
}

void Application::handleActions(const UiActions& actions) {
  if (actions.spawn) spawnAt(config::kMenuSpawnPointM);
  if (actions.save) saveScene();
  if (actions.load) loadScene();
  if (actions.clear) clearScene();
  if (actions.togglePause) togglePause();
}

Vec2 Application::mouseWorldPosition() const {
  const sf::Vector2i pixel = sf::Mouse::getPosition(renderer_.window());
  return units::toMeters(Vec2{static_cast<float>(pixel.x), static_cast<float>(pixel.y)});
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------

void Application::spawnAtCursor() {
  const Vec2 position = mouseWorldPosition();
  if (!isInsidePlayfield(position)) return;  // cursor over the menu or the walls
  spawnAt(position);
}

void Application::spawnAt(Vec2 position) {
  if (!world_.spawn(state_.selectedShape, position, 0.0f, state_.material)) {
    ui_.setStatus("Object limit reached", true);
  }
}

// ---------------------------------------------------------------------------
// Drag and throw
// ---------------------------------------------------------------------------

void Application::beginDrag(Vec2 pointMeters) {
  if (!world_.beginDrag(pointMeters)) return;
  mouseVelocity_.reset();
  mouseVelocity_.addSample(clampToPlayfield(pointMeters), now());
}

void Application::updateDrag() {
  if (!world_.isDragging()) return;
  const Vec2 target = clampToPlayfield(mouseWorldPosition());
  world_.updateDrag(target);
  mouseVelocity_.addSample(target, now());
}

void Application::endDrag() {
  if (!world_.isDragging()) return;
  mouseVelocity_.addSample(clampToPlayfield(mouseWorldPosition()), now());
  world_.endDrag(limitSpeed(mouseVelocity_.velocity(), config::kMaxThrowSpeed));
}

// ---------------------------------------------------------------------------
// Scene commands
// ---------------------------------------------------------------------------

void Application::togglePause() {
  state_.paused = !state_.paused;
  spdlog::info("{}", state_.paused ? "Paused" : "Resumed");
}

void Application::clearScene() {
  world_.clear();
  debris_.clear();
  ui_.setStatus("Scene cleared", false);
}

void Application::saveScene() {
  const SceneData scene = world_.captureScene();
  if (SceneSerializer::save(scene, kScenePath)) {
    ui_.setStatus("Saved " + kScenePath.string() + " (" + std::to_string(scene.bodies.size()) +
                      " objects)",
                  false);
  } else {
    ui_.setStatus("Save failed, see sandbox.log", true);
  }
}

void Application::loadScene() {
  const std::optional<SceneData> scene = SceneSerializer::load(kScenePath);
  if (!scene) {
    ui_.setStatus("Load failed, see sandbox.log", true);
    return;
  }
  world_.restoreScene(*scene);
  debris_.clear();
  state_.gravity = world_.gravity();
  ui_.setStatus("Loaded " + kScenePath.string() + " (" + std::to_string(world_.objectCount()) +
                    " objects)",
                false);
}

// ---------------------------------------------------------------------------
// Simulation and drawing
// ---------------------------------------------------------------------------

void Application::advanceSimulation(float frameSeconds) {
  if (state_.paused) {
    accumulator_ = 0.0f;
    return;
  }
  // Fixed time step: render frames of any length are converted into whole
  // 1/60 s physics steps, so the simulation does not depend on the frame rate.
  accumulator_ += std::min(frameSeconds, config::kMaxFrameTimeSeconds);
  while (accumulator_ >= config::kTimeStep) {
    world_.step(config::kTimeStep);
    for (const DestructionEvent& event : world_.takeDestructionEvents()) {
      debris_.spawnFragments(event);
    }
    debris_.update(config::kTimeStep, world_.gravity());
    accumulator_ -= config::kTimeStep;
  }
}

void Application::render() {
  renderer_.beginFrame();
  renderer_.drawBoundaries();
  renderer_.drawBodies(world_);
  renderer_.drawDebris(debris_);
  ui_.render(renderer_.window());
  renderer_.endFrame();
}

}  // namespace sandbox
