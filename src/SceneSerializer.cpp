#include "sandbox/SceneSerializer.h"

#include <spdlog/spdlog.h>

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include <nlohmann/json.hpp>

#include "sandbox/ShapeFactory.h"

namespace sandbox {
namespace {

using nlohmann::json;

constexpr int kSceneVersion = 1;

json toJson(Vec2 v) { return json::array({v.x, v.y}); }

float readFiniteFloat(const json& value, const char* what) {
  const float number = value.get<float>();
  if (!std::isfinite(number)) throw std::runtime_error(std::string("non-finite value in ") + what);
  return number;
}

Vec2 readVec2(const json& value, const char* what) {
  if (!value.is_array() || value.size() != 2) {
    throw std::runtime_error(std::string(what) + " must be an array of two numbers");
  }
  return Vec2{readFiniteFloat(value[0], what), readFiniteFloat(value[1], what)};
}

json toJson(const BodyRecord& body) {
  return json{{"type", ShapeFactory::toKey(body.type)},
              {"position", toJson(body.position)},
              {"angle", body.angle},
              {"linearVelocity", toJson(body.linearVelocity)},
              {"angularVelocity", body.angularVelocity},
              {"friction", body.material.friction},
              {"restitution", body.material.restitution}};
}

BodyRecord bodyFromJson(const json& item) {
  BodyRecord body;
  const std::string key = item.at("type").get<std::string>();
  const std::optional<ShapeType> type = ShapeFactory::fromKey(key);
  if (!type) throw std::runtime_error("unknown shape type '" + key + "'");
  body.type = *type;
  body.position = readVec2(item.at("position"), "position");
  body.angle = readFiniteFloat(item.at("angle"), "angle");
  body.linearVelocity = readVec2(item.at("linearVelocity"), "linearVelocity");
  body.angularVelocity = readFiniteFloat(item.at("angularVelocity"), "angularVelocity");
  body.material.friction = readFiniteFloat(item.at("friction"), "friction");
  body.material.restitution = readFiniteFloat(item.at("restitution"), "restitution");
  return body;
}

}  // namespace

bool SceneSerializer::save(const SceneData& scene, const std::filesystem::path& path) {
  try {
    json root;
    root["version"] = kSceneVersion;
    root["gravity"] = scene.gravity;
    root["bodies"] = json::array();
    for (const BodyRecord& body : scene.bodies) root["bodies"].push_back(toJson(body));

    // Write to a temporary file first so a failed save never destroys the old one.
    std::filesystem::path temporary = path;
    temporary += ".tmp";
    {
      std::ofstream out(temporary, std::ios::trunc);
      if (!out) throw std::runtime_error("cannot open '" + temporary.string() + "' for writing");
      out << root.dump(2) << '\n';
      out.flush();
      if (!out) throw std::runtime_error("write error on '" + temporary.string() + "'");
    }
    std::error_code error;
    std::filesystem::rename(temporary, path, error);
    if (error) throw std::runtime_error("cannot replace '" + path.string() + "': " + error.message());

    spdlog::info("Saved scene to {} ({} bodies)", path.string(), scene.bodies.size());
    return true;
  } catch (const std::exception& error) {
    spdlog::error("Saving scene failed: {}", error.what());
    return false;
  }
}

std::optional<SceneData> SceneSerializer::load(const std::filesystem::path& path) {
  try {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open '" + path.string() + "'");
    const json root = json::parse(in);

    const int version = root.at("version").get<int>();
    if (version != kSceneVersion) {
      throw std::runtime_error("unsupported scene version " + std::to_string(version));
    }

    SceneData scene;
    scene.version = version;
    scene.gravity = readFiniteFloat(root.at("gravity"), "gravity");
    const json& bodies = root.at("bodies");
    if (!bodies.is_array()) throw std::runtime_error("'bodies' must be an array");
    for (const json& item : bodies) scene.bodies.push_back(bodyFromJson(item));

    spdlog::info("Loaded scene from {} ({} bodies)", path.string(), scene.bodies.size());
    return scene;
  } catch (const std::exception& error) {
    // Covers json::parse_error / type_error / out_of_range as well.
    spdlog::error("Loading scene failed: {}", error.what());
    return std::nullopt;
  }
}

}  // namespace sandbox
