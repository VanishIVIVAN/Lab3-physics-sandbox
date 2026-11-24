#pragma once

#include <filesystem>
#include <optional>

#include "sandbox/SceneData.h"

namespace sandbox {

/// Wrapper around nlohmann/json: converts SceneData <-> a JSON file.
/// Errors are logged with spdlog and reported through the return value.
class SceneSerializer {
 public:
  /// Writes the scene to `path` (via a temporary file). Returns false on failure.
  static bool save(const SceneData& scene, const std::filesystem::path& path);

  /// Reads and validates a scene. Returns std::nullopt if the file is missing,
  /// malformed or contains invalid values; the caller's state is untouched.
  static std::optional<SceneData> load(const std::filesystem::path& path);
};

}  // namespace sandbox
