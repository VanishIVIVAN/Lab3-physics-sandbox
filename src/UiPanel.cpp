#include "sandbox/UiPanel.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <stdexcept>

#include "sandbox/ShapeFactory.h"

namespace sandbox {
namespace {

// ImGui-SFML for SFML 2.6 takes (window, event); older releases take only the
// event. The first overload is chosen when it compiles (SFINAE), otherwise the
// second one, so the code builds against either version.
template <typename Window, typename Event>
auto forwardEvent(Window& window, const Event& event, int)
    -> decltype(ImGui::SFML::ProcessEvent(window, event), void()) {
  ImGui::SFML::ProcessEvent(window, event);
}

template <typename Window, typename Event>
void forwardEvent(Window&, const Event& event, long) {
  ImGui::SFML::ProcessEvent(event);
}

}  // namespace

UiPanel::UiPanel(sf::RenderWindow& window) {
  if (!ImGui::SFML::Init(window)) throw std::runtime_error("ImGui-SFML initialisation failed");
  ImGui::GetIO().IniFilename = nullptr;  // do not write imgui.ini

  for (ShapeType type : ShapeFactory::allTypes()) {
    shapeTypes_.push_back(type);
    shapeNames_.push_back(ShapeFactory::create(type)->name());
  }
}

UiPanel::~UiPanel() { ImGui::SFML::Shutdown(); }

void UiPanel::processEvent(sf::RenderWindow& window, const sf::Event& event) {
  forwardEvent(window, event, 0);
}

void UiPanel::beginFrame(sf::RenderWindow& window, sf::Time frameTime) {
  ImGui::SFML::Update(window, frameTime);
}

void UiPanel::render(sf::RenderWindow& window) { ImGui::SFML::Render(window); }

bool UiPanel::wantsMouse() const { return ImGui::GetIO().WantCaptureMouse; }

bool UiPanel::wantsKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

void UiPanel::setStatus(const std::string& message, bool isError) {
  status_ = message;
  statusIsError_ = isError;
}

UiActions UiPanel::drawMenu(UiState& state, std::size_t bodyCount) {
  UiActions actions;

  ImGui::SetNextWindowPos(ImVec2(static_cast<float>(config::kWorldWidthPx), 0.0f));
  ImGui::SetNextWindowSize(
      ImVec2(static_cast<float>(config::kMenuWidthPx), static_cast<float>(config::kWindowHeightPx)));
  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
  ImGui::Begin("Physics Sandbox", nullptr, flags);

  ImGui::Text("Objects: %zu / %zu", bodyCount, config::kMaxBodies);
  ImGui::Text("State: %s", state.paused ? "PAUSED" : "running");
  ImGui::Separator();

  // --- Spawning ---
  std::size_t selectedIndex = 0;
  for (std::size_t i = 0; i < shapeTypes_.size(); ++i) {
    if (shapeTypes_[i] == state.selectedShape) selectedIndex = i;
  }
  if (ImGui::BeginCombo("Object", shapeNames_[selectedIndex].c_str())) {
    for (std::size_t i = 0; i < shapeTypes_.size(); ++i) {
      const bool isSelected = (i == selectedIndex);
      if (ImGui::Selectable(shapeNames_[i].c_str(), isSelected)) state.selectedShape = shapeTypes_[i];
      if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  if (ImGui::Button("Spawn", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) actions.spawn = true;

  // --- Physics parameters ---
  ImGui::Separator();
  ImGui::SliderFloat("Gravity", &state.gravity, 0.0f, config::kMaxGravity, "%.2f m/s^2");
  ImGui::SliderFloat("Friction", &state.material.friction, 0.0f, 1.0f, "%.2f");
  ImGui::SliderFloat("Bounciness", &state.material.restitution, 0.0f, 1.0f, "%.2f");
  ImGui::TextDisabled("Gravity affects all objects;");
  ImGui::TextDisabled("friction/bounciness only new ones.");

  // --- Scene buttons ---
  ImGui::Separator();
  const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
  if (ImGui::Button("Save (F5)", ImVec2(half, 0.0f))) actions.save = true;
  ImGui::SameLine();
  if (ImGui::Button("Load (F9)", ImVec2(half, 0.0f))) actions.load = true;
  if (ImGui::Button("Clear (C)", ImVec2(half, 0.0f))) actions.clear = true;
  ImGui::SameLine();
  if (ImGui::Button(state.paused ? "Resume (Space)" : "Pause (Space)", ImVec2(half, 0.0f))) {
    actions.togglePause = true;
  }

  // --- Status line ---
  if (!status_.empty()) {
    ImGui::Separator();
    const ImVec4 color = statusIsError_ ? ImVec4(1.0f, 0.45f, 0.4f, 1.0f)
                                        : ImVec4(0.55f, 0.9f, 0.6f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextWrapped("%s", status_.c_str());
    ImGui::PopStyleColor();
  }

  // --- Help ---
  ImGui::Separator();
  ImGui::TextWrapped("S - spawn selected object at the cursor");
  ImGui::TextWrapped("Left mouse - grab, drag and throw objects");
  ImGui::TextWrapped("Fast impacts break objects.");

  ImGui::End();
  return actions;
}

}  // namespace sandbox
