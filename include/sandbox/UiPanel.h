#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <string>
#include <vector>

#include "sandbox/Config.h"
#include "sandbox/SceneData.h"
#include "sandbox/Shape.h"

namespace sandbox {

/// Values edited through the menu.
struct UiState {
  ShapeType selectedShape = ShapeType::Box;
  float gravity = config::kDefaultGravity;  // applies to the whole world
  Material material;                        // applies to newly spawned objects
  bool paused = false;
};

/// Buttons clicked during one frame.
struct UiActions {
  bool spawn = false;
  bool save = false;
  bool load = false;
  bool clear = false;
  bool togglePause = false;
};

/// Wrapper around Dear ImGui / ImGui-SFML: owns the ImGui context lifetime and
/// draws the side menu.
class UiPanel {
 public:
  explicit UiPanel(sf::RenderWindow& window);
  ~UiPanel();
  UiPanel(const UiPanel&) = delete;
  UiPanel& operator=(const UiPanel&) = delete;

  void processEvent(sf::RenderWindow& window, const sf::Event& event);
  /// Starts a new ImGui frame; call once per frame before drawMenu().
  void beginFrame(sf::RenderWindow& window, sf::Time frameTime);
  UiActions drawMenu(UiState& state, std::size_t bodyCount);
  void render(sf::RenderWindow& window);

  /// True when the mouse is over the menu (so the world must ignore clicks).
  bool wantsMouse() const;
  /// True while a text field has keyboard focus (so hotkeys must be ignored).
  bool wantsKeyboard() const;

  void setStatus(const std::string& message, bool isError);

 private:
  std::vector<ShapeType> shapeTypes_;
  std::vector<std::string> shapeNames_;
  std::string status_;
  bool statusIsError_ = false;
};

}  // namespace sandbox
